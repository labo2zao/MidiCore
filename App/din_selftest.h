#pragma once

// DEPRECATED: This legacy DIN self-test is obsolete
// Use MODULE_TEST_SRIO instead for comprehensive SRIO testing
// This file is only compiled when MODULE_TEST_DIN_SELFTEST is enabled

#ifdef MODULE_TEST_DIN_SELFTEST

#ifdef __cplusplus
extern "C" {
#endif

/** Run a blocking SRIO DIN self-test (prints raw DIN bytes periodically). 
 * @deprecated Use MODULE_TEST_SRIO instead
 */
void din_selftest_run(void);

#ifdef __cplusplus
}
#endif

#endif // MODULE_TEST_DIN_SELFTEST
