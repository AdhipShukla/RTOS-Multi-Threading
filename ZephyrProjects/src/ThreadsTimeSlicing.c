#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/zephyr.h>
#define STACKSIZE 1024
#define THREAD0_PRIORITY 7
#define THREAD1_PRIORITY 7
#define CAS(P, O, N) xthal_compare_and_set((P), (O), (N))
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(blinking_led), gpios);

void thread0(void)
{
    if (!device_is_ready(led.port)){//Checking if the GPIO controller is ready
	    return;
    }
    int ret=gpio_pin_configure_dt(&led,GPIO_OUTPUT_ACTIVE);
    if (ret != 0)
	    return;
    while (1) {
        ret = gpio_pin_set_dt(&led,1);
        if (ret != 0)
            return;
        printk("Hello, I am thread0\n");
        //k_msleep(5);
        //k_yield();
        k_busy_wait(1000000);//Making CPU hog
    }
}

void thread1(void)
{
    if (!device_is_ready(led.port)){//Checking if the GPIO controller is ready
	    return;
    }
    int ret=gpio_pin_configure_dt(&led,GPIO_OUTPUT_ACTIVE);
    if (ret != 0)
	    return;
    while (1) {
        ret = gpio_pin_set_dt(&led,0);
        if (ret != 0)
            return;
        printk("Hello, I am thread1\n");
        //k_msleep(5);
        k_busy_wait(1000000);//Making CPU hog
    }
}
K_THREAD_DEFINE(thread0_id, STACKSIZE, thread0, NULL, NULL, NULL, THREAD0_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread1_id, STACKSIZE, thread1, NULL, NULL, NULL, THREAD1_PRIORITY, 0, 0);
