#include "Services/patch/patch_router.h"
#include "Services/router/router.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static void trim(char* s) {
  // leading
  char* p=s;
  while (*p && isspace((unsigned char)*p)) p++;
  if (p!=s) memmove(s,p,strlen(p)+1);
  // trailing
  size_t n=strlen(s);
  while (n && isspace((unsigned char)s[n-1])) { s[n-1]=0; n--; }
}

static uint8_t ieq(const char* a, const char* b) {
  while (*a && *b) {
    char ca=(char)toupper((unsigned char)*a++);
    char cb=(char)toupper((unsigned char)*b++);
    if (ca!=cb) return 0;
  }
  return *a==0 && *b==0;
}

static uint8_t starts_with_ci(const char* s, const char* pfx) {
  while (*pfx) {
    if (!*s) return 0;
    char cs=(char)toupper((unsigned char)*s++);
    char cp=(char)toupper((unsigned char)*pfx++);
    if (cs!=cp) return 0;
  }
  return 1;
}

static uint8_t parse_u8(const char* s, uint8_t* out) {
  if (!s || !*s) return 0;
  char* end=0;
  long v=strtol(s,&end,10);
  if (end==s) return 0;
  if (v<0 || v>255) return 0;
  *out=(uint8_t)v;
  return 1;
}

static int parse_node_name(const char* token) {
  // Accept forms:
  //  DIN_IN1, DIN1, IN1 -> DIN IN1
  //  DIN_OUT2, OUT2 -> DIN OUT2
  //  USB_IN, USB_OUT
  char t[24]; strncpy(t, token, sizeof(t)-1); t[sizeof(t)-1]=0;
  trim(t);

  // Normalize
  for (char* p=t; *p; ++p) *p=(char)toupper((unsigned char)*p);

  if (ieq(t,"USB_IN")) return ROUTER_NODE_USB_IN;
  if (ieq(t,"USB_OUT")) return ROUTER_NODE_USB_OUT;

  if (starts_with_ci(t,"DIN_IN")) {
    uint8_t n; if (parse_u8(t+6,&n) && n>=1 && n<=4) return (int)(ROUTER_NODE_DIN_IN1 + (n-1));
  }
  if (starts_with_ci(t,"DIN_OUT")) {
    uint8_t n; if (parse_u8(t+7,&n) && n>=1 && n<=4) return (int)(ROUTER_NODE_DIN_OUT1 + (n-1));
  }

  if (starts_with_ci(t,"DIN")) {
    // DIN1 => assume IN
    uint8_t n; if (parse_u8(t+3,&n) && n>=1 && n<=4) return (int)(ROUTER_NODE_DIN_IN1 + (n-1));
  }
  if (starts_with_ci(t,"IN")) {
    uint8_t n; if (parse_u8(t+2,&n) && n>=1 && n<=4) return (int)(ROUTER_NODE_DIN_IN1 + (n-1));
  }
  if (starts_with_ci(t,"OUT")) {
    uint8_t n; if (parse_u8(t+3,&n) && n>=1 && n<=4) return (int)(ROUTER_NODE_DIN_OUT1 + (n-1));
  }
  return -1;
}

static uint16_t parse_chmask_list(const char* s) {
  // Examples: "1..4,6,10..12"
  // returns bitmask (bit0=ch1)
  if (!s) return ROUTER_CHMASK_ALL;
  char buf[80]; strncpy(buf, s, sizeof(buf)-1); buf[sizeof(buf)-1]=0;
  trim(buf);
  if (!buf[0]) return ROUTER_CHMASK_ALL;

  uint16_t mask=0;
  char* p=buf;
  while (*p) {
    while (*p && (isspace((unsigned char)*p) || *p==',')) p++;
    if (!*p) break;

    char* token=p;
    while (*p && *p!=',') p++;
    char saved=*p; *p=0;

    // token could be "a..b" or "a"
    char* dots=strstr(token,"..");
    if (dots) {
      *dots=0;
      uint8_t a=0,b=0;
      trim(token); trim(dots+2);
      if (parse_u8(token,&a) && parse_u8(dots+2,&b)) {
        if (a<1) a=1; if (a>16) a=16;
        if (b<1) b=1; if (b>16) b=16;
        uint8_t lo=a<=b?a:b, hi=a<=b?b:a;
        for (uint8_t ch=lo; ch<=hi; ch++) mask |= (uint16_t)(1u<<(ch-1));
      }
    } else {
      uint8_t a=0;
      trim(token);
      if (parse_u8(token,&a) && a>=1 && a<=16) mask |= (uint16_t)(1u<<(a-1));
    }

    *p=saved;
  }
  return mask ? mask : ROUTER_CHMASK_ALL;
}

static void parse_route_value(const char* value, int* src, int* dst, uint16_t* chmask) {
  *src=-1; *dst=-1; *chmask=ROUTER_CHMASK_ALL;
  if (!value) return;

  char buf[120]; strncpy(buf, value, sizeof(buf)-1); buf[sizeof(buf)-1]=0;
  trim(buf);
  if (!buf[0]) return;

  // split optional " ch=..."
  char* chp = strstr(buf, " ch=");
  if (!chp) chp = strstr(buf, " CH=");
  if (chp) {
    *chp = 0;
    chp += 4; // after " ch="
    *chmask = parse_chmask_list(chp);
  }

  // split SRC->DST
  char* arrow = strstr(buf, "->");
  if (!arrow) return;
  *arrow = 0;
  char* ssrc = buf;
  char* sdst = arrow+2;
  trim(ssrc); trim(sdst);

  *src = parse_node_name(ssrc);
  *dst = parse_node_name(sdst);
}

void patch_router_apply(const patch_ctx_t* ctx) {
  // Iterate all entries in [router] section
  uint32_t n = patch_adv_count();
  for (uint32_t i=0;i<n;i++) {
    const patch_entry_t* e = patch_adv_at(i);
    if (!e) continue;
    if (strcmp(e->section, "router") != 0) continue;
    if (!starts_with_ci(e->key, "route")) continue;

    if (e->cond[0]) {
      if (!patch_adv_cond_eval(e->cond, ctx)) continue;
    }

    int src=-1,dst=-1;
    uint16_t chmask=ROUTER_CHMASK_ALL;
    parse_route_value(e->value, &src, &dst, &chmask);
    if (src < 0 || dst < 0) continue;

    router_set_route((uint8_t)src, (uint8_t)dst, 1);
    router_set_chanmask((uint8_t)src, (uint8_t)dst, chmask);
  }
}
