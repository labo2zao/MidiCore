#pragma once
#ifdef __cplusplus
extern "C" {
#endif

/** Run a blocking SRIO DIN self-test (prints raw DIN bytes periodically). */
void din_selftest_run(void);

#ifdef __cplusplus
}
#endif
