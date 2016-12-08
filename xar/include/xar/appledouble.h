/* Information pulled from:
 * "AppleSingle/AppleDouble Formats for Foreign Files Developer's Note"
 * (c) Apple Computer 1990
 * File assembled by Rob Braun (bbraun@synack.net)
 */

/*
 * Second pass - information pulled from RFC 1740
 * (Re)assembled by Jakub Kaszycki <kuba@kaszycki.net.pl>
 */

#ifndef __APPLEDOUBLE__
#define __APPLEDOUBLE__

#include <sys/types.h>
#if __STDC_VERSION__ >= 199901
#include <stdint.h>
#else
#include <inttypes.h>
#endif

#ifndef has_attribute
#define has_attribute(x) 0
#endif

#if !defined (__packed)
#if (defined (__GNUC__)) || (has_attribute (__packed__))
#define __packed __attribute__ ((__packed__))
#elif (has_attribute (packed))
#define __packed __attribute__ ((packed))
#else
#define __PRAGMA_PACKING
#pragma pack(1)
#define __packed
#endif
#endif

/* Structure of an AppleSingle file:
 *   ----------------------
 *   | AppleSingleHeader  |
 *   |--------------------|
 *   | ASH.entries # of   |
 *   | AppleSingleEntry   |
 *   | Descriptors        |
 *   |         1          |
 *   |         .          |
 *   |         .          |
 *   |         n          |
 *   |--------------------|
 *   |   Datablock 1      |
 *   |--------------------|
 *   |   Datablock 2      |
 *   |--------------------|
 *   |   Datablock n      |
 *   ----------------------
 */

/**
 * Header of an AppleSingle file.
 */
typedef struct AppleSingleHeader
{
  /**
   * Magic number.
   * 
   * @see APPLESINGLE_MAGIC
   * @See APPLEDOUBLE_MAGIC
   */
  uint32_t magic;

  /**
   * Format version.
   * 
   * @see APPLESINGLE_VERSION
   * @see APPLEDOUBLE_VERSION
   */
  uint32_t version;
  
  /**
   * Zero filled.
   */
  char filler[16];

  /**
   * Number of entries in file.
   */
  uint16_t entries;
} __packed AppleSingleHeader;

#define APPLESINGLE_MAGIC 0x00051600
#define APPLEDOUBLE_MAGIC 0x00051607

#define APPLESINGLE_VERSION 0x00020000
#define APPLEDOUBLE_VERSION 0x00020000

/**
 * Entry type.
 * 
 * Entries 1, 3, and 8 are typically created for all files.
 * Macintosh Icon entries are rare, since those are typically in the resource 
 * fork.
 */
typedef enum AppleSingleEntryID
{
  /**
   * Lowest ID reserved by Apple.
   * 
   * @see AS_ID_APPLE_HIGH
   */
  AS_ID_APPLE_LOW = 1,

  /**
   * Data fork.
   */
  AS_ID_DATA = 1,
  
  /**
   * Resource fork.
   */
  AS_ID_RESOURCE = 2,

  /**
   * Name of the file.
   */
  AS_ID_NAME = 3,

  /**
   * Standard Macintosh comment.
   */
  AS_ID_COMMENT = 4,

  /**
   * Standard Macintosh black and white icon.
   */
  AS_ID_BWICON = 5,

  /**
   * Standard Macintosh color icon.
   */
  AS_ID_COLORICON = 6,

  /* There is no 7 */

  /**
   * File creation date, modification date, etc.
   */
  AS_ID_DATES = 8,

  /**
   * Finder information.
   */
  AS_ID_FINDER = 9,

  /**
   * Macintosh file information, attributes, etc.
   */
  AS_ID_MAC = 10,

  /**
   * ProDOS file information.
   */
  AS_ID_PRODOS = 11,

  /**
   * MS-DOS file information.
   */
  AS_ID_MSDOS = 12,

  /**
   * AFP short name.
   */
  AS_ID_SHORTNAME = 13,

  /**
   * AFP file information.
   */
  AS_ID_AFPINFO = 14,

  /**
   * AFP directory ID.
   */
  AS_ID_AFPDIR = 15,

  /**
   * Highest ID value reserved by Apple.
   * 
   * @see AS_ID_APPLE_LOW
   */
  AS_ID_APPLE_HIGH = 0x7FFFFFFF
} AppleSingleEntryID;

/**
 * Entry in AppleSingle file.
 */
typedef struct AppleSingleEntry
{
  /**
   * Entry type ID.
   */
  AppleSingleEntryID entry_id : 32;

  /**
   * Data offset, measured from the beginning of file.
   */
  uint32_t offset;

  /**
   * Length of data, can be zero.
   */
  uint32_t length;
} __packed AppleSingleEntry;

/**
 * File Dates are stored as the # of seconds before or after
 * 12am Jan 1, 2000 GMT. Negative value means before, positive after.
 */
typedef struct MacTimes
{
  /**
   * File creation time.
   */
  int32_t creation;

  /**
   * File modification time.
   */
  int32_t modification;

  /**
   * File backup time.
   */
  int32_t backup;

  /**
   * File access time.
   */
  int32_t access;
} __packed MacTimes;

/**
 * Location in directory view in Finder.
 */
typedef struct FinderPoint
{
  /**
   * X coordinate.
   */
  int16_t x;

  /**
   * Y coordinate.
   */
  int16_t y;
} __packed FinderPoint;

/**
 * Flags of Finder information.
 * 
 * @see FinderInfo.flags
 */
typedef enum FinderFlags
{
  /**
   * File is on desktop (HFS only).
   */
  AS_FF_ON_DESKTOP = 0x1,

  /**
   * Color coding (3 bits).
   */
  AS_FF_MASK_COLOR = 0xE,

  /* 0x10 reserved for System 7 */

  /**
   * Reserved for System 7.
   */
  AS_FF_SWITCH_LAUNCH = 0x20,

  /**
   * Available to multiple users.
   */
  AS_FF_SHARED = 0x40,

  /**
   * Contains no INIT resources.
   */
  AS_FF_NOINITS = 0x80,

  /**
   * Finder has loaded bundle resources.
   */
  AS_FF_BEENINITED = 0x100,

  /* 0x200 reserved for System 7 */

  /**
   * File has a custom icon.
   */
  AS_FF_CUSTOM_ICON = 0x400,

  /**
   * File is a stationary pad.
   */
  AS_FF_STATIONARY = 0x800,

  /**
   * File can't be renamed using Finder.
   */
  AS_FF_NAMELOCKED = 0x1000,

  /**
   * File has a bundle.
   */
  AS_FF_HASBUNDLE = 0x2000,

  /**
   * File icon is invisible.
   */
  AS_FF_INVISIBLE = 0x4000,

  /**
   * File is an alias (System 7).
   */
  AS_FF_ALIAS = 0x8000,
} FinderFlags;

/**
 * Finder normal information.
 */
typedef struct FinderNInfo
{
  /**
   * Type of file.
   */
  char type[4];

  /**
   * Creator of file.
   */
  char creator[4];

  /**
   * File flags.
   */
  FinderFlags flags : 16;

  /**
   * Location of file in directory view.
   */
  FinderPoint location;

  /**
   * Folder index.
   */
  int16_t folder;
} __packed FinderNInfo;

/**
 * Finder extended information.
 */
typedef struct FinderXInfo
{
  /**
   * Icon number.
   */
  int16_t icon_id;

  /**
   * Spare.
   */
  int16_t unused[3];

  /**
   * Script flag and code.
   */
  int8_t script;

  /**
   * Reserved.
   */
  int8_t xflags;

  /**
   * Comment ID number.
   */
  int16_t comment;

  /**
   * Home directory ID.
   */
  int32_t putaway;
} __packed FinderXInfo;

/**
 * Black and white icon.
 */
typedef struct ASIconBW
{
  /**
   * 32 rows of 32 1-bit pixels (bit set = black, bit unset = white).
   */
  uint32_t bitrow[32];
} __packed ASIconBW;

/**
 * Color icon.
 */
typedef struct ASIconColor
{
  // TODO: RFC says nothing :(
} __packed ASIconColor;

/**
 * Finder information.
 */
typedef struct FinderInfo
{
  /**
   * Normal information.
   */
  FinderNInfo n_info;

  /**
   * Extended information.
   */
  FinderXInfo x_info;
} __packed FinderInfo;

/**
 * Macintosh info.
 */
typedef struct MacInfo
{
  /**
   * Filler, currently zeroed.
   */
  uint8_t filler[3];

  /**
   * File information as obtained by <code>PBGetFileInfo</code> or
   * <code>PBGetCatInfo</code>.
   */
  uint8_t fileinfo;
} __packed MacInfo;

/**
 * DOS file attributes.
 */
typedef enum DOSAttr
{
  /**
   * File is read only.
   */
  AS_DOS_READONLY = 0x1,

  /**
   * File is hidden.
   */
  AS_DOS_HIDDEN = 0x2,

  /**
   * File is a system file, some tools hide them.
   */
  AS_DOS_SYSTEM = 0x4,

  /**
   * File is a volume label.
   */
  AS_DOS_VOLID = 0x8,

  /**
   * File is a subdirectory.
   */
  AS_DOS_SUBDIR = 0x10,

  /**
   * New or modified - needs backup.
   */
  AS_DOS_ARCHIVE = 0x20,
} DOSAttr;

/**
 * ProDOS information.
 */
typedef struct ProDOSInfo
{
  /**
   * Access word.
   */
  uint16_t access;

  /**
   * Type of original file.
   */
  uint16_t filetype;

  /**
   * Auxiliary type of original file.
   */
  uint32_t auxtype;
} __packed ProDOSInfo;

/**
 * MS-DOS info.
 */
typedef struct MSDOSInfo
{
  /**
   * Zero filler.
   */
  uint8_t filler;

  /**
   * File attributes.
   */
  DOSAttr attr : 8;
} __packed MSDOSInfo;

typedef enum AFPAttr
{
  /**
   * File is invisible.
   */
  AS_AFP_INVISIBLE = 0x1,

  /**
   * Simultaneous user access allowed.
   */
  AS_AFP_MULTIUSER = 0x2,

  /**
   * System file.
   */
  AS_AFP_SYSTEM = 0x4,

  /**
   * New or modified (needs backup).
   */
  AS_AFP_BACKUPNEEDED = 0x40
} AFPAttr;

/**
 * AFP file information.
 */
typedef struct AFPInfo
{
  /**
   * Zero filler.
   */
  uint8_t filler[3];

  /**
   * Attributes.
   */
  AFPAttr attr : 8;
} __packed AFPInfo;

/**
 * AFP server directory ID.
 */
typedef struct AFPDirId
{
  /**
   * Directory ID.
   */
  uint32_t dir_id;
} __packed AFPDirId;

/* Entries can be placed in any order, although Apple recommends:
 * Place the data block (1) last.
 * Finder Info, File Dates Info, and Macintosh File Info first.
 * Allocate resource for entries in 4K blocks.
 */

/* AppleDouble files are simply AppleSingle files without the data fork.
 * The magic number is different as a read optimization. 
 */

#ifdef __PRAGMA_PACKING
#undef __PRAGMA_PACKING
#pragma pack()
#endif

#endif /* __APPLEDOUBLE__ */
