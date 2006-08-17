#import <Foundation/Foundation.h>
#import <xar/xar.h>

@class XarArchive, XarFile;

@interface XarEnumerator : NSEnumerator
{
	unsigned enumeratorType;
	xar_iter_t iterator;
	xar_file_t currentFile;
	const char *currentProp;
	xar_subdoc_t currentSubdoc;
}

+ (id)fileEnumeratorWithArchive:(XarArchive *)archive;
+ (id)propEnumeratorWithFile:(XarFile *)archive;
+ (id)subdocEnumeratorWithArchive:(XarArchive *)archive;

@end
