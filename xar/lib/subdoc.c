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
 * 04-Apr-2005
 * DRI: Rob Braun <bbraun@synack.net>
 */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include <xar/xar.h>
#include <xar/subdoc.h>
#include <xar/archive.h>
#include <xar/filetree.h>

xar_subdoc_t
xar_subdoc_new (xar_archive_t x, const char *name)
{
  xar_subdoc_t ret;

  if (xar_subdoc_find (x, name))
    return NULL;

  ret = malloc (sizeof (xar_subdoc));
  if (!ret)
    return NULL;

  memset (ret, 0, sizeof (xar_subdoc));
  ret->name = strdup (name);
  ret->next = x->subdocs;
  x->subdocs = ret;
  ret->x = x;

  return ret;
}

int32_t
xar_subdoc_prop_set (xar_subdoc_t s, const char *key, const char *value)
{
  return xar_prop_set ((xar_base_t) s, key, value);
}

int32_t
xar_subdoc_prop_get (xar_subdoc_t s, const char *key, const char **value)
{
  return xar_prop_get ((xar_base_t) s, key, value);
}

int32_t
xar_subdoc_attr_set (xar_subdoc_t s, const char *prop, const char *key,
                     const char *value)
{
  return xar_attr_set ((xar_base_t) s, prop, key, value);
}

const char *
xar_subdoc_attr_get (xar_subdoc_t s, const char *prop, const char *key)
{
  return xar_attr_get ((xar_base_t) s, prop, key);
}

xar_subdoc_t
xar_subdoc_first (xar_archive_t x)
{
  return x->subdocs;
}

xar_subdoc_t
xar_subdoc_next (xar_subdoc_t s)
{
  return s->next;
}

const char *
xar_subdoc_name (xar_subdoc_t s)
{
  return s->name;
}

xar_subdoc_t
xar_subdoc_find (xar_archive_t x, const char *name)
{
  xar_subdoc_t i;

  for (i = x->subdocs; i; i = i->next)
    {
      if (strcmp (name, i->name) == 0)
        return i;
    }

  return NULL;
}

int32_t
xar_subdoc_copyout (xar_subdoc_t s, unsigned char **ret, unsigned int *size)
{
  xmlBufferPtr buf;
  xmlTextWriterPtr writer;

  buf = xmlBufferCreate ();
  if (!buf)
    return -1;

  writer = xmlNewTextWriterMemory (buf, 0);
  if (!writer)
    {
      xmlBufferFree (buf);
      return -1;
    }

  xmlTextWriterSetIndent (writer, 4);
  xar_subdoc_serialize (s, writer, 0);

  xmlTextWriterEndDocument (writer);
  xmlFreeTextWriter (writer);

  if (size != NULL)
    *size = buf->use;
  *ret = malloc (buf->size);
  if (*ret == NULL)
    {
      xmlBufferFree (buf);
      return -1;
    }

  assert (size != NULL);
  memcpy (*ret, buf->content, *size);
  xmlBufferFree (buf);
  return 0;
}

int32_t
xar_subdoc_copyin (xar_subdoc_t s, const unsigned char *buf, unsigned int len)
{
  xmlTextReaderPtr reader;

  reader = xmlReaderForMemory ((const char *) buf, len, NULL, NULL, 0);
  if (!reader)
    return -1;

  xar_subdoc_unserialize (s, reader);
  xmlFreeTextReader (reader);
  return 0;
}

/* xar_subdoc_serialize
 * s: a subdoc structure allocated and initialized by xar_subdoc_new()
 * writer: and xmlTextWriterPtr that has already been opend and initialized
 * and is pointing to the place where the subdocument will be serialized.
 * wrap: an integer describing whether the subdocument is to be wrapped
 * for placement in the xml header of an archive, or if we are trying to
 * reconstruct the original document.  1 for wrapping, 0 for original.
 */
void
xar_subdoc_serialize (xar_subdoc_t s, xmlTextWriterPtr writer, int wrap)
{
  if (!s)
    return;
  if (wrap)
    {
      xmlTextWriterStartElementNS (writer, BAD_CAST (s->prefix),
                                   BAD_CAST ("subdoc"),
                                   BAD_CAST (s->ns));
      xmlTextWriterWriteAttribute (writer, BAD_CAST ("subdoc_name"),
                                   BAD_CAST (s->name));
      if (s->value)
        xmlTextWriterWriteString (writer, BAD_CAST (s->value));
    }
  xar_prop_serialize (s->props, writer);
  xmlTextWriterEndElement (writer);
}

void
xar_subdoc_remove (xar_subdoc_t s)
{
  xar_prop_t p;
  xar_subdoc_t tmp = xar_subdoc_first (s->x);

  if (tmp == s)
    {
      s->x->subdocs = s->next;
    }
  else
    {
      while (tmp->next)
        {
          if (tmp->next == s)
            {
              tmp->next = s->next;
              break;
            }
          tmp = xar_subdoc_next (tmp);
        }
    }

  while (s->props)
    {
      p = s->props;
      s->props = p->next;
      xar_prop_free (p);
    }
  free ((char *) s->blank1);
  free ((char *) s->name);
  free ((void *) s);
  return;
}

void
xar_subdoc_unserialize (xar_subdoc_t s, xmlTextReaderPtr reader)
{
  int type;

  while (xmlTextReaderRead (reader) == 1)
    {
      type = xmlTextReaderNodeType (reader);
      if (type == XML_READER_TYPE_ELEMENT)
        {
          xar_prop_unserialize ((xar_file_t) s, NULL, reader);
        }
      if (type == XML_READER_TYPE_TEXT)
        {
          const char *value;
          value = (const char *) xmlTextReaderConstValue (reader);
          free ((char *) s->value);
          s->value = strdup (value);
        }
      if (type == XML_READER_TYPE_END_ELEMENT)
        {
          break;
        }
    }

  return;
}
