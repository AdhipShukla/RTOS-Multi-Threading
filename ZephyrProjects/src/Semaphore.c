#include<zephyr/kernel.h>
#include<zephyr/zephyr.h>
#include <zephyr/random/rand32.h>

#define PRODUCER_PRIORITY   5
#define CONSUMER_PRIORITY   4
#define STACK_SIZE  1024
volatile uint32_t available_resource = 10;

K_SEM_DEFINE(instance_monitor_sem, 10, 10);

void release_access(){
    available_resource++;
    printk("Resource given and available_instance_count = %d\n",available_resource);
    k_sem_give(&instance_monitor_sem);
}
void producer(){
    printk("Producer Thread Started");
    while(1){
        release_access();
        k_msleep(500 + sys_rand32_get()%10);
    }
}

void get_access(){
    int ret;
    ret = k_sem_take(&instance_monitor_sem, K_MSEC(50));
    if(ret==-EAGAIN){
        printk("Timeout at semaphore take.\n");
        return;
    } else if(ret==0){
        available_resource--;
        printk("Resource taken and available_instance_count = %d\n",available_resource);
    }
}
void consumer(){
    printk("Consumer Thread Started");
    while(1){
        get_access();
        k_msleep(sys_rand32_get()%10);
    }
}

K_THREAD_DEFINE(producer_id, STACK_SIZE, producer, NULL, NULL, NULL, PRODUCER_PRIORITY, 0, 0);
K_THREAD_DEFINE(consumer_id, STACK_SIZE, consumer, NULL, NULL, NULL, CONSUMER_PRIORITY, 0, 0);