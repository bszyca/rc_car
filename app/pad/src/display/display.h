#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

#define OLED_W 128
#define OLED_H 64
#define OLED_BUF_SIZE (OLED_W * OLED_H / 8)

const struct display_buffer_descriptor *display_get_desc(void);
void draw_heart(int cx, int cy, int size);
void display_init(void);
