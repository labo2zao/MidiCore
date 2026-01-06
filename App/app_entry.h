#ifndef APP_ENTRY_H
#define APP_ENTRY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Single entry point for application init & task creation.
 *
 * Call this from a CubeMX USER CODE block (e.g. StartDefaultTask) so CubeMX regen
 * won't overwrite application logic.
 */
void app_entry_start(void);

#ifdef __cplusplus
}
#endif

#endif // APP_ENTRY_H
