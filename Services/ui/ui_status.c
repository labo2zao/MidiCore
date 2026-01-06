#include "Services/ui/ui_status.h"
#include <string.h>

static char g_status[64];

void ui_set_status_line(const char* s)
{
  if(!s){ g_status[0]=0; return; }
  strncpy(g_status, s, sizeof(g_status)-1);
  g_status[sizeof(g_status)-1]=0;
}
