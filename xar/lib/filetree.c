/*
 * Copyright (c) 2005-2008 Rob Braun
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlstring.h>

#ifndef HAVE_ASPRINTF
#include <xar/asprintf.h>
#endif
#include <xar/xar.h>
#include <xar/filetree.h>
#include <xar/archive.h>
#include <xar/b64.h>
#include <xar/ea.h>

/* Overview:
 * xar_file_t's exist within a xar_archive_t.  xar_prop_t's exist
 * within xar_file_t's and xar_attr_t's exist within xar_prop_t's
 * and xar_file_t's.
 * Basically, a xar_file_t is a container for xar_prop_t's.
 * xar_attr_t's are things like: <foo bar=5>blah</foo>
 * In this example, foo is the key of a xar_prop_t, and blah is
 * the value.  bar is the key of a xar_attr_t which is part of
 * foo's xar_prop_t, and 5 is bar's value.
 * xar_file_t's have xar_attr_t's for the case of:
 * <file id=42>
 * The file has an attribute of "id" with a value of "42".
 */

struct __xar_iter_t
{
  void *iter;
  char *path;
  void *node;
  int nochild;
};

/* Convenience macros for dereferencing the structs */
#define XAR_ITER(x) ((struct __xar_iter_t *)(x))

/* xar_attr_prop
 * Returns: a newly allocated and initialized property attribute.
 * It is the caller's responsibility to associate the attribute
 * with either a file or a property.
 */
xar_attr_t
xar_attr_new (void)
{
  xar_attr_t ret;

  ret = malloc (sizeof (struct xar_attr));
  if (!ret)
    return NULL;

  ret->key = NULL;
  ret->value = NULL;
  ret->next = NULL;
  ret->ns = NULL;
  return ret;
}

int32_t
xar_attr_pset (xar_base_t f, xar_prop_t p, const char *key, const char *value)
{
  xar_attr_t a, i;
  if (!p)
    {
      a = f->attrs;
    }
  else
    {
      a = p->attrs;
    }

  if (!a)
    {
      a = xar_attr_new ();
      if (!p)
        f->attrs = a;
      else
        p->attrs = a;
      a->key = strdup (key);
      a->value = strdup (value);
      return 0;
    }

  for (i = a; i && i->next; i = i->next)
    {
      if (strcmp (i->key, key) == 0)
        {
          free ((char *) i->value);
          i->value = strdup (value);
          return 0;
        }
    }
  a = xar_attr_new ();
  if (!p)
    {
      a->next = f->attrs;
      f->attrs = a;
    }
  else
    {
      a->next = p->attrs;
      p->attrs = a;
    }
  a->key = strdup (key);
  a->value = strdup (value);
  return 0;
}

/* xar_attr_set
 * f: the file the attribute is associated with
 * prop: The property key the attribute is associated with.  This can
 *       be NULL to signify the attribute should be set for the file,
 *       rather than the property.
 * key: The name of the attribute to set.
 * value: The value of the attribute.
 * Returns: 0 on success, -1 on failure.
 * Summary: Basically, sets an attribute.  The only tricky part is
 * it can set an attribute on a property or a file.
 */
int32_t
xar_attr_set (xar_base_t f, const char *prop, const char *key,
              const char *value)
{
  if (!prop)
    {
      return xar_attr_pset (f, NULL, key, value);
    }
  else
    {
      xar_prop_t p = NULL;
      p = xar_prop_find (f->props, prop);
      if (!p)
        return -1;
      return xar_attr_pset (f, p, key, value);
    }
}

const char *
xar_attr_pget (xar_base_t f, xar_prop_t p, const char *key)
{
  xar_attr_t a, i;

  if (!p)
    a = f->attrs;
  else
    a = p->attrs;

  if (!a)
    return NULL;

  for (i = a; i && i->next; i = i->next)
    {
      if (strcmp (i->key, key) == 0)
        {
          return i->value;
        }
    }
  if (i && (strcmp (i->key, key) == 0))
    return i->value;
  return NULL;
}

/* xar_attr_get
 * f: file to find the associated attribute in
 * prop: name of the property the attribute is of.  May be NULL to specify
 *       the file's attributes.
 * key: name of the attribute to search for.
 * Returns: a reference to the value of the attribute.
 */
const char *
xar_attr_get (xar_base_t f, const char *prop, const char *key)
{
  if (!prop)
    return xar_attr_pget (f, NULL, key);
  else
    {
      xar_prop_t p = NULL;
      p = xar_prop_find (f->props, prop);
      if (!p)
        return NULL;
      return xar_attr_pget (f, p, key);
    }
}

/* xar_attr_free
 * a: attribute to free
 * Summary: frees the attribute structure and everything inside it.
 * It is the caller's responsibility to ensure the linked list gets
 * updated.  This will *not* do anything to ensure the consistency
 * of the attribute linked list.
 */
void
xar_attr_free (xar_attr_t a)
{
  if (!a)
    return;
  free ((char *) a->key);
  free ((char *) a->value);
  free (a);
  return;
}

/* xar_attr_first
 * f: file to associate the iterator with
 * prop: the name of the property within the file to associate the iterator with
 * i: an iterator as returned by xar_iter_new
 * Returns: a pointer to the value of the first attribute associated with the
 * property 'prop' associated with the file 'f'
 * Summary: This MUST be called prior to calling xar_attr_next,
 * to iterate over the attributes of property key 'prop'.
 */
const char *
xar_attr_first (xar_base_t f, const char *prop, xar_iter_t i)
{
  xar_prop_t p = NULL;
  xar_attr_t a;

  if (!prop)
    a = f->attrs;
  else
    {
      p = xar_prop_find (f->props, prop);
      if (!p)
        return NULL;
      a = p->attrs;
    }

  if (!a)
    return NULL;

  XAR_ITER (i)->iter = a;
  free (XAR_ITER (i)->node);
  XAR_ITER (i)->node = strdup (a->key);
  return XAR_ITER (i)->node;
}

/* xar_attr_next
 * i: iterator allocated by xar_iter_new, and initialized with xar_attr_first
 * Returns: a pointer to the key of the next attribute associated with
 * the iterator.  NULL will be returned when there are no more attributes
 * to find.
 */
const char *
xar_attr_next (xar_iter_t i)
{
  xar_attr_t a = XAR_ITER (i)->iter;

  if (a->next == NULL)
    return NULL;

  XAR_ITER (i)->iter = a->next;
  free (XAR_ITER (i)->node);
  XAR_ITER (i)->node = strdup (((xar_attr_t) XAR_ITER (i)->iter)->key);
  return XAR_ITER (i)->node;
}

/* xar_iter_new
 * Returns a newly allocated iterator for use on files, properties, or
 * attributes.
 */
xar_iter_t
xar_iter_new (void)
{
  xar_iter_t ret = malloc (sizeof (struct __xar_iter_t));
  if (!ret)
    return NULL;

  XAR_ITER (ret)->iter = NULL;
  XAR_ITER (ret)->path = NULL;
  XAR_ITER (ret)->node = NULL;
  XAR_ITER (ret)->nochild = 0;
  return ret;
}

/* xar_iter_free
 * Frees memory associated with the specified iterator
 */
void
xar_iter_free (xar_iter_t i)
{
  free (XAR_ITER (i)->node);
  if (XAR_ITER (i)->path)
    free (XAR_ITER (i)->path);
  free (XAR_ITER (i));
}

const char *
xar_prop_getkey (xar_prop_t p)
{
  return p->key;
}

const char *
xar_prop_getvalue (xar_prop_t p)
{
  return p->value;
}

int32_t
xar_prop_setkey (xar_prop_t p, const char *key)
{
  free ((char *) p->key);
  if (key)
    p->key = strdup (key);
  return 0;
}

int32_t
xar_prop_setvalue (xar_prop_t p, const char *value)
{
  free ((char *) p->value);
  if (value)
    p->value = strdup (value);
  return 0;
}

/* xar_prop_pfirst
 * f: file to retrieve the first property from
 * Returns: a xar_prop_t corresponding to the first xar_prop_t associated with f
 * NULL if there are no properties associated with the file.
 */
xar_prop_t
xar_prop_pfirst (xar_file_t f)
{
  return f->props;
}

/* xar_prop_pnext
 * p: previous property used to retrieve the next
 * Returns: a xar_prop_t if there is a next, NULL otherwise
 */
xar_prop_t
xar_prop_pnext (xar_prop_t p)
{
  return p->next;
}

/* xar_prop_first
 * f: file to associate the iterator with
 * i: an iterator as returned by xar_iter_new
 * Returns: a pointer to the value of the first property associated with
 * the file 'f'.
 * Summary: This MUST be called first prior to calling xar_prop_next,
 * to iterate over properties of file 'f'.  This has the side effect of
 * associating the iterator with the file's properties, which is needed
 * before xar_prop_next.
 */
const char *
xar_prop_first (xar_file_t f, xar_iter_t i)
{
  XAR_ITER (i)->iter = f->props;
  free (XAR_ITER (i)->node);
  XAR_ITER (i)->node = strdup (((xar_prop_t) XAR_ITER (i)->iter)->key);
  return XAR_ITER (i)->node;
}

/* xar_prop_next
 * i: iterator allocated by xar_iter_new, and initialized with xar_prop_first
 * Returns: a pointer to the value of the next property associated with
 * the iterator.  NULL will be returned when there are no more properties
 * to find.  If a property has a NULL value, the string "" will be returned.
 * This will recurse down child properties, flattening the namespace and
 * adding separators.  For instance a1->b1->c1, a1 will first be returned,
 * the subsequent call will return "a1/b1", and the next call will return
 * "a1/b1/c1", etc.
 */
const char *
xar_prop_next (xar_iter_t i)
{
  xar_prop_t p = XAR_ITER (i)->iter;
  if (!(XAR_ITER (i)->nochild) && p->children)
    {
      int err;
      char *tmp = XAR_ITER (i)->path;
      if (tmp)
        {
          err =
            asprintf (&XAR_ITER (i)->path, "%s/%s", tmp, p->key);
          free (tmp);
        }
      else
        XAR_ITER (i)->path = strdup (p->key);
      if (!XAR_ITER (i)->path)
        return NULL;
      XAR_ITER (i)->iter = p = p->children;
      goto SUCCESS;
    }
  XAR_ITER (i)->nochild = 0;

  if (p->next)
    {
      XAR_ITER (i)->iter = p = p->next;
      goto SUCCESS;
    }

  if (p->parent)
    {
      char *tmp1, *tmp2;
      char *dname;

      if (strstr (XAR_ITER (i)->path, "/"))
        {
          tmp1 = tmp2 = XAR_ITER (i)->path;
          dname = dirname (tmp2);
          XAR_ITER (i)->path = strdup (dname);
          free (tmp1);
        }
      else
        {
          free (XAR_ITER (i)->path);
          XAR_ITER (i)->path = NULL;
        }

      XAR_ITER (i)->iter = p = p->parent;
      XAR_ITER (i)->nochild = 1;
      return xar_prop_next (i);
    }

  return NULL;
SUCCESS:
  free (XAR_ITER (i)->node);
  if (XAR_ITER (i)->path)
    {
      int err;
      char *result = NULL;
      err =
        asprintf (&result, "%s/%s", XAR_ITER (i)->path, p->key);
      XAR_ITER (i)->node = result;
    }
  else
    {
      if (p->key == NULL)
        XAR_ITER (i)->node = strdup ("");
      else
        XAR_ITER (i)->node = strdup (p->key);
    }
  return XAR_ITER (i)->node;
}

/* xar_prop_new
 * f: file to associate the new file with.  May not be NULL
 * parent: the parent property of the new property.  May be NULL
 * Returns: a newly allocated and initialized property.  
 * Summary: in addition to allocating the new property, it
 * will be inserted into the parent node's list of children,
 * and/or added to the file's list of properties, as appropriate.
 */
xar_prop_t
xar_prop_new (xar_base_t f, xar_prop_t parent)
{
  xar_prop_t p;

  p = malloc (sizeof (xar_prop));
  if (!p)
    return NULL;

  p->key = NULL;
  p->value = NULL;
  p->children = NULL;
  p->next = NULL;
  p->attrs = NULL;
  p->parent = parent;
  p->file = (xar_file_t) f;
  p->prefix = f->prefix;
  p->ns = NULL;
  if (parent)
    {
      if (!parent->children)
        {
          parent->children = p;
        }
      else
        {
          p->next = parent->children;
          parent->children = p;
        }
    }
  else
    {
      if (f->props == NULL)
        {
          f->props = p;
        }
      else
        {
          p->next = f->props;
          f->props = p;
        }
    }

  return p;
}

/* xar_prop_find
 * p: property to check
 * key: name of property to find.
 * Returns: reference to the property with the specified key
 * Summary: A node's name may be specified by a path, such as 
 * "a1/b1/c1", and child nodes will be searched for each 
 * "/" separator.  
 */
xar_prop_t
xar_prop_find (xar_prop_t p, const char *key)
{
  xar_prop_t i, ret;
  char *tmp1, *tmp2, *tmp3;

  if (!p)
    return NULL;
  tmp2 = tmp1 = strdup (key);
  tmp3 = strsep (&tmp2, "/");
  i = p;
  do
    {
      if (strcmp (tmp3, i->key) == 0)
        {
          if (tmp2 == NULL)
            {
              free (tmp1);
              return i;
            }
          ret = xar_prop_find (i->children, tmp2);
          free (tmp1);
          return ret;
        }
      i = i->next;
    }
  while (i);
  free (tmp1);
  return NULL;
}

/* xar_prop_set_r
 * p: property to recurse down and set the property of
 * key: key of the property to set
 * value: desired value of the property
 * Returns: 0 on sucess, -1 on failure.
 * Summary: This is an internal helper function for xar_prop_set() which
 * does the recursion down the property tree.
 */
static xar_prop_t
xar_prop_set_r (xar_base_t f, xar_prop_t p, const char *key,
                const char *value, int overwrite)
{
  xar_prop_t i, ret, ret2, start;
  char *tmp1, *tmp2, *tmp3;

  tmp2 = tmp1 = strdup (key);
  tmp3 = strsep (&tmp2, "/");

  if (!p)
    {
      start = f->props;
    }
  else
    {
      start = p->children;
    }

  for (i = start; i; i = i->next)
    {
      if (strcmp (tmp3, i->key) == 0)
        {
          if (!tmp2)
            {
              if (overwrite)
                {
                  xar_prop_setvalue (i, value);
                  free (tmp1);
                  return i;
                }
              else
                {
                  ret = xar_prop_new ((xar_base_t) f, p);
                  if (!ret)
                    {
                      free (tmp1);
                      return ret;
                    }
                  xar_prop_setvalue (ret, value);
                  xar_prop_setkey (ret, tmp3);
                  free (tmp1);
                  return ret;
                }
            }

          ret2 = xar_prop_set_r (f, i, tmp2, value, overwrite);
          free (tmp1);
          return ret2;
        }
    }

  ret = xar_prop_new ((xar_base_t) f, p);
  if (!ret)
    {
      free (tmp1);
      return ret;
    }

  if (!tmp2)
    {
      xar_prop_setvalue (ret, value);
      xar_prop_setkey (ret, tmp3);
      free (tmp1);
      return ret;
    }

  xar_prop_setkey (ret, tmp3);
  xar_prop_setvalue (ret, NULL);

  ret2 = xar_prop_set_r (f, ret, tmp2, value, overwrite);
  free (tmp1);
  return ret2;
}

/* xar_prop_set
 * f: file to set the property on
 * key: key of the property to set
 * value: desired value of the property
 * Returns: 0 on success, -1 on failure
 * Summary: If the property already exists, its value is overwritten
 * by 'value'.  If the property does not exist, it is created.
 * Copies of key and value are kept in the tree.  The caller may do
 * what they wish with these values after the call returns.
 * References to these copies will be returned by iterating over
 * the properties, or by calling xar_prop_get().
 * These copies will be released when the property is released.
 *
 * Note that you *CANNOT* have a node with a value and children.
 * This implementation will let you, but the serialization to xml
 * will not be what you're hoping for.
 */
int32_t
xar_prop_set (xar_base_t f, const char *key, const char *value)
{
  if (xar_prop_set_r ((xar_base_t) f, NULL, key, value, 1))
    return 0;
  return -1;
}

/* xar_prop_pset
 * Same as xar_prop_set, except it takes a xar_prop_t which will be
 * treated as the root property.
 * Returns a xar_prop_t that was created or set.  Returns NULL if error.
 */
xar_prop_t
xar_prop_pset (xar_base_t f, xar_prop_t p, const char *key, const char *value)
{
  return xar_prop_set_r ((xar_base_t) f, p, key, value, 1);
}

/* xar_prop_create
 * Identical to xar_prop_set, except it will not overwrite an existing
 * property, it will create another one.
 */
int32_t
xar_prop_create (xar_base_t f, const char *key, const char *value)
{
  if (xar_prop_set_r ((xar_base_t) f, NULL, key, value, 0))
    return 0;
  return -1;
}

/* xar_prop_get
 * f: file to look for the property in
 * key: name of property to find.
 * value: on return, *value will point to the value of the property
 * value may be NULL, in which case, only the existence of the property
 * is tested.
 * Returns: 0 for success, -1 on failure
 * Summary: A node's name may be specified by a path, such as 
 * "a1/b1/c1", and child nodes will be searched for each 
 * "/" separator.  
 */
int32_t
xar_prop_get (xar_base_t f, const char *key, const char **value)
{
  xar_prop_t r = xar_prop_find (f->props, key);
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
xar_prop_pget (xar_prop_t p, const char *key)
{
  char *tmp;
  const char *k;
  xar_prop_t ret;
  k = p->key;
  if (asprintf (&tmp, "%s/%s", k, key) == -1)
    return NULL;
  ret = xar_prop_find (p, tmp);
  free (tmp);
  return ret;
}

/* xar_prop_replicate_r
* f: file to attach property
* p: property (list) to iterate and add
* parent: parent property
* Summary: Recursivley adds property list (p) to file (f) and parent (parent).
*/

void
xar_prop_replicate_r (xar_base_t f, xar_prop_t p, xar_prop_t parent)
{
  xar_prop_t property = p;

  /* look through properties */
  for (property = p; property; property = property->next)
    {
      xar_prop_t newprop = xar_prop_new ((xar_base_t) f, parent);
      xar_attr_t a;
      xar_attr_t last;

      /* copy the key value for the property */
      newprop->key = strdup (property->key);
      if (property->value)
        newprop->value = strdup (property->value);

      /* loop through the attributes and copy them */
      a = NULL;
      last = NULL;

      /* copy attributes for file */
      for (a = property->attrs; a; a = a->next)
        {
          if (NULL == newprop->attrs)
            {
              last = xar_attr_new ();
              newprop->attrs = last;
            }
          else
            {
              last->next = xar_attr_new ();
              last = last->next;
            }

          last->key = strdup (a->key);
          if (a->value)
            last->value = strdup (a->value);
        }

      /* loop through the children properties and recursively add them */
      xar_prop_replicate_r ((xar_base_t) f, property->children, newprop);
    }

}

/* xar_prop_free
 * p: property to free
 * Summary: frees the specified property and all its children.
 * Stored copies of the key and value will be released, as will
 * all attributes.
 */
void
xar_prop_free (xar_prop_t p)
{
  xar_prop_t i;
  xar_attr_t a;
  while (p->children)
    {
      i = p->children;
      p->children = i->next;
      xar_prop_free (i);
    }
  while (p->attrs)
    {
      a = p->attrs;
      p->attrs = a->next;
      xar_attr_free (a);
    }
  free ((char *) p->key);
  free ((char *) p->value);
  free (p);
}

void
xar_prop_punset (xar_base_t f, xar_prop_t p)
{
  xar_prop_t i;
  if (!p)
    {
      return;
    }
  if (p->parent)
    {
      i = p->parent->children;
      if (i == p)
        {
          p->parent->children = p->next;
          xar_prop_free (p);
          return;
        }
    }
  else
    {
      i = f->props;
      if (i == p)
        {
          f->props = p->next;
          xar_prop_free (p);
          return;
        }
    }

  while (i && (i->next != p))
    {
      i = i->next;
    }
  if (i && (i->next == p))
    {
      i->next = p->next;
      xar_prop_free (p);
    }
  return;
}

void
xar_prop_unset (xar_base_t f, const char *key)
{
  xar_prop_t r = xar_prop_find (f->props, key);

  xar_prop_punset ((xar_base_t) f, r);
  return;
}

/* xar_file_new
 * f: parent file of the file to be created.  May be NULL
 * Returns: a newly allocated file structure.
 */
xar_file_t
xar_file_new (xar_file_t f)
{
  xar_file_t ret, i;

  ret = calloc (1, sizeof (xar_file));
  if (!ret)
    return NULL;

  ret->parent = f;
  ret->next = NULL;
  ret->children = NULL;
  ret->props = NULL;
  ret->attrs = NULL;
  ret->prefix = NULL;
  ret->ns = NULL;
  ret->fspath = NULL;
  ret->eas = NULL;
  ret->nexteaid = 0;
  if (f)
    {
      if (!f->children)
        {
          f->children = ret;
        }
      else
        {
          for (i = f->children; i->next;
               i = i->next);
          i->next = ret;
        }
    }

  return ret;
}

xar_file_t
xar_file_replicate (xar_file_t original, xar_file_t newparent)
{
  xar_file_t ret = xar_file_new (newparent);
  xar_attr_t a;

  /* copy attributes for file */
  for (a = original->attrs; a; a = a->next)
    {
      /* skip the id attribute */
      if (0 == strcmp (a->key, "id"))
        continue;

      xar_attr_set ((xar_base_t) ret, NULL, a->key, a->value);
    }

  /* recursively copy properties */
  xar_prop_replicate_r ((xar_base_t) ret, original->props, NULL);

  return ret;
}

/* xar_file_free
 * f: file to free
 * Summary: frees the specified file and all children,
 * properties, and attributes associated with the file.
 */
void
xar_file_free (xar_file_t f)
{
  xar_file_t i;
  xar_prop_t n;
  xar_attr_t a;
  while (f->children)
    {
      i = f->children;
      f->children = i->next;
      xar_file_free (i);
    }
  while (f->props)
    {
      n = f->props;
      f->props = n->next;
      xar_prop_free (n);
    }
  while (f->attrs)
    {
      a = f->attrs;
      f->attrs = a->next;
      xar_attr_free (a);
    }
  free ((char *) f->fspath);
  free (f);
}

/* xar_file_first
 * x: archive to associate the iterator with
 * i: an iterator as returned by xar_iter_new
 * Returns: a pointer to the name of the first file associated with
 * the archive 'x'.
 * Summary: This MUST be called first prior to calling xar_file_next,
 * to iterate over files of archive 'x'.  This has the side effect of
 * associating the iterator with the archive's files, which is needed
 * before xar_file_next.
 */
xar_file_t
xar_file_first (xar_archive_t x, xar_iter_t i)
{
  XAR_ITER (i)->iter = x->files;
  free (XAR_ITER (i)->node);
  return XAR_ITER (i)->iter;
}

/* xar_file_next
 * i: iterator allocated by xar_iter_new, and initialized with xar_file_first
 * Returns: a pointer to the name of the next file associated with
 * the iterator.  NULL will be returned when there are no more files
 * to find.  
 * This will recurse down child files (directories), flattening the 
 * namespace and adding separators.  For instance a1->b1->c1, a1 will 
 * first be returned, the subsequent call will return "a1/b1", and the 
 * next call will return "a1/b1/c1", etc.
 */
xar_file_t
xar_file_next (xar_iter_t i)
{
  xar_file_t f = XAR_ITER (i)->iter;
  const char *name;
  if (!(XAR_ITER (i)->nochild) && f->children)
    {
      char *tmp = XAR_ITER (i)->path;
      xar_prop_get ((xar_base_t) f, "name", &name);
      if (tmp)
        {
          int err;
          err = asprintf (&XAR_ITER (i)->path, "%s/%s", tmp, name);
          free (tmp);
        }
      else
        XAR_ITER (i)->path = strdup (name);
      if (!XAR_ITER (i)->path)
        return NULL;
      XAR_ITER (i)->iter = f = f->children;
      goto FSUCCESS;
    }
  XAR_ITER (i)->nochild = 0;

  if (f->next)
    {
      XAR_ITER (i)->iter = f = f->next;
      goto FSUCCESS;
    }

  if (f->parent)
    {
      char *tmp1, *tmp2;
      char *dname;

      if (strstr (XAR_ITER (i)->path, "/"))
        {
          tmp1 = tmp2 = XAR_ITER (i)->path;
          dname = dirname (tmp2);
          XAR_ITER (i)->path = strdup (dname);
          free (tmp1);
        }
      else
        {
          free (XAR_ITER (i)->path);
          XAR_ITER (i)->path = NULL;
        }

      XAR_ITER (i)->iter = f = f->parent;
      XAR_ITER (i)->nochild = 1;
      return xar_file_next (i);
    }

  return NULL;
FSUCCESS:
  xar_prop_get ((xar_base_t) f, "name", &name);
  XAR_ITER (i)->iter = (void *) f;

  return XAR_ITER (i)->iter;
}

/* xar_file_find
 * f: file subtree to look under
 * path: path to file to find
 * Returns the file_t describing the file, or NULL if not found.
 */
xar_file_t
xar_file_find (xar_file_t f, const char *path)
{
  xar_file_t i, ret;
  char *tmp1, *tmp2, *tmp3;

  if (!f)
    return NULL;
  tmp2 = tmp1 = strdup (path);
  tmp3 = strsep (&tmp2, "/");
  i = f;
  do
    {
      const char *name;
      xar_prop_get ((xar_base_t) i, "name", &name);
      if (name == NULL)
        continue;
      if (strcmp (tmp3, name) == 0)
        {
          if (tmp2 == NULL)
            {
              free (tmp1);
              return i;
            }
          ret = xar_file_find (i->children, tmp2);
          free (tmp1);
          return ret;
        }
      i = i->next;
    }
  while (i);
  free (tmp1);
  return NULL;
}


/* xar_prop_serialize
 * p: property to serialize
 * writer: the xmlTextWriterPtr allocated by xmlNewTextWriter*()
 * Summary: recursively serializes the property passed to it, including
 * children, siblings, attributes, etc.
 */
void
xar_prop_serialize (xar_prop_t p, xmlTextWriterPtr writer)
{
  xar_prop_t i;
  xar_attr_t a;

  if (!p)
    return;
  i = p;
  do
    {
      if (i->prefix || i->ns)
        xmlTextWriterStartElementNS (writer, BAD_CAST (i->prefix),
                                     BAD_CAST (i->key), NULL);
      else
        xmlTextWriterStartElement (writer, BAD_CAST (i->key));
      for (a = i->attrs; a; a = a->next)
        {
          xmlTextWriterWriteAttributeNS (writer, BAD_CAST (a->ns),
                                         BAD_CAST (a->key), NULL,
                                         BAD_CAST (a->value));
        }
      if (i->value)
        {
          if (strcmp (i->key, "name") == 0)
            {
              unsigned char *tmp;
              int outlen = (int) strlen (i->value);
              int inlen, len;

              inlen = len = outlen;

              tmp = malloc (len);
              assert (tmp);
              if (UTF8Toisolat1
                  (tmp, &len, BAD_CAST (i->value), &inlen) < 0)
                {
                  xmlTextWriterWriteAttribute (writer, BAD_CAST ("enctype"),
                                               BAD_CAST ("base64"));
                  xmlTextWriterWriteBase64 (writer, i->value, 0,
                                            (int)
                                            strlen (i->value));
                }
              else
                xmlTextWriterWriteString (writer,
                                          BAD_CAST (i->value));
              free (tmp);
            }
          else
            xmlTextWriterWriteString (writer, BAD_CAST (i->value));
        }

      if (i->children)
        {
          xar_prop_serialize (i->children, writer);
        }
      xmlTextWriterEndElement (writer);

      i = i->next;
    }
  while (i);
}

/* xar_file_serialize
 * f: file to serialize
 * writer: the xmlTextWriterPtr allocated by xmlNewTextWriter*()
 * Summary: recursively serializes the file passed to it, including
 * children, siblings, properties, attributes, etc.
 */
void
xar_file_serialize (xar_file_t f, xmlTextWriterPtr writer)
{
  xar_file_t i;
  xar_attr_t a;

  i = f;
  do
    {
      xmlTextWriterStartElement (writer, BAD_CAST ("file"));
      for (a = i->attrs; a; a = a->next)
        {
          xmlTextWriterWriteAttribute (writer, BAD_CAST (a->key),
                                       BAD_CAST (a->value));
        }
      xar_prop_serialize (i->props, writer);
      if (i->children)
        xar_file_serialize (i->children, writer);
      xmlTextWriterEndElement (writer);
      i = i->next;
    }
  while (i);
  return;
}

/* xar_prop_unserialize
 * f: file the property is to belong to
 * p: parent property, may be NULL
 * reader: xmlTextReaderPtr already allocated
 */
int32_t
xar_prop_unserialize (xar_file_t f, xar_prop_t parent,
                      xmlTextReaderPtr reader)
{
  const char *name, *value, *ns;
  int type, i, isempty = 0;
  int isname = 0, isencoded = 0;
  xar_prop_t p;

  p = xar_prop_new ((xar_base_t) f, parent);
  if (xmlTextReaderIsEmptyElement (reader))
    isempty = 1;
  i = xmlTextReaderAttributeCount (reader);
  name = (const char *) xmlTextReaderConstLocalName (reader);
  p->key = strdup (name);
  ns = (const char *) xmlTextReaderConstPrefix (reader);
  if (ns)
    p->prefix = strdup (ns);
  if (strcmp (name, "name") == 0)
    isname = 1;
  if (i > 0)
    {
      for (i = xmlTextReaderMoveToFirstAttribute (reader); i == 1;
           i = xmlTextReaderMoveToNextAttribute (reader))
        {
          xar_attr_t a;
          const char *name =
            (const char *) xmlTextReaderConstLocalName (reader);
          const char *value = (const char *) xmlTextReaderConstValue (reader);
          const char *ns = (const char *) xmlTextReaderConstPrefix (reader);
          if (isname && (strcmp (name, "enctype") == 0)
              && (strcmp (value, "base64") == 0))
            {
              isencoded = 1;
            }
          else
            {
              a = xar_attr_new ();
              a->key = strdup (name);
              a->value = strdup (value);
              if (ns)
                a->ns = strdup (ns);
              a->next = p->attrs;
              p->attrs = a;
            }
        }
    }
  if (isempty)
    return 0;
  while (xmlTextReaderRead (reader) == 1)
    {
      type = xmlTextReaderNodeType (reader);
      switch (type)
        {
        case XML_READER_TYPE_ELEMENT:
          xar_prop_unserialize (f, p, reader);
          break;
        case XML_READER_TYPE_TEXT:
          value = (const char *) xmlTextReaderConstValue (reader);
          free ((char *) p->value);
          if (isencoded)
            p->value =
              (const char *) xar_from_base64 (BAD_CAST (value),
                                              (unsigned) strlen (value),
                                              NULL);
          else
            p->value = strdup (value);
          if (isname)
            {
              if (f->parent)
                {
                  int err;
                  err =
                    asprintf ((char **) &f->fspath, "%s/%s",
                              f->parent->fspath,
                              p->value);
                }
              else
                {
                  f->fspath = strdup (p->value);
                }
              if (!f->fspath)
                return -1;
            }
          break;
        case XML_READER_TYPE_END_ELEMENT:
          return 0;
          break;
        }
    }

  /* XXX: Should never be reached */
  return 0;
}

/* xar_file_unserialize
 * x: archive we're unserializing to
 * parent: The parent file of the file to be unserialized.  May be NULL
 * reader: The xmlTextReaderPtr we are reading the xml from.
 * Summary: Takes a <file> node, and adds all attributes, child properties,
 * and child files.
 */
xar_file_t
xar_file_unserialize (xar_archive_t x, xar_file_t parent, xmlTextReaderPtr reader)
{
  xar_file_t ret;
  const char *name;
  int type, i;

  ret = xar_file_new (parent);

  i = xmlTextReaderAttributeCount (reader);
  if (i > 0)
    {
      for (i = xmlTextReaderMoveToFirstAttribute (reader); i == 1;
           i = xmlTextReaderMoveToNextAttribute (reader))
        {
          xar_attr_t a;
          const char *name =
            (const char *) xmlTextReaderConstLocalName (reader);
          const char *value = (const char *) xmlTextReaderConstValue (reader);
          a = xar_attr_new ();
          a->key = strdup (name);
          a->value = strdup (value);
          a->next = ret->attrs;
          ret->attrs = a;
        }
    }

  while (xmlTextReaderRead (reader) == 1)
    {
      type = xmlTextReaderNodeType (reader);
      name = (const char *) xmlTextReaderConstLocalName (reader);
      if ((type == XML_READER_TYPE_END_ELEMENT)
          && (strcmp (name, "file") == 0))
        {
          const char *opt;
          xar_prop_get ((xar_base_t) ret, "type", &opt);
          if (opt && (strcmp (opt, "hardlink") == 0))
            {
              opt = xar_attr_get ((xar_base_t) ret, "type", "link");
              if (opt && (strcmp (opt, "original") == 0))
                {
                  opt = xar_attr_get ((xar_base_t) ret, NULL, "id");
                  xmlHashAddEntry (x->link_hash, BAD_CAST (opt),
                                   ret);
                }
            }
          return ret;
        }

      if (type == XML_READER_TYPE_ELEMENT)
        {
          if (strcmp (name, "file") == 0)
            {
              xar_file_unserialize (x, ret, reader);
            }
          else
            xar_prop_unserialize (ret, NULL, reader);
        }
    }

  /* XXX Should never be reached */
  return ret;
}
