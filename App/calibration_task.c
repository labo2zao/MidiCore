#include "App/calibration_task.h"
#include "cmsis_os2.h"
#include "Services/pressure/pressure_i2c.h"
#include "Services/log/log.h"
#include "Services/expression/expression.h"
#include <string.h>

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
static int write_pressure_cfg(const pressure_cfg_t* pcfg){
#if !CAL_HAS_FATFS
  (void)pcfg; return -10;
#else
  FIL f;
  if(f_open(&f, "0:/cfg/pressure.tmp", FA_CREATE_ALWAYS|FA_WRITE)!=FR_OK) return -1;
  char buf[256];
  UINT bw=0;
  int n = snprintf(buf,sizeof(buf),
    "ENABLE=%u\r\nI2C_BUS=%u\r\nADDR=0x%02X\r\nTYPE=%u\r\nMAP_MODE=%u\r\nINTERVAL_MS=%u\r\n"
    "PMIN_PA=%ld\r\nPMAX_PA=%ld\r\nATM0_PA=%ld\r\n",
    (unsigned)pcfg->enable, (unsigned)pcfg->i2c_bus, (unsigned)pcfg->addr7,
    (unsigned)pcfg->type, (unsigned)pcfg->map_mode, (unsigned)pcfg->interval_ms,
    (long)pcfg->pmin_pa, (long)pcfg->pmax_pa, (long)pcfg->atm0_pa);
  if(n<0) { f_close(&f); return -2; }
  if(f_write(&f, buf, (UINT)n, &bw)!=FR_OK || bw!=(UINT)n){ f_close(&f); return -3; }
  f_close(&f);
  if(cc.keep_files){ /* keep */ } else { f_unlink("0:/cfg/pressure.bak"); }
  f_rename("0:/cfg/pressure.ngc","0:/cfg/pressure.bak");
  if(f_rename("0:/cfg/pressure.tmp","0:/cfg/pressure.ngc")!=FR_OK) return -4;
  return 0;
#endif
}

static int patch_expression_rawminmax(uint16_t raw_min, uint16_t raw_max){
#if !CAL_HAS_FATFS
  (void)raw_min;(void)raw_max; return -10;
#else
  // Try in-place update: read, replace RAW_MIN/RAW_MAX, rewrite atomically
  FIL f; 
  if(f_open(&f,"0:/cfg/expression.ngc",FA_READ)==FR_OK){
    char buf[4096]; UINT br=0; size_t off=0;
    for(;;){
      if(f_read(&f, buf+off, sizeof(buf)-off-1, &br)!=FR_OK) break;
      off += br;
      if(br==0) break;
      if(off > sizeof(buf)-64) break;
    }
    buf[off]=0;
    f_close(&f);

    // replace or append keys (case-insensitive)
    // simple regex-like replacement using manual lines processing
    std::string in(buf);
    auto set_key = [](const std::string& input, const std::string& key, const std::string& val){
      std::string out; out.reserve(input.size()+32);
      bool found=false;
      std::istringstream iss(input);
      std::string line;
      std::regex rx("^\\s*"+key+"\\s*=.*$", std::regex::icase);
      while(std::getline(iss, line)){
        if(std::regex_search(line, rx)){
          out += key + "=" + val + "\n";
          found=true;
        }else{
          out += line + "\n";
        }
      }
      if(!found){
        if(out.size() && out.back()!='\n') out.push_back('\n');
        out += key + "=" + val + "\n";
      }
      return out;
    };
    std::string out = set_key(in, "RAW_MIN", std::to_string(raw_min));
    out = set_key(out, "RAW_MAX", std::to_string(raw_max));

    if(f_open(&f,"0:/cfg/expression.tmp",FA_CREATE_ALWAYS|FA_WRITE)!=FR_OK) return -1;
    UINT bw=0;
    if(f_write(&f, out.c_str(), (UINT)out.size(), &bw)!=FR_OK || bw!=(UINT)out.size()){ f_close(&f); return -2; }
    f_close(&f);
    f_rename("0:/cfg/expression.ngc","0:/cfg/expression.bak");
    if(f_rename("0:/cfg/expression.tmp","0:/cfg/expression.ngc")!=FR_OK) return -3;
    return 0;
  } else {
    // fallback to previous behavior (minimal file)
    // original function body remains below this replacement
#endif

#if !CAL_HAS_FATFS
  (void)raw_min;(void)raw_max; return -10;
#else
  // naive: rewrite minimal expression.ngc with existing keys if file missing
  // For now: read existing file if present, but simplest is to rewrite key set.
  FIL f;
  if(f_open(&f,"0:/cfg/expression.tmp", FA_CREATE_ALWAYS|FA_WRITE)!=FR_OK) return -1;
  char buf[256];
  UINT bw=0;
  int n = snprintf(buf,sizeof(buf),
    "# auto-updated by calibration\r\nRAW_MIN=%u\r\nRAW_MAX=%u\r\n", (unsigned)raw_min, (unsigned)raw_max);
  if(n<0){ f_close(&f); return -2; }
  if(f_write(&f, buf, (UINT)n, &bw)!=FR_OK || bw!=(UINT)n){ f_close(&f); return -3; }
  f_close(&f);
  // append mode: keep old file as .bak and replace with .tmp (user can merge other keys)
  if(cc.keep_files){ /* keep */ } else { f_unlink("0:/cfg/expression.bak"); }
  f_rename("0:/cfg/expression.ngc","0:/cfg/expression.bak");
  if(f_rename("0:/cfg/expression.tmp","0:/cfg/expression.ngc")!=FR_OK) return -4;
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
    log_printf("CAL", "Pressure sensor not enabled or not XGZP");
    osThreadExit();
    return;
  }

  log_printf("CAL", "Start: keep bellows REST for %ums", (unsigned)cc.atm_ms);

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
    log_printf("CAL","No samples for ATM0");
    osThreadExit();
    return;
  }
  int32_t atm0 = (int32_t)llround((double)acc/(double)n);

  // Apply baseline immediately
  pressure_cfg_t pcfg = *cur;
  pcfg.atm0_pa = atm0;
  pressure_set_cfg(&pcfg);

  log_printf("CAL","ATM0=%ld Pa, now do full PUSH/PULL for %ums", (long)atm0, (unsigned)cc.ext_ms);

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
    log_printf("CAL","Invalid extremes pmin=%ld pmax=%ld", (long)pmin, (long)pmax);
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
  log_printf("CAL","Hot reload: expression RAW_MIN=%u RAW_MAX=%u", (unsigned)raw_min, (unsigned)raw_max);
}

// Persist

  int wrp = write_pressure_cfg(&pcfg);
  int wre = patch_expression_rawminmax(raw_min, raw_max);

  log_printf("CAL","Saved: PMIN=%ld PMAX=%ld ATM0=%ld RAW_MIN=%u RAW_MAX=%u (wrp=%d wre=%d)",
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
