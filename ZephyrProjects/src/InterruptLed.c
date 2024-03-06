#include <zephyr/zephyr.h> //Header file that descrivbes the device tree headers
#include <zephyr/drivers/gpio.h> //Header file that describes GPIO
#include <zephyr/kernel.h>  
#include <stdint.h>
static struct gpio_callback button_cb_data; //Interrupt callback struct
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(blinking_led), gpios);//Defined in overlay file for pin 25
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_NODELABEL(button), gpios);//Defined in overlay file for pin 13

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins){
    int ret;
    ret = gpio_pin_toggle_dt(&led);
    if (ret!=0){
        printk("could not toggle LED\n");
    }
}

void main(void)
{

  if (!device_is_ready(led.port)){//Checking if the GPIO controller is ready
	return;
  }
  if (!device_is_ready(button.port)){//Checking if the GPIO controller is ready
	return;
  }
  int ret;
  ret=gpio_pin_configure_dt(&led,GPIO_OUTPUT_ACTIVE);
  if (ret != 0)
	return;
  ret=gpio_pin_configure_dt(&button, GPIO_INPUT);
  if (ret != 0)
	return;
  ret= gpio_pin_interrupt_configure_dt(&button,GPIO_INT_EDGE_TO_ACTIVE);//Activating both the rising and falling edge for the interrupt at node &button
  if (ret != 0)
	return;
  gpio_init_callback(&button_cb_data,button_pressed, BIT(button.pin)); //First argument is the call back struct the next two arguments are the values for the fields, 2nd argument is callback and third is Pin mask
  gpio_add_callback(button.port,&button_cb_data);//Function enabling the interrupt
  //gpio_remove_callback(button.port,&button_cb_data);//Function disabling the interrupt
  ret= gpio_pin_interrupt_configure_dt(&button,GPIO_INT_EDGE_TO_ACTIVE);//Activating both the rising and falling edge for the interrupt at node &button
  if (ret != 0)
	return;
}
