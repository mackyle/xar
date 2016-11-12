/*
 * Simple Gettext wrapper written by Jakub Kaszycki for XAR.
 * This file is in the Public Domain.
 */

#ifndef XAR_GETTEXT_H
#define XAR_GETTEXT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef ENABLE_NLS
#include <locale.h>
#include <libintl.h>

#ifdef GETTEXT_DOMAIN
#define _(_Message) D_ (GETTEXT_DOMAIN, _Message)
#else
#define _(_Message) gettext (_Message)
#endif
#define D_(_Domain, _Message) dgettext (_Domain, _Message)
#else
#define _(_Message) _Message
#define D_(_Domain, _Message) _Message
#endif

#endif
