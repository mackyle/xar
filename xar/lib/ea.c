/*
 * Copyright (c) 2008 Rob Braun
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
#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
#include <inttypes.h>

#ifndef HAVE_ASPRINTF
#include <xar/asprintf.h>
#endif
#include <xar/xar.h>
#include <xar/filetree.h>
#include <xar/archive.h>
#include <xar/b64.h>
#include <xar/ea.h>

xar_ea_t
xar_ea_new (xar_file_t f, const char *name)
{
  xar_ea_t ret;
  char *newidvalue;
  int err;

  err = asprintf (&newidvalue, "%" PRIu64, f->nexteaid);
  if (err == -1)
    return NULL;

  ret = calloc (sizeof (xar_ea), 1);
  if (!ret)
    {
      free (newidvalue);
      return NULL;
    }

  ret->prop = xar_prop_new ((xar_base_t) f, NULL);
  if (!ret->prop)
    {
      free (newidvalue);
      free (ret);
      return NULL;
    }

  f->nexteaid++;
  xar_prop_setkey (ret->prop, "ea");
  xar_prop_setvalue (ret->prop, NULL);
  ret->prop->attrs = xar_attr_new ();
  ret->prop->attrs->key = strdup ("id");
  ret->prop->attrs->value = newidvalue;

  xar_prop_pset ((xar_base_t) f, ret->prop, "name", name);

  return ret;
}

int32_t
xar_ea_pset (xar_file_t f, xar_ea_t e, const char *key, const char *value)
{
  if (xar_prop_pset ((xar_base_t) f, e->prop, key, value))
    return 0;
  return -1;
}

int32_t
xar_ea_pget (xar_ea_t e, const char *key, const char **value)
{
  xar_prop_t r = xar_prop_find (e->prop, key);
  if (!r)
    {
      if (value)
        *value = NULL;
      return -1;
    }

  if (value)
    *value = r->value;

  return 0;
}

xar_prop_t
xar_ea_root (xar_ea_t e)
{
  return e->prop;
}

xar_prop_t
xar_ea_find (xar_file_t f, const char *name)
{
  xar_prop_t p;

  for (p = xar_prop_pfirst (f); p; p = xar_prop_pnext (p))
    {
      const char *tmp;
      xar_prop_t tmpp;

      tmp = xar_prop_getkey (p);
      if (strncmp (tmp, XAR_EA_FORK, strlen (XAR_EA_FORK)) != 0)
        continue;
      if (strlen (tmp) != strlen (XAR_EA_FORK))
        continue;

      tmpp = xar_prop_pget (p, "name");
      if (!tmpp)
        continue;
      tmp = xar_prop_getvalue (tmpp);
      if (!tmp)
        continue;

      if (strcmp (tmp, name) == 0)
        return p;
    }

  return NULL;
}
