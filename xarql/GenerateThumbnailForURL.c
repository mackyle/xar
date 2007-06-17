#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

#include <xar/xar.h>

/* -----------------------------------------------------------------------------
    Generate a thumbnail for file

   This function's job is to create thumbnail for designated file as fast as possible
   ----------------------------------------------------------------------------- */
#define XFSIZE 25
#define XWIDTH 300
#define XHEIGHT 400
OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize)
{
	CGContextRef ctx;
	CGSize sz;
	CGRect r;
	xar_t x;
	xar_file_t f;
	xar_iter_t i;
	char cpath[PATH_MAX];
	CFStringRef cfpathref;
	float y;
	
	cfpathref = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
	if( !cfpathref )
		return noErr;
	memset(cpath, 0, sizeof(cpath));
	if( CFStringGetCString(cfpathref, cpath, sizeof(cpath)-1, kCFStringEncodingUTF8) == FALSE) {
		CFRelease(cfpathref);
		return noErr;
	}
	CFRelease(cfpathref);
	
	sz.height = XHEIGHT;
	sz.width = XWIDTH;
	ctx = QLThumbnailRequestCreateContext(thumbnail, sz, true, NULL);
	if( !ctx )
		return noErr;
	
	r.origin.x = 0;
	r.origin.y = 0;
	r.size = sz;
	CGContextSetRGBFillColor(ctx, 0, 0, 0, 1);
	CGContextFillRect(ctx, r);
	
	CGContextSetShouldAntialias(ctx, true);
	CGContextSetShouldSmoothFonts(ctx, false);
	CGContextSelectFont(ctx, "Times", XFSIZE, kCGEncodingMacRoman);
	CGContextSetCharacterSpacing(ctx, 10);
	CGContextSetTextDrawingMode (ctx, kCGTextFillStroke);	
	CGContextSetRGBFillColor (ctx, 0, 1, 0, 1);
	CGContextSetRGBStrokeColor (ctx, 0, 1, 0, 1);

	y = sz.height - XFSIZE;
	CGContextShowTextAtPoint(ctx, (sz.width-(4*XFSIZE))/2, y, "xar!", 4);

	x = xar_open(cpath, READ);
	if( !x )
		return noErr;
		
	i = xar_iter_new();
	for(f = xar_file_first(x, i); f; f = xar_file_next(i)) {
		char *path;
		path = xar_get_path(f);
		if( path ) {
			y -= XFSIZE;
			if( y <= 0 ) {
				free(path);
				break;
			}
			CGContextShowTextAtPoint(ctx, 0, y, path, strlen(path));
			free(path);
		}
	}
	
	xar_close(x);

	QLThumbnailRequestFlushContext(thumbnail, ctx);
	return noErr;
}

void CancelThumbnailGeneration(void* thisInterface, QLThumbnailRequestRef thumbnail)
{
    // implement only if supported
}
