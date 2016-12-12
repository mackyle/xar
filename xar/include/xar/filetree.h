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

#ifndef _XAR_FILETREE_H_
#define _XAR_FILETREE_H_

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>

#include <xar/xar.h>
#include <xar/ea.h>

void xar_file_free (xar_file_t f);
xar_attr_t xar_attr_new (void);
int32_t xar_attr_pset (xar_base_t f, xar_prop_t p, const char *key,
                       const char *value);
const char *xar_attr_pget (xar_base_t f, xar_prop_t p, const char *key);
void xar_attr_free (xar_attr_t a);
void xar_file_serialize (xar_file_t f, xmlTextWriterPtr writer);
xar_file_t xar_file_unserialize (xar_t x, xar_file_t parent,
                                 xmlTextReaderPtr reader);
xar_file_t xar_file_find (xar_file_t f, const char *path);
xar_file_t xar_file_new (xar_file_t f);
xar_file_t xar_file_replicate (xar_file_t original, xar_file_t newparent);
void xar_file_free (xar_file_t f);

void xar_prop_serialize (xar_prop_t p, xmlTextWriterPtr writer);
int32_t xar_prop_unserialize (xar_file_t f, xar_prop_t parent,
                              xmlTextReaderPtr reader);
void xar_prop_free (xar_prop_t p);
xar_prop_t xar_prop_new (xar_base_t f, xar_prop_t parent);
xar_prop_t xar_prop_pset (xar_base_t f, xar_prop_t p, const char *key,
                          const char *value);
xar_prop_t xar_prop_find (xar_prop_t p, const char *key);
xar_prop_t xar_prop_pget (xar_prop_t p, const char *key);
const char *xar_prop_getkey (xar_prop_t p);
const char *xar_prop_getvalue (xar_prop_t p);
int32_t xar_prop_setkey (xar_prop_t p, const char *key);
int32_t xar_prop_setvalue (xar_prop_t p, const char *value);
xar_prop_t xar_prop_pfirst (xar_file_t f);
xar_prop_t xar_prop_pnext (xar_prop_t p);
void xar_prop_punset (xar_base_t f, xar_prop_t p);

#endif /* _XAR_FILETREE_H_ */
