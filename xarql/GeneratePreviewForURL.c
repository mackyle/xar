#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

#include <xar/xar.h>

/* -----------------------------------------------------------------------------
   Generate a preview for file

   This function's job is to create preview for designated file
   ----------------------------------------------------------------------------- */

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options)
{
    CFMutableStringRef htmlresp, textresp;
	CFStringRef cfpathref;
	CFMutableDictionaryRef dict;
	CFDataRef qldata;
	char cpath[PATH_MAX];
	xar_t x;
	xar_file_t f;
	xar_iter_t i;
	
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
		
	x = xar_open(cpath, READ);
	if( !x )
		return noErr;
	
	textresp = CFStringCreateMutable(NULL, 0);
	htmlresp = CFStringCreateMutable(NULL, 0);
	if( !htmlresp ) {
		xar_close(x);
		return noErr;
	}
	
	CFStringAppendCString(htmlresp, "<html>\n", kCFStringEncodingUTF8);
	CFStringAppendCString(htmlresp, "<body>\n", kCFStringEncodingUTF8);
	CFStringAppendCString(htmlresp, "<pre>\n", kCFStringEncodingUTF8);
	
	i = xar_iter_new();
	for(f = xar_file_first(x, i); f; f = xar_file_next(i)) {
		char *path;
		path = xar_get_path(f);
		if( path ) {
			CFStringAppendCString(htmlresp, path, kCFStringEncodingUTF8);
			CFStringAppendCString(htmlresp, "\n", kCFStringEncodingUTF8);
			
			CFStringAppendCString(textresp, path, kCFStringEncodingUTF8);
			CFStringAppendCString(textresp, "\n", kCFStringEncodingUTF8);
			free(path);
		}
	}
	CFStringAppendCString(htmlresp, "</pre>", kCFStringEncodingUTF8);
	CFStringAppendCString(htmlresp, "</body>\n", kCFStringEncodingUTF8);
	CFStringAppendCString(htmlresp, "</html>\n", kCFStringEncodingUTF8);
	
	xar_close(x);
	
	
	dict = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
	if( !dict ) {
		CFRelease(htmlresp);
		return noErr;
	}
	
	CFDictionarySetValue(dict, kQLPreviewPropertyTextEncodingNameKey, CFSTR("UTF-8"));
	CFDictionarySetValue(dict, kQLPreviewPropertyMIMETypeKey, CFSTR("text/html"));
	
	
	//qldata = CFStringCreateExternalRepresentation(NULL, htmlresp, kCFStringEncodingUTF8, 0);	
	//QLPreviewRequestSetDataRepresentation(preview, qldata, kUTTypeHTML, dict); 
	qldata = CFStringCreateExternalRepresentation(NULL, textresp, kCFStringEncodingUTF8, 0);
	QLPreviewRequestSetDataRepresentation(preview, qldata, kUTTypeUTF8PlainText, NULL);
	
	CFRelease(htmlresp);
	CFRelease(textresp);
	CFRelease(dict);
	CFRelease(qldata);
	
    return noErr;
}

void CancelPreviewGeneration(void* thisInterface, QLPreviewRequestRef preview)
{
    // implement only if supported
}
