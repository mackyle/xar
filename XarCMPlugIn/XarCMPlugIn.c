// =============================================================================
//	XarCMPlugIn.c
// =============================================================================

// Uncomment ONE of these lines
// #define DEBUGSTR(s) DebugStr(s)
#define DEBUGSTR(s)

// =============================================================================

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPlugInCOM.h>
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>
#include <unistd.h>
#include <libgen.h>
#include <fts.h>
#include <xar/xar.h>

static void archivedir(char *path);
static void extractdir(char *path);
// =============================================================================
//	typedefs, structs, consts, enums, etc.
// =============================================================================

#define kXarCMPlugInFactoryID	( CFUUIDGetConstantUUIDWithBytes( NULL,		\
																   0xC5, 0x2C, 0x25, 0x66, 0x3D, 0xC1, 0x11, 0xD5,	\
																   0xBB, 0xA3, 0x00, 0x30, 0x65, 0xB3, 0x00, 0xBC))
// "C52C2566-3DC1-11D5-BBA3-003065B300BC"

// The layout for an instance of XarCMPlugInType.
typedef struct XarCMPlugInType
{
	ContextualMenuInterfaceStruct	*cmInterface;
	CFUUIDRef						factoryID;
	UInt32							refCount;
} XarCMPlugInType;

// =============================================================================
//	local function prototypes
// =============================================================================

// =============================================================================
//	interface function prototypes
// =============================================================================
static HRESULT XarCMPlugInQueryInterface(void* thisInstance,REFIID iid,LPVOID* ppv);
static ULONG XarCMPlugInAddRef(void *thisInstance);
static ULONG XarCMPlugInRelease(void *thisInstance);

static OSStatus XarCMPlugInExamineContext(void* thisInstance,const AEDesc* inContext,AEDescList* outCommandPairs);
static OSStatus XarCMPlugInHandleSelection(void* thisInstance,AEDesc* inContext,SInt32 inCommandID);
static void XarCMPlugInPostMenuCleanup( void *thisInstance );

// =============================================================================
//	local function implementations
// =============================================================================

// -----------------------------------------------------------------------------
//	DeallocXarCMPlugInType
// -----------------------------------------------------------------------------
//	Utility function that deallocates the instance when
//	the refCount goes to zero.
//
static void DeallocXarCMPlugInType( XarCMPlugInType* thisInstance )
{
	CFUUIDRef	theFactoryID = thisInstance->factoryID;

	DEBUGSTR("\p|DeallocXarCMPlugInType-I-Debug;g");

	free( thisInstance );
	if ( theFactoryID )
	{
		CFPlugInRemoveInstanceForFactory( theFactoryID );
		CFRelease( theFactoryID );
	}
}

// -----------------------------------------------------------------------------
//	testInterfaceFtbl	definition
// -----------------------------------------------------------------------------
//	The TestInterface function table.
//
static ContextualMenuInterfaceStruct testInterfaceFtbl =
{
	// Required padding for COM
	NULL,

	// These three are the required COM functions
	XarCMPlugInQueryInterface,
	XarCMPlugInAddRef,
	XarCMPlugInRelease,

	// Interface implementation
	XarCMPlugInExamineContext,
	XarCMPlugInHandleSelection,
	XarCMPlugInPostMenuCleanup
};

// -----------------------------------------------------------------------------
//	AllocXarCMPlugInType
// -----------------------------------------------------------------------------
//	Utility function that allocates a new instance.
//
static XarCMPlugInType* AllocXarCMPlugInType(CFUUIDRef		inFactoryID )
{
	// Allocate memory for the new instance.
	XarCMPlugInType *theNewInstance;

	DEBUGSTR("\p|AllocXarCMPlugInType-I-Debug;g");

	theNewInstance = (XarCMPlugInType*) malloc(sizeof( XarCMPlugInType ) );

	// Point to the function table
	theNewInstance->cmInterface = &testInterfaceFtbl;

	// Retain and keep an open instance refcount<
	// for each factory.
	theNewInstance->factoryID = CFRetain( inFactoryID );
	CFPlugInAddInstanceForFactory( inFactoryID );

	// This function returns the IUnknown interface
	// so set the refCount to one.
	theNewInstance->refCount = 1;
	return theNewInstance;
}

// -----------------------------------------------------------------------------
//	XarCMPlugInFactory
// -----------------------------------------------------------------------------
//	Implementation of the factory function for this type.
//
extern void* XarCMPlugInFactory(CFAllocatorRef allocator,CFUUIDRef typeID );
void* XarCMPlugInFactory(CFAllocatorRef allocator,CFUUIDRef typeID )
{
#pragma unused (allocator)

	DEBUGSTR("\p|XarCMPlugInFactory-I-Debug;g");

	// If correct type is being requested, allocate an
	// instance of TestType and return the IUnknown interface.
	if ( CFEqual( typeID, kContextualMenuTypeID ) )
	{
		XarCMPlugInType *result;
		result = AllocXarCMPlugInType(kXarCMPlugInFactoryID);
		return result;
	}
	else
	{
		// If the requested type is incorrect, return NULL.
		return NULL;
	}
}

// -----------------------------------------------------------------------------
//	AddCommandToAEDescList
// -----------------------------------------------------------------------------
static OSStatus AddCommandToAEDescList(CFStringRef	inCommandCFStringRef,
									   SInt32		inCommandID,
									   AEDescList*	ioCommandList)
{
	OSStatus anErr = noErr;
	AERecord theCommandRecord = { typeNull, NULL };
	CFIndex length = CFStringGetLength(inCommandCFStringRef);
    const UniChar* dataPtr = CFStringGetCharactersPtr(inCommandCFStringRef);
	const UniChar* tempPtr = nil;

	//	printf("\nXarCMPlugIn->AddCommandToAEDescList: Trying to add an item." );

    if (dataPtr == NULL)
	{
		tempPtr = (UniChar*) NewPtr(length * sizeof(UniChar));
		if (nil == tempPtr)
			goto AddCommandToAEDescList_fail;

		CFStringGetCharacters(inCommandCFStringRef, CFRangeMake(0,length), (UniChar*) tempPtr);
		dataPtr = tempPtr;
	}
	if (nil == dataPtr)
		goto AddCommandToAEDescList_fail;

	// create an apple event record for our command
	anErr = AECreateList( NULL, 0, true, &theCommandRecord );
	require_noerr( anErr, AddCommandToAEDescList_fail );

	// stick the command text into the aerecord
	anErr = AEPutKeyPtr( &theCommandRecord, keyAEName, typeUnicodeText, dataPtr, length * sizeof(UniChar));
	require_noerr( anErr, AddCommandToAEDescList_fail );

	// stick the command ID into the AERecord
	anErr = AEPutKeyPtr( &theCommandRecord, keyContextualMenuCommandID,
					  typeLongInteger, &inCommandID, sizeof( inCommandID ) );
	require_noerr( anErr, AddCommandToAEDescList_fail );

	// stick this record into the list of commands that we are
	// passing back to the CMM
	anErr = AEPutDesc(ioCommandList, 			// the list we're putting our command into
				   0, 						// stick this command onto the end of our list
				   &theCommandRecord );	// the command I'm putting into the list

AddCommandToAEDescList_fail:
		// clean up after ourself; dispose of the AERecord
		AEDisposeDesc( &theCommandRecord );

	if (nil != tempPtr)
		DisposePtr((Ptr) tempPtr);

    return anErr;

} // AddCommandToAEDescList

// =============================================================================
//	interface function implementations
// =============================================================================

// -----------------------------------------------------------------------------
//	XarCMPlugInQueryInterface
// -----------------------------------------------------------------------------
//	Implementation of the IUnknown QueryInterface function.
//
static HRESULT XarCMPlugInQueryInterface(void* thisInstance,REFIID iid,LPVOID* ppv)
{
	// Create a CoreFoundation UUIDRef for the requested interface.
	CFUUIDRef	interfaceID = CFUUIDCreateFromUUIDBytes( NULL, iid );

	DEBUGSTR("\p|XarCMPlugInQueryInterface-I-Debug;g");

	// Test the requested ID against the valid interfaces.
	if ( CFEqual( interfaceID, kContextualMenuInterfaceID ) )
	{
		// If the TestInterface was requested, bump the ref count,
		// set the ppv parameter equal to the instance, and
		// return good status.
  //		((XarCMPlugInType*) thisInstance )->cmInterface->AddRef(thisInstance );
		XarCMPlugInAddRef(thisInstance);

		*ppv = thisInstance;
		CFRelease( interfaceID );
		return S_OK;
	}
	else if ( CFEqual( interfaceID, IUnknownUUID ) )
	{
		// If the IUnknown interface was requested, same as above.
  //		( ( XarCMPlugInType* ) thisInstance )->cmInterface->AddRef(thisInstance );
		XarCMPlugInAddRef(thisInstance);

		*ppv = thisInstance;
		CFRelease( interfaceID );
		return S_OK;
	}
	else
	{
		// Requested interface unknown, bail with error.
		*ppv = NULL;
		CFRelease( interfaceID );
		return E_NOINTERFACE;
	}
}

// -----------------------------------------------------------------------------
//	XarCMPlugInAddRef
// -----------------------------------------------------------------------------
//	Implementation of reference counting for this type. Whenever an interface
//	is requested, bump the refCount for the instance. NOTE: returning the
//	refcount is a convention but is not required so don't rely on it.
//
static ULONG XarCMPlugInAddRef( void *thisInstance )
{
	DEBUGSTR("\p|XarCMPlugInAddRef-I-Debug;g");

	( ( XarCMPlugInType* ) thisInstance )->refCount += 1;
	return ( ( XarCMPlugInType* ) thisInstance)->refCount;
}

// -----------------------------------------------------------------------------
// XarCMPlugInRelease
// -----------------------------------------------------------------------------
//	When an interface is released, decrement the refCount.
//	If the refCount goes to zero, deallocate the instance.
//
static ULONG XarCMPlugInRelease( void *thisInstance )
{
	DEBUGSTR("\p|XarCMPlugInRelease-I-Debug;g");

	((XarCMPlugInType*) thisInstance )->refCount -= 1;
	if (((XarCMPlugInType*) thisInstance)->refCount == 0)
	{
		DeallocXarCMPlugInType((XarCMPlugInType*) thisInstance);
		return 0;
	}
	else
	{
		return ((XarCMPlugInType*) thisInstance)->refCount;
	}
}

// -----------------------------------------------------------------------------
//	XarCMPlugInExamineContext
// -----------------------------------------------------------------------------
//	The implementation of the ExamineContext test interface function.
//

static OSStatus XarCMPlugInExamineContext(	void*				thisInstance,
											  const AEDesc*		inContext,
											  AEDescList*			outCommandPairs)
{
	DEBUGSTR("\p|XarCMPlugInExamineContext-I-Debug;g");

	// this is a quick sample that looks for text in the context descriptor

	// make sure the descriptor isn't null
	if ( inContext != NULL )
	{
		int i;
		long num;

		//AddCommandToAEDescList( CFSTR("-"), ++gNumCommandIDs,outCommandPairs );

		AECountItems(inContext, &num);
		for(i = 1; i <= num; i++) {
			AEDesc tae;
			OSErr e;
			Size dsize;
			AliasHandle ah;
			FSRef fsr;
			CFURLRef url;
			CFStringRef cfpath, base, dispstr;
			char *path;
			Boolean changed;
			int pathlen;

			e = AEGetNthDesc(inContext, i, typeAlias, NULL, &tae);
			if( e != noErr ) continue;

			dsize = AEGetDescDataSize(&tae);
			ah = (AliasHandle)NewHandle(dsize);
			if( ah == nil ) continue;
			e = AEGetDescData(&tae, *ah, dsize);
			if( e != noErr ) continue;

			e = FSResolveAlias(NULL, ah, &fsr, &changed);
			if( e != noErr ) continue;

			url = CFURLCreateFromFSRef(kCFAllocatorDefault, &fsr);
			base = CFURLCopyLastPathComponent(url);

			cfpath = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
			CFRelease(url);
			if( cfpath == NULL ) {
				fprintf(stderr, "cfpath null\n");
				CFRelease(base);
				continue;
			}
			pathlen = CFStringGetLength(cfpath);
			pathlen *= 2;
			path = malloc(pathlen);
			if( path == NULL ) {
				fprintf(stderr, "path null\n");
				CFRelease(base);
				CFRelease(cfpath);
				continue;
			}
			memset(path, 0, pathlen);
			CFStringGetCString(cfpath, path, (CFIndex)pathlen, kCFStringEncodingUTF8);
			if( strcmp(path+strlen(path)-4, ".xar") == 0 ) {
				dispstr = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("Xar Extract '%@'"), base);
				AddCommandToAEDescList( dispstr, 1, outCommandPairs );
			} else {
				dispstr = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("Xar Archive '%@'"), base);
				AddCommandToAEDescList( dispstr, 2, outCommandPairs );
			}
			CFRelease(base);
			CFRelease(dispstr);
			CFRelease(cfpath);
			free(path);
			
		}
	}

	return noErr;
}

// -----------------------------------------------------------------------------
//	HandleSelection
// -----------------------------------------------------------------------------
//	The implementation of the HandleSelection test interface function.
//
static OSStatus XarCMPlugInHandleSelection(void*				thisInstance,
											  AEDesc*				inContext,
											  SInt32				inCommandID )
{
	DEBUGSTR("\p|XarCMPlugInHandleSelection-I-Debug;g");

	// this is a quick sample that looks for text in the context descriptor
	if ( inContext != NULL )
	{
		int i;
		long num;

		AECountItems(inContext, &num);
		for(i = 1; i <= num; i++) {
			AEDesc tae;
			OSErr e;
			Size dsize;
			AliasHandle ah;
			FSRef fsr;
			CFURLRef url;
			CFStringRef cfpath, base;
			char *path;
			Boolean changed;
			int pathlen;

			e = AEGetNthDesc(inContext, i, typeAlias, NULL, &tae);
			if( e != noErr ) continue;

			dsize = AEGetDescDataSize(&tae);
			ah = (AliasHandle)NewHandle(dsize);
			if( ah == nil ) continue;
			e = AEGetDescData(&tae, *ah, dsize);
			if( e != noErr ) continue;

			e = FSResolveAlias(NULL, ah, &fsr, &changed);
			if( e != noErr ) continue;

			url = CFURLCreateFromFSRef(kCFAllocatorDefault, &fsr);
			base = CFURLCopyLastPathComponent(url);

			cfpath = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
			CFRelease(url);
			if( cfpath == NULL ) {
				fprintf(stderr, "cfpath null\n");
				CFRelease(base);
				continue;
			}
			pathlen = CFStringGetLength(cfpath);
			pathlen *= 2;
			path = malloc(pathlen);
			if( path == NULL ) {
				fprintf(stderr, "path null\n");
				CFRelease(base);
				CFRelease(cfpath);
				continue;
			}
			memset(path, 0, pathlen);
			CFStringGetCString(cfpath, path, (CFIndex)pathlen, kCFStringEncodingUTF8);
			if( inCommandID == 2 ) {
				archivedir(path);
			}
			if( inCommandID == 1 ) {
				extractdir(path);
			}

			CFRelease(base);
			CFRelease(cfpath);
			free(path);
			
		}
	}

	return noErr;
}	// XarCMPlugInHandleSelection

// -----------------------------------------------------------------------------
//	PostMenuCleanup
// -----------------------------------------------------------------------------
//	The implementation of the PostMenuCleanup test interface function.
//
static void XarCMPlugInPostMenuCleanup( void *thisInstance )
{
	DEBUGSTR("\p|XarCMPlugInPostMenuCleanup-I-Debug;g");

	// No need to clean up.  We are a tidy folk.
}

static void print_file(xar_file_t f) {
	char *path = xar_get_path(f);
	printf("%s\n", path);
	free(path);
}

static int Err = 0;

static int32_t err_callback(int32_t sev, int32_t err, xar_errctx_t ctx, void *usrctx) {
	xar_file_t f;
	const char *str;
	int e;
	
	f = xar_err_get_file(ctx);
	str = xar_err_get_string(ctx);
	e = xar_err_get_errno(ctx);

	switch(sev) {
	case XAR_SEVERITY_DEBUG:
	case XAR_SEVERITY_INFO:
		break;
	case XAR_SEVERITY_WARNING:
		printf("%s\n", str);
		break;
	case XAR_SEVERITY_NORMAL:
		if( (err = XAR_ERR_ARCHIVE_CREATION) && f )
			print_file(f);
			break;
	case XAR_SEVERITY_NONFATAL:
	case XAR_SEVERITY_FATAL:
		Err = 2;
		printf("Error while ");
		if( err == XAR_ERR_ARCHIVE_CREATION ) printf("creating");
		if( err == XAR_ERR_ARCHIVE_EXTRACTION ) printf("extracting");
		printf(" archive");
		if( f ) {
			const char *file = xar_get_path(f);
			if( file ) printf(":(%s)", file);
			free((char *)file);
		}
		if( str ) printf(": %s", str);
		if( err ) printf(" (%s)", strerror(e));
		if( sev == XAR_SEVERITY_NONFATAL ) printf(" - ignored");
		printf("\n");
		break;
	}
	return 0;
}

static void archivedir(char *path) {
	xar_t x;
	FTS *fts;
	FTSENT *ent;
	char *file, *dir, *tmp, *aname;
	char cwd[4096];
	char *args[2];
	
	memset(cwd, 0, sizeof(cwd));
	getcwd(cwd, sizeof(cwd)-1);

	tmp = strdup(path);
	dir = dirname(tmp);
	file = basename(path);

	if( chdir(dir) == -1 ) {
		printf("Unable to chdir to %s\n", dir);
		free(tmp);
		free(aname);
		return;
	}
	asprintf(&aname, "%s.xar", file);

	x = xar_open(aname, WRITE);
	if( !x ) {
		printf("Error creating archive %s\n", aname);
		free(tmp);
		free(aname);
		return;
	}

	xar_register_errhandler(x, err_callback, NULL);

	args[0] = file;
	args[1] = NULL;
	fts = fts_open(args, FTS_PHYSICAL|FTS_NOSTAT|FTS_NOCHDIR, NULL);
	if( !fts ) {
		printf("Error traversing file tree\n");
		free(tmp);
		free(aname);
		return;
	}

	while( (ent = fts_read(fts)) ) {
		if( ent->fts_info == FTS_DP )
			continue;
		xar_add(x, ent->fts_path);
	}
	fts_close(fts);
	xar_close(x);
	free(tmp);
	free(aname);
	chdir(cwd);
	return;
}

static void extractdir(char *path) {
	xar_t x;
	xar_iter_t i;
	xar_file_t f;
	char cwd[4096];
	char *dir, *tmp;
	
	memset(cwd, 0, sizeof(cwd));
	getcwd(cwd, sizeof(cwd)-1);

	tmp = strdup(path);
	dir = dirname(tmp);

	if( chdir(dir) == -1 ) {
		printf("Unable to chdir to %s\n", dir);
		free(tmp);
		return;
	}
	free(tmp);

	x = xar_open(path, READ);
	if( !x ) {
		printf("Error opening archive: %s\n", path);
		return;
	}

	xar_register_errhandler(x, err_callback, NULL);

	i = xar_iter_new();
	for(f = xar_file_first(x, i); f; f = xar_file_next(i)) {
		xar_extract(x, f);
	}
	xar_iter_free(i);
	xar_close(x);
	chdir(cwd);
	return;
}
