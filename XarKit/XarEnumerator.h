#import <Foundation/Foundation.h>
#import <xar/xar.h>

@class XarArchive, XarFile, XarSubdoc, XarBase;

@interface XarEnumerator : NSEnumerator
{
  unsigned enumeratorType;
  xar_iter_t iterator;
  xar_file_t currentFile;
  const char *currentProp;
  xar_subdoc_t currentSubdoc;
  xar_base_t currentBase;
}

+ (id)fileEnumeratorWithArchive:(XarArchive *)archive;
+ (id)propEnumeratorWith:(XarBase *)base;
+ (id)subdocEnumeratorWithArchive:(XarArchive *)archive;

@end
