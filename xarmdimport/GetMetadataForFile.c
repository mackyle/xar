#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h> 

#include <xar/xar.h>

/* -----------------------------------------------------------------------------
   Step 1
   Set the UTI types the importer supports
  
   Modify the CFBundleDocumentTypes entry in Info.plist to contain
   an array of Uniform Type Identifiers (UTI) for the LSItemContentTypes 
   that your importer can handle
  
   ----------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
   Step 2 
   Implement the GetMetadataForFile function
  
   Implement the GetMetadataForFile function below to scrape the relevant
   metadata from your document and return it as a CFDictionary using standard keys
   (defined in MDItem.h) whenever possible.
   ----------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
   Step 3 (optional) 
   If you have defined new attributes, update the schema.xml file
  
   Edit the schema.xml file to include the metadata keys that your importer returns.
   Add them to the <allattrs> and <displayattrs> elements.
  
   Add any custom types that your importer requires to the <attributes> element
  
   <attribute name="com_mycompany_metadatakey" type="CFString" multivalued="true"/>
  
   ----------------------------------------------------------------------------- */



/* -----------------------------------------------------------------------------
    Get metadata attributes from file
   
   This function's job is to extract useful information your file format supports
   and return it as a dictionary
   ----------------------------------------------------------------------------- */

Boolean GetMetadataForFile(void* thisInterface, 
			   CFMutableDictionaryRef attributes, 
			   CFStringRef contentTypeUTI,
			   CFStringRef pathToFile)
{
    /* Pull any available metadata from the file at the specified path */
    /* Return the attribute keys and attribute values in the dict */
    /* Return TRUE if successful, FALSE if there was no data provided */
    char cpath[PATH_MAX];
	xar_t x;
	xar_file_t f;
	xar_iter_t i;
	CFMutableStringRef filelist;

	if( CFStringGetCString(pathToFile, cpath, sizeof(cpath), kCFStringEncodingUTF8) == FALSE )
		return FALSE;
	
	x = xar_open(cpath, READ);
	if( !x )
		return FALSE;
	
	i = xar_iter_new();
	if( !i )
		return FALSE;
		
	filelist = CFStringCreateMutable(NULL, 0);	
	for(f = xar_file_first(x, i); f; f = xar_file_next(i)) {
		const char *path;
		path = xar_get_path(f);
		CFStringAppendCString(filelist, " ", kCFStringEncodingUTF8);
		CFStringAppendCString(filelist, path, kCFStringEncodingUTF8);
	}
	CFDictionarySetValue(attributes, kMDItemTextContent, filelist);
	CFDictionarySetValue(attributes, kMDItemKind, CFSTR("XAR!"));
	CFDictionarySetValue(attributes, kMDItemCreator, CFSTR("xar"));
	CFRelease(filelist);
	
	xar_close(x);
	return TRUE;
}
