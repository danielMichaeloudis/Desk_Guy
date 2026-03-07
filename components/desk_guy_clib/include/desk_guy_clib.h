#include <esp_err.h>
#include <stdbool.h>

void draw_bmp_file(const char *name);
void lcd_init(void);
esp_err_t sd_init(void);
bool check_exists(const char *name);
void list_fs(void);