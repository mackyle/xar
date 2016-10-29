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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <zlib.h>
#include <openssl/evp.h>

#include <xar/xar.h>
#include <xar/hash.h>
#ifndef HAVE_ASPRINTF
#include <xar/asprintf.h>
#endif

struct _hash_context
{
  EVP_MD_CTX *unarchived_cts;
  EVP_MD_CTX *archived_cts;
  uint8_t unarchived;
  uint8_t archived;
  uint64_t count;
};

#define CONTEXT(x) ((struct _hash_context *)(*x))

static char *xar_format_hash (const unsigned char *m, unsigned int len);

static struct _hash_context *
context_create (void)
{
  struct _hash_context *context;
  context =
    (struct _hash_context *) calloc (1, sizeof (struct _hash_context));
  if (context)
    {
      context->unarchived_cts = EVP_MD_CTX_create ();
      if (!context->unarchived_cts)
        {
          free (context);
          return NULL;
        }
      context->archived_cts = EVP_MD_CTX_create ();
      if (!context->archived_cts)
        {
          EVP_MD_CTX_destroy (context->unarchived_cts);
          free (context);
          return NULL;
        }
    }
  return context;
}

static void
context_destroy (struct _hash_context *context)
{
  if (context)
    {
      EVP_MD_CTX_destroy (context->unarchived_cts);
      EVP_MD_CTX_destroy (context->archived_cts);
      free (context);
    }
}

int32_t
xar_hash_unarchived (xar_t x, xar_file_t f, xar_prop_t p, void **in,
                     size_t * inlen, void **context)
{
  return xar_hash_unarchived_out (x, f, p, *in, *inlen, context);
}

int32_t
xar_hash_unarchived_out (xar_t x, xar_file_t f, xar_prop_t p, void *in,
                         size_t inlen, void **context)
{
  const char *opt;
  const EVP_MD *md;
  xar_prop_t tmpp;

  opt = NULL;
  tmpp = xar_prop_pget (p, "extracted-checksum");
  if (tmpp)
    opt = xar_attr_pget (f, tmpp, "style");

  if (!opt)
    opt = xar_opt_get (x, XAR_OPT_FILECKSUM);

  if (!opt || (0 == strcmp (opt, XAR_OPT_VAL_NONE)))
    return 0;

  if (!CONTEXT (context))
    {
      *context = context_create ();
      OpenSSL_add_all_digests ();
    }

  if (!CONTEXT (context)->unarchived)
    {
      md = EVP_get_digestbyname (opt);
      if (md == NULL)
        return -1;
      EVP_DigestInit_ex (CONTEXT (context)->unarchived_cts, md, NULL);
      CONTEXT (context)->unarchived = 1;
    }

  if (inlen == 0)
    return 0;

  CONTEXT (context)->count += inlen;
  EVP_DigestUpdate (CONTEXT (context)->unarchived_cts, in, (unsigned) inlen);
  return 0;
}

int32_t
xar_hash_archived (xar_t x, xar_file_t f, xar_prop_t p, void **in,
                   size_t * inlen, void **context)
{
  return xar_hash_archived_in (x, f, p, *in, *inlen, context);
}

int32_t
xar_hash_archived_in (xar_t x, xar_file_t f, xar_prop_t p, void *in,
                      size_t inlen, void **context)
{
  const char *opt;
  const EVP_MD *md;
  xar_prop_t tmpp;

  opt = NULL;
  tmpp = xar_prop_pget (p, "archived-checksum");
  if (tmpp)
    opt = xar_attr_pget (f, tmpp, "style");

  if (!opt)
    opt = xar_opt_get (x, XAR_OPT_FILECKSUM);

  if (!opt || (0 == strcmp (opt, XAR_OPT_VAL_NONE)))
    return 0;

  if (!CONTEXT (context))
    {
      *context = context_create ();
      OpenSSL_add_all_digests ();
    }

  if (!CONTEXT (context)->archived)
    {
      md = EVP_get_digestbyname (opt);
      if (md == NULL)
        return -1;
      EVP_DigestInit_ex (CONTEXT (context)->archived_cts, md, NULL);
      CONTEXT (context)->archived = 1;
    }

  if (inlen == 0)
    return 0;

  CONTEXT (context)->count += inlen;
  EVP_DigestUpdate (CONTEXT (context)->archived_cts, in, (unsigned) inlen);
  return 0;
}

int32_t
xar_hash_done (xar_t x, xar_file_t f, xar_prop_t p, void **context)
{
  unsigned char hashstr[HASH_MAX_MD_SIZE];
  char *str;
  unsigned int len;
  xar_prop_t tmpp;

  (void) x;
  if (!CONTEXT (context))
    return 0;

  if (CONTEXT (context)->count == 0)
    goto DONE;

  if (CONTEXT (context)->unarchived)
    {
      EVP_MD_CTX *ctx = CONTEXT (context)->unarchived_cts;
      const EVP_MD *md = EVP_MD_CTX_md (ctx);
      const char *type = OBJ_nid2ln (EVP_MD_nid (md));

      memset (hashstr, 0, sizeof (hashstr));
      EVP_DigestFinal_ex (CONTEXT (context)->unarchived_cts, hashstr, &len);
      str = xar_format_hash (hashstr, len);
      if (f)
        {
          tmpp = xar_prop_pset (f, p, "extracted-checksum", str);
          if (tmpp)
            xar_attr_pset (f, tmpp, "style", type);
        }
      free (str);
    }

  if (CONTEXT (context)->archived)
    {
      EVP_MD_CTX *ctx = CONTEXT (context)->archived_cts;
      const EVP_MD *md = EVP_MD_CTX_md (ctx);
      const char *type = OBJ_nid2ln (EVP_MD_nid (md));

      memset (hashstr, 0, sizeof (hashstr));
      EVP_DigestFinal_ex (CONTEXT (context)->archived_cts, hashstr, &len);
      str = xar_format_hash (hashstr, len);
      if (f)
        {
          tmpp = xar_prop_pset (f, p, "archived-checksum", str);
          if (tmpp)
            xar_attr_pset (f, tmpp, "style", type);
        }
      free (str);
    }

DONE:
  if (*context)
    {
      context_destroy (CONTEXT (context));
      *context = NULL;
    }

  return 0;
}

static char *
xar_format_hash (const unsigned char *m, unsigned int len)
{
  char *result = malloc ((2 * len) + 1);
  char hexValue[3];
  unsigned int itr = 0;

  result[0] = '\0';

  for (itr = 0; itr < len; itr++)
    {
      sprintf (hexValue, "%02x", m[itr]);
      strncat (result, hexValue, 2);
    }

  return result;
}

int32_t
xar_hash_out_done (xar_t x, xar_file_t f, xar_prop_t p, void **context)
{
  const char *uncomp = NULL, *uncompstyle = NULL;
  unsigned char hashstr[HASH_MAX_MD_SIZE];
  unsigned int len;
  const EVP_MD *md;
  int32_t err = 0;
  xar_prop_t tmpp;

  if (!CONTEXT (context))
    return 0;

  if (CONTEXT (context)->archived)
    {
      tmpp = xar_prop_pget (p, "archived-checksum");
      if (tmpp)
        {
          uncompstyle = xar_attr_pget (f, tmpp, "style");
          uncomp = xar_prop_getvalue (tmpp);
        }

      md = EVP_get_digestbyname (uncompstyle);

      if (uncomp && uncompstyle && md && CONTEXT (context)->archived)
        {
          char *str;
          memset (hashstr, 0, sizeof (hashstr));
          EVP_DigestFinal_ex (CONTEXT (context)->archived_cts, hashstr, &len);
          str = xar_format_hash (hashstr, len);
          if (strcmp (uncomp, str) != 0)
            {
              xar_err_new (x);
              xar_err_set_file (x, f);
              xar_err_set_string (x,
                                  "archived-checksum message digest hash values do not match");
              xar_err_callback (x, XAR_SEVERITY_FATAL,
                                XAR_ERR_ARCHIVE_EXTRACTION);
              err = -1;
            }
          free (str);
        }
    }

  if (CONTEXT (context)->unarchived)
    EVP_DigestFinal_ex (CONTEXT (context)->unarchived_cts, hashstr, &len);

  if (*context)
    {
      context_destroy (CONTEXT (context));
      *context = NULL;
    }

  return err;
}
