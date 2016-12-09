/*
 * Copyright (c) 2005-2007 Rob Braun
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
 * DRI: Rob Braun <bbraun@synack.net>
 */
/*
 * Portions Copyright 2006, Apple Computer, Inc.
 * Christopher Ryan <ryanc@apple.com>
 */
/*
 * Minor modified by Jakub Kaszycki, 2016
 * <kuba@kaszycki.net.pl>
 */

#ifndef _XAR_ARCHIVE_H_
#define _XAR_ARCHIVE_H_
#include <zlib.h>
#include <libxml/hash.h>
#include <openssl/evp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <xar/xar.h>
#include "filetree.h"

/**
 * Minimal buffer size in Xar archive stream.
 */
#define XAR_MINIMUM_BUFFER_SIZE 512

/**
 * Default, recommended buffer size in Xar archive stream.
 */
#define XAR_DEFAULT_BUFFER_SIZE 32768

/**
 * Information about where an error occured.
 */
struct errctx
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
  xar_t x;
};

struct __xar_t
{
  xar_prop_t props;
  xar_attr_t attrs;             /* archive options, such as rsize */
  const char *prefix;
  const char *ns;
  const char *filler1;
  const char *filler2;
  xar_file_t files;             /* file forest */
  const char *filename;         /* name of the archive we are operating on */
  char *dirname;                /* directory of the archive, used in creation */
  int fd;                       /* open file descriptor for the archive */
  int heap_fd;                  /* fd for tmp heap archive, used in creation */
  off_t heap_offset;            /* current offset within the heap */
  off_t heap_len;               /* current length of the heap */
  xar_header_ex_t header;       /* header of the xar archive */
  void *readbuf;                /* buffer for reading/writing compressed toc */
  size_t readbuf_len;           /* length of readbuf */
  size_t offset;                /* offset into readbuf for keeping track
                                 * between callbacks. */
  size_t toc_count;             /* current bytes read of the toc */
  z_stream zs;                  /* gz state for compressing/decompressing toc */
  char *path_prefix;            /* used for distinguishing absolute paths */
  err_handler ercallback;       /* callback for errors/warnings */
  struct errctx errctx;         /* error callback context */
  xar_subdoc_t subdocs;         /* linked list of subdocs */
  xar_signature_t signatures;   /* linked list of signatures */
  uint64_t last_fileid;         /* unique fileid's in the archive */
  xmlHashTablePtr ino_hash;     /* Hash for looking up hardlinked files (add) */
  xmlHashTablePtr link_hash;    /* Hash for looking up hardlinked files (extract) */
  xmlHashTablePtr csum_hash;    /* Hash for looking up checksums of files */
  EVP_MD_CTX *toc_ctx;
  int docksum;
  int skipwarn;
  int stripcomps;
  int tostdout;
  int rfcformat;
  struct stat sbcache;
};

/**
 * Converts an object to <code>struct __xar_t *</code>.
 */
#define XAR(x) ((struct __xar_t *)(x))

#endif /* _XAR_ARCHIVE_H_ */
