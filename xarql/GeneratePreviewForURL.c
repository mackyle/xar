#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

#include <xar/xar.h>
#include <stdint.h>
#include <inttypes.h>
#include <libxml/xmlreader.h>
#include <zlib.h>
#include <syslog.h>

static int gCount = 0;
static int gMax = 30;

struct decomp {
	size_t offset;
	struct xar_header header;
	size_t readbuf_len;
	size_t toc_count;
	char *readbuf;
	int fd;
	z_stream zs;
};

static int toc_read_callback(void *context, char *buffer, int len) {
    struct decomp *x = (struct decomp *)context;
    int ret, off = 0;
	
    if ( ((!x->offset) || (x->offset == x->readbuf_len)) && (x->toc_count != x->header.toc_length_compressed) ) {
        x->offset = 0;
        if( (x->readbuf_len - off) + x->toc_count > x->header.toc_length_compressed )
            ret = read(x->fd, x->readbuf, x->header.toc_length_compressed - x->toc_count);
        else
            ret = read(x->fd, x->readbuf, x->readbuf_len);
        if ( ret == -1 )
            return ret;
				
        x->toc_count += ret;
        off += ret;
    }
	
    if( off && (off < x->readbuf_len) )
        x->readbuf_len = off;
    x->zs.next_in = ((unsigned char *)x->readbuf) + x->offset;
    x->zs.avail_in = x->readbuf_len - x->offset;
    x->zs.next_out = (void *)buffer;
    x->zs.avail_out = len;
	
    ret = inflate(&x->zs, Z_SYNC_FLUSH);
    if( ret < 0 )
        return -1;
    x->offset = x->readbuf_len - x->zs.avail_in;
	
    return len - x->zs.avail_out;
}

static int close_callback(void *context) {
    return 0;
}

static uint64_t ntoh64(uint64_t num) {
    int t = 1234;
    union conv {
        uint64_t i64;
        uint32_t i32[2];
    } *in, out;
	
    if( ntohl(t) == t ) {
        out.i64 = num;
        return out.i64;
    }
    in = (union conv *)&num;
    out.i32[1] = ntohl(in->i32[0]);
    out.i32[0] = ntohl(in->i32[1]);
    return(out.i64);
}

static struct xar_header *get_header(int fd)
{
	char buf[sizeof(struct xar_header)];
	
	if( read(fd, buf, 28) < 28 ) {
		return NULL;
	}
	
	struct xar_header *ret = malloc(sizeof(struct xar_header));
	if( !ret ) return NULL;
	
	ret->magic = ntohl(*(uint32_t *)buf);
	ret->size = ntohs(*(uint16_t *)(buf+4));
	ret->version = ntohs(*(uint16_t *)(buf+6));
	ret->toc_length_compressed = ntoh64(*(uint64_t *)(buf+8));
	ret->toc_length_uncompressed = ntoh64(*(uint64_t *)(buf+16));
	ret->cksum_alg = ntohl(*(uint32_t *)(buf+24));
	
	return ret;
}

static char *parse_file(xmlTextReaderPtr reader, char *str, int depth)
{
	char *ret = str;
	int type;
	const char *name;
	
	if( gCount > gMax ) return str;
	
	while( xmlTextReaderRead(reader) == 1 ) {
        type = xmlTextReaderNodeType(reader);
        name = (const char *)xmlTextReaderConstLocalName(reader);
        if( (type == XML_READER_TYPE_END_ELEMENT) && (strcmp(name, "file")==0) ) {
			return ret;
		}
		
		if( type == XML_READER_TYPE_ELEMENT ) {
            if( strcmp(name, "file") == 0 ) {
                ret = parse_file(reader, ret, depth+1);
				syslog(LOG_ERR, "parse_file: parse_file returned %s", ret);
			} else if( strcmp(name, "name") == 0 ) {
				while( xmlTextReaderRead(reader) == 1 ) {
					type = xmlTextReaderNodeType(reader);
					name = (const char *)xmlTextReaderConstLocalName(reader);
					if( type == XML_READER_TYPE_TEXT ) {
						char * value, *tabs;
						value = (char *)xmlTextReaderConstValue(reader);
						tabs = calloc(1, depth+1);
						memset(tabs, '\t', depth);
						asprintf(&ret, "%s%s%s\n", ret, tabs, value);
					}
					if( (type == XML_READER_TYPE_END_ELEMENT) && (strcmp(name, "name")==0) ) {
						break;
					}
				}
				gCount++;
			} else {
				while( xmlTextReaderRead(reader) == 1 ) {
					int newtype  = xmlTextReaderNodeType(reader);
					const char *newname = (const char *)xmlTextReaderConstLocalName(reader);
					if( (newtype == XML_READER_TYPE_END_ELEMENT) && (strcmp(name, newname)==0) ) {
						break;
					}
					
				}
			}
		}
		if( gCount > gMax ) return ret;
	}
	
	// Should never be reached
	return ret;		
}

/* -----------------------------------------------------------------------------
   Generate a preview for file

   This function's job is to create preview for designated file
   ----------------------------------------------------------------------------- */

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options)
{
    CFMutableStringRef textresp;
	CFStringRef cfpathref;
	CFMutableDictionaryRef dict;
	CFDataRef qldata;
	char cpath[PATH_MAX];
	
	if (QLPreviewRequestIsCancelled(preview)) 
		return noErr; 

	cfpathref = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
	if( !cfpathref )
		return noErr;
	memset(cpath, 0, sizeof(cpath));
	if( CFStringGetCString(cfpathref, cpath, sizeof(cpath)-1, kCFStringEncodingUTF8) == FALSE) {
		CFRelease(cfpathref);
		return noErr;
	}
	CFRelease(cfpathref);
	
	int fd = open(cpath, O_RDONLY);
	struct xar_header *h = get_header(fd);

	textresp = CFStringCreateMutable(NULL, 0);
	
	char *headerstr = NULL;
	asprintf(&headerstr, "Magic:                   0x%x\n", h->magic);
	asprintf(&headerstr, "%sSize:                    %" PRIu16 "\n", headerstr,h->size);
	asprintf(&headerstr, "%sVersion:                 %" PRIu16 "\n", headerstr,h->version);
	asprintf(&headerstr, "%sCompressed TOC Length:   %" PRId64 "\n", headerstr,h->toc_length_compressed);
	asprintf(&headerstr, "%sUncompressed TOC Length: %" PRId64 "\n", headerstr,h->toc_length_uncompressed);
	asprintf(&headerstr, "%sChecksum Algorithm:      %" PRIu32 "\n", headerstr,h->cksum_alg);
	free(h);
	CFStringAppendCString(textresp, headerstr, kCFStringEncodingUTF8);
	free(headerstr);
	
	struct decomp *d = calloc(1, sizeof(struct decomp));
	memcpy(&d->header, h, sizeof(struct xar_header));
	d->readbuf_len = 4096;
	d->readbuf = malloc(d->readbuf_len);
	d->fd = fd;
	inflateInit(&d->zs);
	xmlTextReaderPtr reader;
    const xmlChar *name;
    int type, noattr, ret;
	char *filelist = strdup("");
	const char *creatdate = NULL;

	reader = xmlReaderForIO(toc_read_callback, close_callback, d, NULL, NULL, 0);
	
    while( (ret = xmlTextReaderRead(reader)) == 1 ) {
        type = xmlTextReaderNodeType(reader);
        noattr = xmlTextReaderAttributeCount(reader);
        name = xmlTextReaderConstLocalName(reader);
        if( type != XML_READER_TYPE_ELEMENT )
            continue;
        if(strcmp((const char*)name, "xar") != 0)
            continue;
        while( (ret = xmlTextReaderRead(reader)) == 1 ) {
            type = xmlTextReaderNodeType(reader);
            noattr = xmlTextReaderAttributeCount(reader);
            name = xmlTextReaderConstLocalName(reader);
            if( type == XML_READER_TYPE_ELEMENT ) {
                if(strcmp((const char*)name, "toc") == 0) {
                    while( (ret = xmlTextReaderRead(reader)) == 1 ) {
                        type = xmlTextReaderNodeType(reader);
                        noattr = xmlTextReaderAttributeCount(reader);
                        name = xmlTextReaderConstLocalName(reader);
                        if( type == XML_READER_TYPE_ELEMENT ) {
							if( strcmp((const char *)name, "creation-time") == 0 ) {
								while( xmlTextReaderRead(reader) == 1 ) {
									type = xmlTextReaderNodeType(reader);
									name = xmlTextReaderConstLocalName(reader);
									if( type == XML_READER_TYPE_TEXT ) {
										creatdate = strdup((char *)xmlTextReaderConstValue(reader));
									}
									if( (type == XML_READER_TYPE_END_ELEMENT) && (strcmp((const char *)name, "creation-time")==0) ) {
										break;
									}
								}
							} else if( strcmp((const char*)name, "file") == 0) {
								asprintf(&filelist, "%s%s", filelist, parse_file(reader, "", 0));
							}
							if( gCount > gMax )
								goto done;
                        }
                    }
                    if( ret == -1 ) {
                        xmlFreeTextReader(reader);
                        xmlDictCleanup();
                        xmlCleanupCharEncodingHandlers();
                        return -1;
                    }
                }
            }
            if( (type == XML_READER_TYPE_END_ELEMENT) && (strcmp((const char *)name, "toc")==0) ) {
                break;
            }
        }
        if( ret == -1 ) {
            xmlFreeTextReader(reader);
            xmlDictCleanup();
            xmlCleanupCharEncodingHandlers();
            break;
        }
    }
done:
	if( creatdate ) {
		CFStringAppendCString(textresp, "Creation Time:           ", kCFStringEncodingUTF8);
		CFStringAppendCString(textresp, creatdate, kCFStringEncodingUTF8);
		CFStringAppendCString(textresp, "\n", kCFStringEncodingUTF8);
	}
	CFStringAppendCString(textresp, "\nFile List:\n", kCFStringEncodingUTF8);
	CFStringAppendCString(textresp, filelist, kCFStringEncodingUTF8);
	free(filelist);
	dict = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
	if( !dict ) {
		return noErr;
	}
	
	CFDictionarySetValue(dict, kQLPreviewPropertyTextEncodingNameKey, CFSTR("UTF-8"));
	CFDictionarySetValue(dict, kQLPreviewPropertyMIMETypeKey, CFSTR("text/plain"));
	
	
	//qldata = CFStringCreateExternalRepresentation(NULL, htmlresp, kCFStringEncodingUTF8, 0);	
	//QLPreviewRequestSetDataRepresentation(preview, qldata, kUTTypeHTML, dict); 
	qldata = CFStringCreateExternalRepresentation(NULL, textresp, kCFStringEncodingUTF8, 0);
	QLPreviewRequestSetDataRepresentation(preview, qldata, kUTTypeUTF8PlainText, NULL);
	
	CFRelease(textresp);
	CFRelease(dict);
	CFRelease(qldata);
	
    return noErr;
}

void CancelPreviewGeneration(void* thisInterface, QLPreviewRequestRef preview)
{
    // implement only if supported
}
