#include<zephyr/device.h>
#include<zephyr/kernel.h>
#include<zephyr/drivers/pwm.h>

#define NUM_STEPS 100
#define SLEEP_DELTA_MSEC 20U //period after which timer handler will be called
static const struct pwm_dt_spec fading_led = PWM_DT_SPEC_GET(DT_NODELABEL(fading_led));// Fading_led is the label for child node defined in the overlay file. DT_NODELABEL() will return node Id. PWM_DT_SPEC_GET() return the sturct for PWM controller device
int ret;
static uint32_t pulse_width_nsec = 0U;
static uint32_t pulse_width_delta_nsec = 0U;
static bool increasing_intensity = true; //Start with increaing the intensity
static uint32_t steps_taken=0U;
void led_delta_timer_handler(struct k_timer *timer_info){
    if(increasing_intensity){
        if(steps_taken<NUM_STEPS){
            ret = pwm_set_pulse_dt(&fading_led, pulse_width_nsec);
            steps_taken++;
            pulse_width_nsec += pulse_width_delta_nsec;
        } else {
            increasing_intensity = false;
            steps_taken--;
            pulse_width_nsec -= pulse_width_delta_nsec;
        }
    } else {
        if(steps_taken>0){
            ret = pwm_set_pulse_dt(&fading_led, pulse_width_nsec);
            steps_taken--;
            pulse_width_nsec -= pulse_width_delta_nsec;
        } else {
            increasing_intensity = true;
            steps_taken++;
            pulse_width_nsec += pulse_width_delta_nsec;
        }
    }
}
K_TIMER_DEFINE(led_delta_timer, led_delta_timer_handler, NULL);//Initlaizing the timer using macro. The input is the name of the timer and a function to call when timer goes off. The function takes reference to the ktimer struct as an argument
void main(){
 if(!device_is_ready(fading_led.dev)){
    printk("Error: PWM device %s is not ready\n",fading_led.dev->name);
    return;
 }

/*while(1){
// Naive way to make fading led:
    for (int i=0; i<40; i++){
        if(i<21){
            ret = pwm_set_pulse_dt(&fading_led,i*10000/20); //Setting the output to 0 percent duty cycle
            k_sleep(K_SECONDS(0.2));
        } else {
            ret = pwm_set_pulse_dt(&fading_led,(40-i)*10000/20); //Setting the output to 0 percent duty cycle
            k_sleep(K_SECONDS(0.2));
        }
    }
}*/
    pulse_width_delta_nsec = fading_led.period/NUM_STEPS;
    k_timer_start(&led_delta_timer, K_MSEC(SLEEP_DELTA_MSEC),K_MSEC(SLEEP_DELTA_MSEC));//Starting the timer with name and periods
}