#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

static const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart1));//UART does not hava a API-specific structure so using the DEVICE_DE_GET 
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(blinking_led), gpios);
static uint8_t rx_buf[10] = {0};
static uint8_t tx_buf[] =  {"Let's Occupy Mars \n\r"};
/*const struct uart_config uart_cfg = {
		.baudrate = 115200,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.data_bits = UART_CFG_DATA_BITS_8,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE
	};*/
/*struct uart_event_rx{
    uint8_t *buf;
    size_t offset;
    size_t len;
};
struct uart_event{
    enum uart_event_type type;
    union  uart_event_data{
        struct uart_event_tx tx;
        struct uart_event_rx rx;
        struct uart_event_rx_buf rx_buf;
        struct uart_event_rx_stop rx_stop;
    }data;
};*/
/*static void uart_cb(const struct  device *dev, struct uart_event *evt, void *user_data){
    switch (evt->type){
    
    case UART_TX_DONE:
        break;
    
    case UART_TX_ABORTED:
    // do something
    break;
		
	case UART_RX_RDY:
		if((evt->data.rx.len) == 1){
            if(evt->data.rx.buf[evt->data.rx.offset] == '1')
                gpio_pin_toggle_dt(&led);
        }
		break;

	case UART_RX_BUF_REQUEST:
		// do something
		break;

	case UART_RX_BUF_RELEASED:
		// do something
		break;
		
	case UART_RX_DISABLED:
        uart_rx_enable(dev, rx_buf, sizeof(rx_buf), 100);
		// do something
		break;

	case UART_RX_STOPPED:
		// do something
		break;
		
	default:
		break;
    }
}*/

int main(){
    int ret;
    unsigned char *p_char;
    if (!device_is_ready(led.port)){//Checking if the GPIO controller is ready
	    return;
    }
    ret=gpio_pin_configure_dt(&led,GPIO_OUTPUT_ACTIVE);

    if (ret != 0)
	    return;

    if(!device_is_ready(uart)){
        printk("UART device is not ready");
        return;
    }
    /*ret = uart_configure(uart, uart_cfg);
    if(ret == -ENOSYS){
        return -ENOSYS;
    }*/
    /*ret = uart_callback_set(uart, uart_cb, NULL);
    if (ret) {
        return ret;
    }
    ret = uart_rx_enable(uart, rx_buf, sizeof(rx_buf), 100);
    if (ret) {
		return 1;
	}
    ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
	if (ret) {
		return ret;
	}*/
    while(1){
        for(int i=0; i<sizeof(tx_buf); i++){
            uart_poll_out(uart, tx_buf[i]);
        }
        k_msleep(5000);
        uart_poll_in(uart, rx_buf);
        if(rx_buf[0] == 'H'){
            //gpio_pin_toggle_dt(&led);
            ret = gpio_pin_set_dt(&led,1);
        } else if (rx_buf[0] == 'L') {
            ret = gpio_pin_set_dt(&led,0);
        }
    }
}