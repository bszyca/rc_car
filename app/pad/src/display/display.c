#include <string.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "display.h"

static uint8_t framebuf[OLED_BUF_SIZE];

static const struct display_buffer_descriptor desc = {
    .buf_size = OLED_BUF_SIZE,
    .width = OLED_W,
    .height = OLED_H,
    .pitch = OLED_W,
};

const struct display_buffer_descriptor *display_get_desc(void)
{
    return &desc;
}

static void set_pixel(int x, int y)
{
    if (x < 0 || x >= OLED_W || y < 0 || y >= OLED_H) {
        return;
    }

    framebuf[x + (y / 8) * OLED_W] |= (uint8_t)(1U << (y % 8));
}

static void clear_pixel(int x, int y)
{
    if (x < 0 || x >= OLED_W || y < 0 || y >= OLED_H) {
        return;
    }

    framebuf[x + (y / 8) * OLED_W] &= (uint8_t)~(1U << (y % 8));
}

static bool get_pixel(int x, int y)
{
    if (x < 0 || x >= OLED_W || y < 0 || y >= OLED_H) {
        return false;
    }

    return (framebuf[x + (y / 8) * OLED_W] & (uint8_t)(1U << (y % 8))) != 0U;
}

void draw_heart(int cx, int cy, int size)
{
    if (size <= 0) {
        return;
    }

    const int64_t s = 1024;
    const int64_t s2 = s * s;
    const int y_min = -(size / 2);
    const int y_max = (3 * size) / 2;

    for (int y = y_min; y <= y_max; y++) {
        for (int x = -size; x <= size; x++) {
            int64_t xn = ((int64_t)(2 * x) * s) / size;   /* x in [-2, 2] */
            int64_t yn = -((int64_t)(2 * y) * s) / size;  /* y in [-3, 1] */
            int64_t xabs = xn < 0 ? -xn : xn;

            int64_t u = xabs - s;
            int64_t top_term = s2 - (u * u);
            if (top_term < 0) {
                continue;
            }

            int64_t y_top = 0;
            {
                int64_t a = top_term;
                int64_t b = (a + 1) / 2;
                while (b < a) {
                    a = b;
                    b = (a + top_term / a) / 2;
                }
                y_top = a;
            }

            int64_t sqrt_half_abs_x = 0;
            {
                int64_t t = (xabs / 2) * s;
                int64_t a = t;
                int64_t b = (a + 1) / 2;
                while (b < a) {
                    a = b;
                    b = (a + t / a) / 2;
                }
                sqrt_half_abs_x = a;
            }

            int64_t inner = s - sqrt_half_abs_x;
            if (inner < 0) {
                continue;
            }

            int64_t y_bottom = 0;
            {
                int64_t t = inner * s;
                int64_t a = t;
                int64_t b = (a + 1) / 2;
                while (b < a) {
                    a = b;
                    b = (a + t / a) / 2;
                }
                y_bottom = -(27 * a) / 10;
            }

            if (yn <= y_top && yn > y_bottom) {
                set_pixel(cx + x, cy + y);
            }
        }
    }

    /* Trim two center tip pixels if present. */
    int trimmed = 0;
    for (int y = OLED_H - 1; y >= 0 && trimmed < 2; y--) {
        if (get_pixel(cx, y)) {
            clear_pixel(cx, y);
            trimmed++;
        }
    }
}

void display_init()
{
    const struct device *display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    int err;

    if (!device_is_ready(display)) {
        printk("Display device is not ready\n");
        return;
    }

    memset(framebuf, 0x00, sizeof(framebuf));
    draw_heart(OLED_W / 2, (OLED_H / 2) - 5, 16);
    err = display_blanking_off(display);
    if (err) {
        printk("display_blanking_off failed: %d\n", err);
    }

    const struct display_buffer_descriptor *desc = display_get_desc();
    err = display_write(display, 0, 0, desc, framebuf);
    if (err) {
        printk("display_write failed: %d\n", err);
    }
}
