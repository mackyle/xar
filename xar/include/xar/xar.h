/*
 * Copyright (c) 2005 Rob Braun
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Rob Braun nor the names of his contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * 03-Apr-2005
 * DRI: Rob Braun <bbraun@opendarwin.org>
 */
/*
 * Portions Copyright 2006, Apple Computer, Inc.
 * Christopher Ryan <ryanc@apple.com>
 */

#ifndef _XAR_H_
#define _XAR_H_

#include <xar/config.h>

#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <zlib.h>
#include <libxml/hash.h>
#include <openssl/evp.h>

#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(4)

  /**
   * Regular header of xar archive.
   */
  typedef struct xar_header
  {
    /**
     * Magic number
     *
     * @see XAR_HEADER_MAGIC
     */
    uint32_t magic;

    /**
     * Size of header
     */
    uint16_t size;

    /**
     * Archive format version
     */
    uint16_t version;

    /**
     * Length of compressed TOC.
     */
    uint64_t toc_length_compressed;

    /**
     * Length of uncompressed TOC.
     */
    uint64_t toc_length_uncompressed;

    /**
     * Checksum algorithm. If set to XAR_CKSUM_OTHER, xar_header_ex is used
     * instead of xar_header.
     *
     * @see XAR_CKSUM_OTHER
     * @see xar_header_ex
     */
    uint32_t cksum_alg;
  } xar_header;

  /**
   * Extended header of xar archive.
   */
  typedef struct xar_header_ex
  {
    /**
     * Magic number.
     *
     * @see XAR_HEADER_MAGIC
     */
    uint32_t magic;

    /**
     * Size of header.
     */
    uint16_t size;

    /**
     * Xar format version.
     */
    uint16_t version;

    /**
     * Length of compressed TOC.
     */
    uint64_t toc_length_compressed;

    /**
     * Length of uncompressed TOC.
     */
    uint64_t toc_length_uncompressed;

    /**
     * Checksum algorithm, here must be set to XAR_CKSUM_OTHER, or otherwise
     * xar_header is used.
     *
     * @see XAR_CKSUM_OTHER
     * @see xar_header
     */
    uint32_t cksum_alg;

    /**
     * Name of custom TOC checksum type.
     */
    char toc_cksum_name[36];
  } xar_header_ex;

#pragma pack()

  /**
   * Compatibility typedef for <code>xar_header</code>.
   */
  typedef struct xar_header xar_header_t;

  /**
   * Compatibility typedef for <code>xar_header_ex</code>.
   */
  typedef struct xar_header_ex xar_header_ex_t;

/**
 * Magic number of a xar archive.
 */
#define XAR_HEADER_MAGIC 0x78617221

/**
 * Name of EA property.
 */
#define XAR_EA_FORK "ea"

/**
 * Indicates that there is no TOC checksum.
 */
#define XAR_CKSUM_NONE   0

/**
 * Indicates that TOC is verified using SHA-1 checksum.
 */
#define XAR_CKSUM_SHA1   1

/**
 * Indicates that TOC is verified using MD5 checksum.
 */
#define XAR_CKSUM_MD5    2

/**
 * Indicates that TOC uses a custom checksum.
 */
#define XAR_CKSUM_OTHER  3

  /**
   * Pointer typedef.
   */
  typedef struct xar_iter *xar_iter_t;

  /**
   * Pointer typedef.
   */
  typedef struct xar_archive *xar_archive_t;

  /**
   * Pointer typedef.
   */
  typedef struct xar_signature *xar_signature_t;

  /**
   * Pointer typedef.
   */
  typedef struct xar_x509cert *xar_x509cert_t;

  /**
   * Pointer typedef.
   */
  typedef struct xar_base *xar_base_t;

  /**
   * Pointer typedef.
   */
  typedef struct xar_file *xar_file_t;

  /**
   * Pointer typedef.
   */
  typedef struct xar_attr *xar_attr_t;

  /**
   * Pointer typedef.
   */
  typedef struct xar_prop *xar_prop_t;

  /**
   * Pointer typedef.
   */
  typedef struct xar_subdoc *xar_subdoc_t;

  /**
   * Pointer typedef.
   */
  typedef struct xar_ea *xar_ea_t;

  /**
   * Xar base object - implemented partially by <code>xar_archive</code> and
   * <code>xar_subdoc</code>, fully by <code>xar_file</code>.
   *
   * @since 2.0.0
   * @see xar_archive
   * @see xar_subdoc
   * @see xar_file
   */
  typedef struct xar_base
  {
    /**
     * Properties of this object.
     */
    xar_prop_t props;

    /**
     * Attributes of this object.
     */
    xar_attr_t attrs;

    /**
     * Prefix of this object.
     */
    const char *prefix;

    /**
     * Namespace of this object.
     */
    const char *ns;
  } xar_base;

  /**
   * An attribute of object.
   *
   * @see xar_base
   */
  typedef struct xar_attr
  {
    /**
     * Name of this attribute.
     */
    char *key;

    /**
     * Value of this attribute.
     */
    char *value;

    /**
     * Namespace of this attribute.
     */
    char *ns;

    /**
     * Next attribute in linked list.
     */
    xar_attr_t next;
  } xar_attr;

  /**
   * Property of an object.
   *
   * @see xar_base
   */
  typedef struct xar_prop
  {
    const char *key;
    const char *value;
    xar_prop_t parent;
    xar_prop_t children;
    xar_prop_t next;
    xar_attr_t attrs;
    xar_file_t file;
    const char *prefix;
    const char *ns;
  } xar_prop;

  typedef struct xar_subdoc
  {
    /**
     * Properties of this object.
     */
    xar_prop_t props;

    /**
     * Attributes of this object.
     */
    xar_attr_t attrs;

    /**
     * Prefix of this object.
     */
    const char *prefix;

    /**
     * Namespace of this object.
     */
    const char *ns;
    const char *blank1;           /* filler for xar_file_t compatibility */
    const char *blank2;           /* filler for xar_file_t compatibility */
    char blank3;            /* filler for xar_file_t compatibility */
    const char *name;
    xar_subdoc_t next;
    const char *value;            /* a subdoc should very rarely have a value */
    xar_archive_t x;
  } xar_subdoc;

  typedef struct xar_ea
  {
    xar_prop_t prop;
    xar_ea_t next;
  } xar_ea;

  typedef struct xar_file
  {
    /**
     * Properties of this object.
     */
    xar_prop_t props;

    /**
     * Attributes of this object.
     */
    xar_attr_t attrs;

    /**
     * Prefix of this object.
     */
    const char *prefix;

    /**
     * Namespace of this object.
     */
    const char *ns;
    const char *fspath;
    char parent_extracted;
    xar_file_t parent;
    xar_file_t children;
    xar_file_t next;
    xar_ea_t eas;
    uint64_t nexteaid;
  } xar_file;

  typedef struct xar_x509cert
  {
    uint8_t *content;
    int32_t len;
    xar_x509cert_t next;
  } xar_x509cert;

  /**
   * Information about where an error occured.
   */
  typedef struct xar_errctx
  {
    /**
     * Error message.
     */
    const char *str;

    /**
     * Last value of <code>errno</code> from system call.
     */
    int saved_errno;

    /**
     * File, which was being processed when the error occured.
     */
    xar_file_t file;

    /**
     * Value passed from <code>xar_register_errhandler</code>.
     *
     * @see xar_register_errhandler
     */
    void *usrctx;

    /**
     * Archive which was being processed when the error occured.
     */
    xar_archive_t x;
  } xar_errctx;

  typedef xar_errctx *xar_errctx_t;

  typedef struct
  {
    char *next_out;
    unsigned int avail_out;

    unsigned long long total_in;
    unsigned long long total_out;

    void *state;
  } xar_stream;

  typedef int32_t (*err_handler) (int32_t severit, int32_t instance,
                                  xar_errctx_t ctx, void *usrctx);
/* the signed_data must be allocated durring the callback and will be released by the xar lib after the callback */
  typedef int32_t (*xar_signer_callback) (xar_signature_t sig, void *context,
                                          uint8_t * data, uint32_t length,
                                          uint8_t ** signed_data,
                                          uint32_t * signed_len);

  typedef struct xar_signature
  {
    char *type;
    int32_t len;
    off_t offset;
    int32_t x509cert_count;
    xar_x509cert_t x509certs;
    xar_signature_t next;
    xar_signer_callback signer_callback;  /* callback for signing */
    void *callback_context;       /* context for callback */
    xar_archive_t x;
  } xar_signature;

  /**
   * An open xar archive.
   */
  typedef struct xar_archive
  {
    /**
     * Properties of this object.
     */
    xar_prop_t props;

    /**
     * Archive options.
     */
    xar_attr_t attrs;

    /**
     * Prefix.
     */
    const char *prefix;

    /**
     * Namespace.
     */
    const char *ns;

    /**
     * For compatibility with xar_file_t.
     *
     * @see xar_file_t
     */
    const char *filler1;

    /**
     * For compatibility with xar_file_t.
     *
     * @see xar_file_t
     */
    const char *filler2;

    /**
     * File forest.
     */
    xar_file_t files;

    /**
     * Name of archive we are operating on.
     */
    const char *filename;

    /**
     * Directory of the archive, used in creation.
     */
    char *dirname;

    /**
     * Open file descriptor for the archive.
     */
    int fd;

    /**
     * FD for temporary heap archive, used in creation.
     */
    int heap_fd;

    /**
     * Current offset within the heap.
     */
    off_t heap_offset;

    /**
     * Current length of the heap.
     */
    off_t heap_len;

    /**
     * Archive header.
     */
    xar_header_ex header;

    /**
     * Buffer for reading/writing compressed TOC.
     */
    void *readbuf;

    /**
     * Length of readbuf.
     */
    size_t readbuf_len;

    /**
     * Offset into readbuf for keeping track between callbacks.
     */
    size_t offset;

    /**
     * Current bytes read of the TOC.
     */
    size_t toc_count;

    /**
     * Zlib state for compressing/decompressing TOC.
     */
    z_stream zs;

    /**
     * Used for distinguishing absolute paths.
     */
    char *path_prefix;

    /**
     * Callback for errors/warnings.
     */
    err_handler ercallback;

    /**
     * Error callback context.
     */
    xar_errctx errctx;

    /**
     * Linked list of subdocuments.
     */
    xar_subdoc_t subdocs;

    /**
     * Linked list of signatures.
     */
    xar_signature_t signatures;

    /**
     * Last unique file ID used.
     */
    uint64_t last_fileid;

    /**
     * Hash for looking up hardlinked files (add).
     */
    xmlHashTablePtr ino_hash;

    /**
     * Hash for looking up hardlinked files (extract).
     */
    xmlHashTablePtr link_hash;

    /**
     * Hash for looking up checksums of files.
     */
    xmlHashTablePtr csum_hash;

    /**
     * Message digest generation context.
     */
    EVP_MD_CTX *toc_ctx;

    /**
     * If set to <code>1</code>, checksum checks are done.
     */
    bool docksum;

    /**
     * If set to <code>1</code>, warnings are skipped.
     */
    bool skipwarn;

    /**
     * If set to <code>1</code>, components are stripped.
     */
    bool stripcomps;

    /**
     * If set to <code>1</code>, data is extracted to stdout.
     */
    bool tostdout;
    
    /**
     * If set to <code>1</code>, RFC 6713 format is used in compressing TOC.
     */
    bool rfcformat;

    /**
     * Cached results of <code>stat</code>.
     */
    struct stat sbcache;
  } xar_archive;

  /**
   * Legacy alias, probable subject for future removal.
   *
   * @since 2.0.0
   * @deprecated Use xar_archive_t instead
   * @see xar_archive_t
   */
  typedef xar_archive_t xar_t;

  typedef struct xar_iter
  {
    void *iter;
    char *path;
    void *node;
    int nochild;
  } xar_iter;

#define READ 0
#define WRITE 1

/* xar stream return codes */
#define XAR_STREAM_OK   0
#define XAR_STREAM_END  1
#define XAR_STREAM_ERR -1

/* Valid xar options & values */
#define XAR_OPT_OWNERSHIP    "ownership"        /* setting owner/group behavior */
#define XAR_OPT_VAL_SYMBOLIC "symbolic" /* set owner/group based on names */
#define XAR_OPT_VAL_NUMERIC  "numeric"  /* set owner/group based on uid/gid */

#define XAR_OPT_TOCCKSUM	"toc-cksum"     /* set the toc checksum algorithm */
#define XAR_OPT_FILECKSUM	"file-chksum"   /* set the file checksum algorithm */
#define XAR_OPT_VAL_NONE   "none"
#define XAR_OPT_VAL_MD5    "md5"
#define XAR_OPT_VAL_SHA1   "sha1"
#define XAR_OPT_VAL_SHA224 "sha224"
#define XAR_OPT_VAL_SHA256 "sha256"
#define XAR_OPT_VAL_SHA384 "sha384"
#define XAR_OPT_VAL_SHA512 "sha512"
/* Actually any valid hash function name that returns non-NULL from EVP_get_digestbyname can be used as a value */

#define XAR_OPT_COMPRESSION    "compression"    /* set the file compression type */
#define XAR_OPT_COMPRESSIONARG "compression-arg"        /* set the compression opts */
#define XAR_OPT_VAL_GZIP       "gzip"
#define XAR_OPT_VAL_BZIP       "bzip2"
#define XAR_OPT_VAL_LZMA       "lzma"
#define XAR_OPT_VAL_XZ         "xz"

#define XAR_OPT_RSIZE          "rsize"  /* Read io buffer size */

#define XAR_OPT_COALESCE       "coalesce"       /* Coalesce identical heap blocks */
#define XAR_OPT_LINKSAME       "linksame"       /* Hardlink identical files */

#define XAR_OPT_PROPINCLUDE    "prop-include"   /* File property to include */
#define XAR_OPT_PROPEXCLUDE    "prop-exclude"   /* File property to exclude */

#define XAR_OPT_SAVESUID       "savesuid"       /* Preserve setuid/setgid bits */
#define XAR_OPT_VAL_TRUE       "true"
#define XAR_OPT_VAL_FALSE      "false"

#define XAR_OPT_RECOMPRESS     "recompress"     /* Allow recompression (true/false) */

#define XAR_OPT_EXTRACTSTDOUT  "extract-stdout" /* Extract data to stdout instead of file (true/false) */

#define XAR_OPT_STRIPCOMPONENTS "strip-components"      /* Number of components to strip on extraction (default 0) */

/* Use xar_open("",WRITE) to get an xar_archive_t to use xar_opt_get on to fetch this then xar_close it if an xar_archive_t is not otherwise available */
/* Older versions will return NULL from xar_open("",WRITE), current versions will not but only xar_opt_get and xar_close should be called on the result */
#define XAR_OPT_XARLIBVERSION   "xar-library-version"   /* Library's XAR_VERSION_NUM (as 0xVALUE string [%i scanf format, strtol base 0], get only) */

/* Note that this option is forced to true whenever XAR_CKSUM_OTHER is in effect */
#define XAR_OPT_RFC6713FORMAT  "rfc6713-format" /* Generate application/zlib instead of application/x-gzip encoding styles (true/false) */

/* xar signing algorithms */
#define XAR_SIG_SHA1RSA		1


/* xar error handler macros */
#define XAR_SEVERITY_DEBUG    1
#define XAR_SEVERITY_INFO     2
#define XAR_SEVERITY_NORMAL   3
#define XAR_SEVERITY_WARNING  4
#define XAR_SEVERITY_NONFATAL 5
#define XAR_SEVERITY_FATAL    6

#define XAR_ERR_ARCHIVE_CREATION   1
#define XAR_ERR_ARCHIVE_EXTRACTION 2

  xar_archive_t xar_open (const char *file, int32_t flags);
  int xar_close (xar_archive_t x);
  xar_file_t xar_add (xar_archive_t x, const char *path);

  xar_file_t xar_add_frombuffer (xar_archive_t x, xar_file_t parent, const char *name,
                                 char *buffer, size_t length);
  xar_file_t xar_add_folder (xar_archive_t x, xar_file_t f, const char *name,
                             struct stat *info);
  xar_file_t xar_add_frompath (xar_archive_t x, xar_file_t parent, const char *name,
                               const char *realpath);

  xar_file_t xar_add_from_archive (xar_archive_t x, xar_file_t parent,
                                   const char *name, xar_archive_t sourcearchive,
                                   xar_file_t sourcefile);

  int32_t xar_extract (xar_archive_t x, xar_file_t f);
  int32_t xar_extract_tofile (xar_archive_t x, xar_file_t f, const char *path);
  int32_t xar_extract_tobuffer (xar_archive_t x, xar_file_t f, char **buffer);
  int32_t xar_extract_tobuffersz (xar_archive_t x, xar_file_t f, char **buffer,
                                  size_t * size);
  int32_t xar_extract_tostream_init (xar_archive_t x, xar_file_t f,
                                     xar_stream * stream);
  int32_t xar_extract_tostream (xar_stream * stream);
  int32_t xar_extract_tostream_end (xar_stream * stream);

  int32_t xar_verify (xar_archive_t x, xar_file_t f);


  const char *xar_opt_get (xar_archive_t x, const char *option);
  int32_t xar_opt_set (xar_archive_t x, const char *option, const char *value);
  int32_t xar_opt_unset (xar_archive_t x, const char *option);

  int32_t xar_prop_set (xar_base_t f, const char *key, const char *value);
  int32_t xar_prop_create (xar_base_t f, const char *key, const char *value);
  int32_t xar_prop_get (const xar_base_t f, const char *key, const char **value);

  xar_iter_t xar_iter_new (void);
  void xar_iter_free (xar_iter_t i);

  const char *xar_prop_first (xar_file_t f, xar_iter_t i);
  const char *xar_prop_next (xar_iter_t i);

  void xar_prop_unset (xar_base_t f, const char *key);
  xar_file_t xar_file_first (xar_archive_t x, xar_iter_t i);
  xar_file_t xar_file_next (xar_iter_t i);

  const char *xar_attr_get (const xar_base_t f, const char *prop, const char *key);
  int32_t xar_attr_set (xar_base_t f, const char *prop, const char *key,
                        const char *value);
  const char *xar_attr_first (xar_base_t f, const char *prop, xar_iter_t i);
  const char *xar_attr_next (xar_iter_t i);

  xar_subdoc_t xar_subdoc_new (xar_archive_t x, const char *name);
  int32_t xar_subdoc_prop_set (xar_subdoc_t s, const char *key,
                               const char *value);
  int32_t xar_subdoc_prop_get (xar_subdoc_t s, const char *key,
                               const char **value);
  int32_t xar_subdoc_attr_set (xar_subdoc_t s, const char *prop,
                               const char *key, const char *value);
  const char *xar_subdoc_attr_get (xar_subdoc_t s, const char *prop,
                                   const char *key);
  xar_subdoc_t xar_subdoc_first (xar_archive_t x);
  xar_subdoc_t xar_subdoc_next (xar_subdoc_t s);
  const char *xar_subdoc_name (xar_subdoc_t s);
  int32_t xar_subdoc_copyout (xar_subdoc_t s, unsigned char **,
                              unsigned int *);
  int32_t xar_subdoc_copyin (xar_subdoc_t s, const unsigned char *,
                             unsigned int);
  void xar_subdoc_remove (xar_subdoc_t s);

/* signature api for adding various signature types */
  xar_signature_t xar_signature_new (xar_archive_t x, const char *type,
                                     int32_t length,
                                     xar_signer_callback callback,
                                     void *callback_context);

  const char *xar_signature_type (xar_signature_t s);

  xar_signature_t xar_signature_first (xar_archive_t x);
  xar_signature_t xar_signature_next (xar_signature_t s);

  int32_t xar_signature_add_x509certificate (xar_signature_t sig,
                                             const uint8_t * cert_data,
                                             uint32_t cert_len);

  int32_t xar_signature_get_x509certificate_count (xar_signature_t sig);
  int32_t xar_signature_get_x509certificate_data (xar_signature_t sig,
                                                  int32_t index,
                                                  const uint8_t ** cert_data,
                                                  uint32_t * cert_len);

  int32_t xar_signature_copy_signed_data (xar_signature_t sig,
                                          uint8_t ** data, uint32_t * length,
                                          uint8_t ** signed_data,
                                          uint32_t * signed_length,
                                          uint64_t * signed_offset);

/* Helper functions - caller must free returned memory */
  char *xar_get_size (xar_archive_t x, xar_file_t f);
  char *xar_get_type (xar_archive_t x, xar_file_t f);
  char *xar_get_mode (xar_archive_t x, xar_file_t f);
  char *xar_get_owner (xar_archive_t x, xar_file_t f);
  char *xar_get_group (xar_archive_t x, xar_file_t f);
  char *xar_get_mtime (xar_archive_t x, xar_file_t f);

/* These are for xar modules and should never be needed from a calling app */
  void xar_register_errhandler (xar_archive_t x, err_handler callback, void *usrctx);
  xar_archive_t xar_err_get_archive (xar_errctx_t ctx);
  xar_file_t xar_err_get_file (xar_errctx_t ctx);
  const char *xar_err_get_string (xar_errctx_t ctx);
  int xar_err_get_errno (xar_errctx_t ctx);
  void xar_err_set_file (xar_archive_t x, xar_file_t f);
  void xar_err_set_string (xar_archive_t x, const char *str);
  void xar_err_set_errno (xar_archive_t x, int e);
  void xar_err_new (xar_archive_t x);
  int32_t xar_err_callback (xar_archive_t x, int32_t sev, int32_t err);

  void xar_serialize (xar_archive_t x, const char *file);
  char *xar_get_path (xar_file_t f);
  uint64_t xar_get_heap_offset (xar_archive_t x);
  uint64_t xar_ntoh64 (uint64_t num);

#ifdef __cplusplus
}
#endif

#endif                          /* _XAR_H_ */
