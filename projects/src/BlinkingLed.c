#include <zephyr/zephyr.h> //Header file that descrivbes the device tree headers
#include <zephyr/drivers/gpio.h> //Header file that describes GPIO
#include <zephyr/kernel.h>  

//static const struct device *gpio_ct_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0)); //Defining a pointer structure to GPIO controller device
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(blinking_led), gpios);
void main(void)
{
 /*if (!device_is_ready(gpio_ct_dev)){//Checking if the GPIO controller is ready
	return;
  }*/
  if (!device_is_ready(led.port)){//Checking if the GPIO controller is ready
	return;
  }
  int ret;
  //ret = gpio_pin_configure(gpio_ct_dev, 25, GPIO_OUTPUT_ACTIVE);
  //ret = gpio_pin_configure(led.port, led.pin, led.dt_flags\); //Thsi fucntion can be still be used with the node specific details of blinking_led
  ret=gpio_pin_configure_dt(&led,GPIO_OUTPUT_ACTIVE);
  if (ret != 0)
	return;

  while(true){
	//ret = gpio_pin_set_raw(gpio_ct_dev, 25, 1); //GPIO API defined in zephyr/drivers/gpio.h
	//ret = gpio_pin_set_raw(led.port, led.pin, 1);
	ret = gpio_pin_set_dt(&led,1);
	//ret =gpio_pin_toggle_dt(&led); //This API can also be used to toggle the LED
	if (ret != 0)
	   return;

	k_msleep(500);
	//ret = gpio_pin_set_raw(gpio_ct_dev, 25, 0);
	ret = gpio_pin_set_dt(&led,0);
	if (ret != 0)
           return;
	
	k_msleep(500);
  }
}
