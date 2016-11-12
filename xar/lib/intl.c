#include <config.h>

#include <stdbool.h>
#include <gettext.h>
#include "localedir.h"

static bool init_done = false;

__attribute__ ((__constructor__))
void
xar__init_intl (void)
{
  if (init_done)
    return;
  init_done = true;

#if ENABLE_NLS
  bindtextdomain ("xar", LOCALEDIR);
#endif
}
