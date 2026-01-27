#include "App/calibration_task.h"
#include "cmsis_os2.h"
#include "Services/pressure/pressure_i2c.h"
#include "App/tests/test_debug.h"
#include "Services/expression/expression.h"
#include <string.h>
#include <stdlib.h>  // strtol
#include <stdio.h>   // snprintf
#include <math.h>    // llround

#if __has_include("ff.h")
  #include "ff.h"
  #define CAL_HAS_FATFS 1
#else
  #define CAL_HAS_FATFS 0
#endif

typedef struct {
  uint8_t keep_files;
  uint8_t enable;
  uint16_t atm_ms;
  uint16_t ext_ms;
  uint16_t margin_raw;
} cal_cfg_t;

static int keyeq_ci(const char* line, const char* key) {
  // match: ^\s*KEY\s*=  (case-insensitive)
  const char* l = line;
  while (*l==' ' || *l=='\t') l++;
  const char* k = key;
  while (*k && *l) {
    char a=*l, b=*k;
    if (a>='a'&&a<='z') a=(char)(a-32);
    if (b>='a'&&b<='z') b=(char)(b-32);
    if (a!=b) return 0;
    l++; k++;
  }
  if (*k) return 0;
  while (*l==' ' || *l=='\t') l++;
  return (*l=='=');
}


static void cal_defaults(cal_cfg_t* c){
  memset(c,0,sizeof(*c));
  c->enable=0;
  c->atm_ms=600;
  c->ext_ms=5000;
  c->margin_raw=60;
  c->keep_files=1;
}

static void trim(char* s){
  while(*s==' '||*s=='\t'||*s=='\r'||*s=='\n') memmove(s,s+1,strlen(s));
  size_t n=strlen(s);
  while(n && (s[n-1]==' '||s[n-1]=='\t'||s[n-1]=='\r'||s[n-1]=='\n')) s[--n]=0;
}

static int load_cal_cfg(cal_cfg_t* c){
#if !CAL_HAS_FATFS
  (void)c; return -10;
#else
  FIL f;
  if(f_open(&f, "0:/cfg/calibration.ngc", FA_READ)!=FR_OK) return -1;
  char line[160];
  while(f_gets(line,sizeof(line),&f)){
    trim(line);
    if(!line[0]||line[0]=='#'||line[0]==';'||line[0]=='[') continue;
    char* eq=strchr(line,'=');
    if(!eq) continue;
    *eq=0;
    char* k=line; char* v=eq+1;
    trim(k); trim(v);

    if(!strcmp(k,"ENABLE")) c->enable = (uint8_t)strtol(v,0,0);
    else if(!strcmp(k,"ATM_MS")) c->atm_ms = (uint16_t)strtol(v,0,0);
    else if(!strcmp(k,"EXT_MS")) c->ext_ms = (uint16_t)strtol(v,0,0);
    else if(!strcmp(k,"MARGIN_RAW")) c->margin_raw = (uint16_t)strtol(v,0,0);
    else if(!strcmp(k,"CAL_KEEP_FILES")) c->keep_files = (uint8_t)strtol(v,0,0);
  }
  f_close(&f);
  if(c->atm_ms < 200) c->atm_ms = 200;
  if(c->ext_ms < 1000) c->ext_ms = 1000;
  return 0;
#endif
}

// very small helper: rewrite whole file with updated keys (atomic by temp+rename)
static int write_pressure_cfg(const cal_cfg_t* cc, const pressure_cfg_t* c) {
#if !CAL_HAS_FATFS
  (void)cc; (void)c; return -10;
#else
  FIL f;
  if(f_open(&f, "0:/cfg/pressure.tmp", FA_CREATE_ALWAYS|FA_WRITE)!=FR_OK) return -1;
  char buf[128];
  UINT bw=0;
  int n = snprintf(buf,sizeof(buf),
    "ENABLE=%u\r\nI2C_BUS=%u\r\nADDR=0x%02X\r\nTYPE=%u\r\nMAP_MODE=%u\r\nINTERVAL_MS=%u\r\n"
    "PMIN_PA=%ld\r\nPMAX_PA=%ld\r\nATM0_PA=%ld\r\n",
    (unsigned)c->enable, (unsigned)c->i2c_bus, (unsigned)c->addr7,
    (unsigned)c->type, (unsigned)c->map_mode, (unsigned)c->interval_ms,
    (long)c->pmin_pa, (long)c->pmax_pa, (long)c->atm0_pa);
  if(n<0) { f_close(&f); return -2; }
  if(f_write(&f, buf, (UINT)n, &bw)!=FR_OK || bw!=(UINT)n){ f_close(&f); return -3; }
  f_close(&f);
  if(cc->keep_files){ /* keep */ } else { f_unlink("0:/cfg/pressure.bak"); }
  f_rename("0:/cfg/pressure.ngc","0:/cfg/pressure.bak");
  if(f_rename("0:/cfg/pressure.tmp","0:/cfg/pressure.ngc")!=FR_OK) return -4;
  return 0;
#endif
}

static int patch_expression_rawminmax(const cal_cfg_t* cc, uint16_t raw_min, uint16_t raw_max) {
#if !CAL_HAS_FATFS
  (void)cc; (void)raw_min; (void)raw_max;
  return -1;
#else
  const char* path = "0:/cfg/expression.ngc";
  const char* bak  = "0:/cfg/expression.bak";

  FIL f;
  if (f_open(&f, path, FA_READ) != FR_OK) return -1;
  UINT sz = f_size(&f);
  if (sz > 512) { f_close(&f); return -2; }

  static char inbuf[513];
  UINT br = 0;
  if (f_read(&f, inbuf, sz, &br) != FR_OK) { f_close(&f); return -3; }
  f_close(&f);
  inbuf[br] = 0;

  (void)f_unlink(bak);
  (void)f_rename(path, bak);

  static char outbuf[600];
  size_t outlen = 0;
  int found_min = 0, found_max = 0;

  const char* s = inbuf;
  while (*s) {
    const char* e = s;
    while (*e && *e != '\n' && *e != '\r') e++;
    size_t linelen = (size_t)(e - s);

    char line[128];
    size_t cpy = (linelen < sizeof(line)-1) ? linelen : (sizeof(line)-1);
    memcpy(line, s, cpy); line[cpy] = 0;

    if (keyeq_ci(line, "RAW_MIN")) {
      int n = snprintf(outbuf + outlen, sizeof(outbuf) - outlen, "RAW_MIN=%u\r\n", (unsigned)raw_min);
      if (n < 0) return -4;
      outlen += (size_t)n;
      found_min = 1;
    } else if (keyeq_ci(line, "RAW_MAX")) {
      int n = snprintf(outbuf + outlen, sizeof(outbuf) - outlen, "RAW_MAX=%u\r\n", (unsigned)raw_max);
      if (n < 0) return -4;
      outlen += (size_t)n;
      found_max = 1;
    } else {
      if (outlen + linelen + 2 >= sizeof(outbuf)) return -5;
      memcpy(outbuf + outlen, s, linelen); outlen += linelen;
      outbuf[outlen++] = '\r';
      outbuf[outlen++] = '\n';
    }

    while (*e == '\r' || *e == '\n') e++;
    s = e;
  }

  if (!found_min) {
    int n = snprintf(outbuf + outlen, sizeof(outbuf) - outlen, "RAW_MIN=%u\r\n", (unsigned)raw_min);
    if (n < 0) return -4;
    outlen += (size_t)n;
  }
  if (!found_max) {
    int n = snprintf(outbuf + outlen, sizeof(outbuf) - outlen, "RAW_MAX=%u\r\n", (unsigned)raw_max);
    if (n < 0) return -4;
    outlen += (size_t)n;
  }

  if (f_open(&f, path, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) return -6;
  UINT bw = 0;
  if (f_write(&f, outbuf, (UINT)outlen, &bw) != FR_OK || bw != (UINT)outlen) { f_close(&f); return -7; }
  f_close(&f);

  if (!cc->keep_files) { (void)f_unlink(bak); }
  return 0;
#endif
}


static void CalibrationTask(void* argument){
  (void)argument;

  cal_cfg_t cc; cal_defaults(&cc);
  if(load_cal_cfg(&cc)!=0 || !cc.enable){
    osThreadExit();
    return;
  }

  const pressure_cfg_t* cur = pressure_get_cfg();
  if(!cur->enable || cur->type != PRESS_TYPE_XGZP6847D_24B){
    dbg_printf("CAL: Pressure sensor not enabled or not XGZP\r\n");
    osThreadExit();
    return;
  }

  dbg_printf("CAL: Start: keep bellows REST for %ums\r\n", (unsigned)cc.atm_ms);

  // Step1: measure atmospheric baseline (absolute Pa)
  int64_t acc=0;
  uint32_t n=0;
  uint32_t t=0;
  while(t < cc.atm_ms){
    int32_t pa_abs=0;
    if(pressure_read_pa_abs(&pa_abs)==0){
      acc += (int64_t)pa_abs;
      n++;
    }
    osDelay(10);
    t += 10;
  }
  if(n==0){
    dbg_printf("CAL: No samples for ATM0\r\n");
    osThreadExit();
    return;
  }
  int32_t atm0 = (int32_t)llround((double)acc/(double)n);

  // Apply baseline immediately
  pressure_cfg_t pcfg = *cur;
  pcfg.atm0_pa = atm0;
  pressure_set_cfg(&pcfg);

  dbg_printf("CAL: ATM0=%ld Pa, now do full PUSH/PULL for %ums\r\n", (long)atm0, (unsigned)cc.ext_ms);

  // Step2: capture extremes of signed pressure
  int32_t pmin =  2147483647;
  int32_t pmax = -2147483647;
  t=0;
  while(t < cc.ext_ms){
    int32_t pa=0;
    if(pressure_read_pa(&pa)==0){
      if(pa < pmin) pmin = pa;
      if(pa > pmax) pmax = pa;
    }
    osDelay(5);
    t += 5;
  }

  if(pmin >= pmax){
    dbg_printf("CAL: Invalid extremes pmin=%ld pmax=%ld\r\n", (long)pmin, (long)pmax);
    osThreadExit();
    return;
  }

  // Update range
  pcfg.pmin_pa = pmin;
  pcfg.pmax_pa = pmax;

  // Map to raw and compute RAW_MIN/MAX with margin
  uint16_t rmin = pressure_to_12b(pmin);
  uint16_t rmax = pressure_to_12b(pmax);
  uint16_t margin = cc.margin_raw;
  uint16_t raw_min = (rmin > margin) ? (uint16_t)(rmin - margin) : 0;
  uint16_t raw_max = (uint16_t)((rmax + margin) < 4095 ? (rmax + margin) : 4095);

  // Hot-reload in RAM (no reboot needed)
{
  expr_cfg_t ec = *expression_get_cfg();
  ec.raw_min = raw_min;
  ec.raw_max = raw_max;
  expression_set_cfg(&ec);
  expression_runtime_reset();
  dbg_printf("CAL: Hot reload: expression RAW_MIN=%u RAW_MAX=%u\r\n", (unsigned)raw_min, (unsigned)raw_max);
}

// Persist

  int wrp = write_pressure_cfg(&cc, &pcfg);
  int wre = patch_expression_rawminmax(&cc, raw_min, raw_max);

  dbg_printf("CAL: Saved: PMIN=%ld PMAX=%ld ATM0=%ld RAW_MIN=%u RAW_MAX=%u (wrp=%d wre=%d)\r\n",
             (long)pmin,(long)pmax,(long)atm0,(unsigned)raw_min,(unsigned)raw_max, wrp, wre);

  // Disable calibration after done by rewriting calibration.ngc quickly
#if CAL_HAS_FATFS
  FIL f;
  if(f_open(&f,"0:/cfg/calibration.ngc",FA_CREATE_ALWAYS|FA_WRITE)==FR_OK){
    UINT bw;
    const char* s="# calibration done\r\nENABLE=0\r\n";
    f_write(&f,s,(UINT)strlen(s),&bw);
    f_close(&f);
  }
#endif

  osThreadExit();
}

void app_start_calibration_task(void){
  const osThreadAttr_t attr = {
    .name="Calib",
    .priority=osPriorityBelowNormal,
    .stack_size=1400
  };
  (void)osThreadNew(CalibrationTask, NULL, &attr);
}


