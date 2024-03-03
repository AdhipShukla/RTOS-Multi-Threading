#include<zephyr/kernel.h>
#include <zephyr/random/random.h>

#define THREAD0_PRIORITY 4
#define THREAD1_PRIORITY 4
#define STACK_SIZE 1024
#define COMBINED_TOTAL 40
K_MUTEX_DEFINE(test_mutex);

int32_t increment_count = 0;
int32_t decrement_count = COMBINED_TOTAL;

void shared_code_selection(){
    k_mutex_lock(&test_mutex, K_FOREVER);
    increment_count+=1;
    increment_count%=COMBINED_TOTAL;
    decrement_count-=1;
    if(decrement_count == 0){
        decrement_count = COMBINED_TOTAL;
    }
    k_mutex_unlock(&test_mutex);
    if(increment_count + decrement_count != COMBINED_TOTAL){
        printk("Race Condition Happened!\n");
        printk("Increment_count (%d) + Decrement_count (%d) = %d \n",increment_count, decrement_count, (increment_count + decrement_count));
        k_msleep(400 + sys_rand32_get() % 10);
    }
}

void thread0(){
    printk("Thread 0 started\n");
    while(1){
        shared_code_selection();
    }
}

void thread1(){
    printk("Thread 1 started\n");    
    while(1){        
        shared_code_selection();
    }
}

K_THREAD_DEFINE(thread0_id,STACK_SIZE,thread0, NULL, NULL, NULL, THREAD0_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread1_id,STACK_SIZE,thread1, NULL, NULL, NULL, THREAD1_PRIORITY, 0, 0);