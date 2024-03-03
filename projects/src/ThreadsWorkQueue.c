#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/zephyr.h>
#include <string.h>
#define CAS(P, O, N) xthal_compare_and_set((P), (O), (N))
#define STACKSIZE 1024
#define THREAD0_PRIORITY 2
#define THREAD1_PRIORITY 3
#define WORKQ_PRIORITY   4
#define MY_STACK_SIZE 512
#define MY_PROIRITY 5

K_THREAD_STACK_DEFINE(my_stack_area, MY_STACK_SIZE);// Define stack area used by workqueue thread
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(blinking_led), gpios);
static struct k_work_q offload_work_q = {0};; // Define workqueue structure

void CPU_Hogging(){
    if (!device_is_ready(led.port)){//Checking if the GPIO controller is ready
	    return;
    }
    int ret=gpio_pin_configure_dt(&led,GPIO_OUTPUT_ACTIVE);
    if (ret != 0)
	    return;
    //gpio_pin_toggle_dt(&led);
    ret = gpio_pin_set_dt(&led,1);
    for( volatile int i=0; i<350000; i++);
    ret = gpio_pin_set_dt(&led,0);
}

struct work_info{//Struct to define the work items
    struct k_work work;
    char name[25];
 } my_work;

void offload_function(struct k_work* work_item){//Work Item handler function input is the address to the work item itself
    
    CPU_Hogging();
}

void thread0(void)
{
    k_work_queue_start(&offload_work_q, my_stack_area, K_THREAD_STACK_SIZEOF(my_stack_area), WORKQ_PRIORITY, NULL); //Initializing the work queue and the work thread
    strcpy(my_work.name, "Thread 0 Hogging");//Naming the work item
    k_work_init(&my_work.work,offload_function);//Associating the work item with the handler and setting inital work as not pending
    uint64_t time_stamp;
    uint64_t delta_time;
    while(1){
        time_stamp = k_uptime_get();
        k_work_submit_to_queue(&offload_work_q, &my_work.work);//Adding work item to the workqueue  
        delta_time = k_uptime_delta(&time_stamp);
        printk("thread0 yielding the CPU in %lld ms \n", delta_time);
        k_msleep(20);
    }
}

void thread1(void)
{
    uint64_t time_stamp;
    uint64_t delta_time;
    while(1){
        time_stamp = k_uptime_get();
        CPU_Hogging();
        delta_time = k_uptime_delta(&time_stamp);
        printk("thread1 yielding the CPU in %lld ms \n", delta_time);
        k_msleep(20);
    }
}

K_THREAD_DEFINE(thread0_id, STACKSIZE, thread0, NULL, NULL, NULL, THREAD0_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread1_id, STACKSIZE, thread1, NULL, NULL, NULL, THREAD1_PRIORITY, 0, 0);