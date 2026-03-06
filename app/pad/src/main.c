#include "display/display.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void)
{
    display_init();

    while (1) {
        printk("RC pad alive\n");
        k_sleep(K_SECONDS(1));
    }

    return 0;
}
