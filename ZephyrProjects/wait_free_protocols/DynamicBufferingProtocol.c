#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <stdint.h>
#include <xtensa/hal.h>

static inline unsigned  CAS( int *addr, int testval, int setval )
{
    int result;

#if XCHAL_HAVE_S32C1I && XCHAL_HW_MIN_VERSION_MAJOR >= 2200
    __asm__ __volatile__ (
	"   wsr.scompare1 %2 \n"
	"   s32c1i %0, %3, 0 \n"
	    : "=a"(result) : "0" (setval), "a" (testval), "a" (addr)
	    : "memory");
#elif XCHAL_HAVE_INTERRUPTS
    int tmp = 0; // clang complains on unitialized var
    __asm__ __volatile__ (
        "   rsil   %4, 15 \n"		// %4 == saved ps
        "   l32i   %0, %3, 0 \n"	// %0 == value to test, return val
        "   bne    %2, %0, 9f \n"	// test
        "   s32i   %1, %3, 0 \n"	// write the new value
	"9: wsr.ps %4 ; rsync \n"	// restore the PS
	: "=a"(result)
	: "0" (setval), "a" (testval), "a" (addr), "a" (tmp)
	: "memory");
#else
    __asm__ __volatile__ (
        "   l32i  %0, %3, 0 \n"		// %0 == value to test, return val
        "   bne   %2, %0, 9f \n"	// test
        "   s32i  %1, %3, 0 \n"		// write the new value
	"9: \n"
	    : "=a"(result) : "0" (setval), "a" (testval), "a" (addr)
	    : "memory");
#endif
    return result;
}

//#define CAS(P,O,N)          xthal_compare_and_set((P),(O),(N))//esp compare and set atomic instruction
#define LOW_READER_STACK    1024
#define HIGH_READER_STACK   1024
#define Writer_STACK        1024
#define LOW_READER_PRIORITY    3
#define HIGH_READER_PRIORITY   1
#define Writer_PRIORITY        2
#define NumOfLoRead            3
#define SiBuf                  (2+NumOfLoRead)

typedef struct bufferProperties{
    int Val;
    uint32_t InUseCnt;
}bufferProp;
const bufferProp B0 = {.InUseCnt=0, .Val=99};
volatile bufferProp buffer[SiBuf]={[0]=B0};          //N+2 where N is the number of lower priority readers
volatile uint32_t previous = 0;             
volatile uint32_t current = 0;              

void low_reader(void *p1, void *p2, void *p3){
    uint8_t temp;
    uint32_t myCurr, myPrev;
    uint32_t loReaderPoi;    //Pointers for lower priority readers 
    while(1){
        printk("\n");
        //Call interrupt at realse to read the previous pointer
        if(*(int*)p3){
            for(;;){
                myPrev = previous;
                if(CAS(&previous,myPrev,myPrev)==myPrev){ //Check if the previous value read is still the previous
                    break;
                }
            }
            loReaderPoi=myPrev;
        } else {
            for(;;){
                myCurr = current;
                if(CAS(&current,myCurr,myCurr)==myCurr){ //Check if the current value read is still the current
                    break;
                }
            }
            loReaderPoi=myCurr;
        }
        temp=buffer[loReaderPoi].Val;
        printk("Value read by low reader%d %d\n",*(int*)p1,temp);
        while(CAS(&buffer[loReaderPoi].InUseCnt,buffer[loReaderPoi].InUseCnt,buffer[loReaderPoi].InUseCnt++)!=buffer[loReaderPoi].InUseCnt); //Incresing the the use count atomically 
        k_msleep(*(int*)p2);
        //for( volatile int i=0; i<*(int*)p2; i++);
        //printk("Value read by low reader%d %d\n",*(int*)p1,temp);
        while(CAS(&buffer[loReaderPoi].InUseCnt,buffer[loReaderPoi].InUseCnt,buffer[loReaderPoi].InUseCnt--)!=buffer[loReaderPoi].InUseCnt); //Value not in use by this thread anymore
        
    }
}

void high_reader(void *p1, void *p2, void *p3){
    uint32_t myPrev;
    uint8_t temp;
    uint32_t hiReaderPoi;      
    while(1){
        printk("\n");
        //Call interrupt at realse to read the previous pointer
        for(;;){
            myPrev = previous;
            if(CAS(&previous,myPrev,myPrev)==myPrev){ //Check if the previous value read is still the previous
                break;
            }
        }
        hiReaderPoi=myPrev;
        temp=buffer[hiReaderPoi].Val;
        printk("Value read by high reader%d %d\n",*(int*)p1,temp);
        while(CAS(&buffer[hiReaderPoi].InUseCnt,buffer[hiReaderPoi].InUseCnt,buffer[hiReaderPoi].InUseCnt++)!=buffer[hiReaderPoi].InUseCnt); //Value not in use by this thread anymore
        for( volatile int i=0; i<*(int*)p2; i++);
        //printk("Value read by high reader%d %d\n",*(int*)p1,temp);
        while(CAS(&buffer[hiReaderPoi].InUseCnt,buffer[hiReaderPoi].InUseCnt,buffer[hiReaderPoi].InUseCnt--)!=buffer[hiReaderPoi].InUseCnt); //Value not in use by this thread anymore
        k_msleep(*(int*)p3);
    }
}

void writer(){
    uint32_t myPrev, myCurr;
    uint32_t free;
    while(1){
        //Call interrupt at task release
        printk("Writer Started\n");
        for(;;){
            myCurr = current;
            if(CAS(&current,myCurr,myCurr)==myCurr){ //Atomically reading the current value
                break;
            }
        }
        for(;;){
            myPrev = previous;
            if(CAS(&previous,myPrev,myCurr)==myPrev){ //Change the previous value with the current value
                break;
            }
        }
        for(int i = 1; i<=SiBuf; i++){
            if((buffer[(myCurr+i)%SiBuf].InUseCnt)==0){
                free=(myCurr+i)%SiBuf;
                break;
            }
            if(i==SiBuf){
                printk("Fault: Empty buffer not found\n");
                return;
            }
        }
        for(;;){
            myCurr = current;
            if(CAS(&current,myCurr,free)==myCurr){ //Change the previous value with the current value
                break;
            }
        }
        buffer[current].Val = 100 + sys_rand32_get() % 50;
        printk("New value written %d\n",buffer[current].Val);
        for(int i=0; i<SiBuf+2;i++){
            if (i==0)
                printk("Previous: %d;   ",buffer[previous].Val);
            else if(i==1)
                printk("Current: %d;   ",buffer[current].Val);
            else
                printk("Element No. %d: %d;   ",i-2,buffer[i-2].Val);
        }
        printk("\n");
        for( volatile int i=0; i<5000000; i++);
        k_msleep(2000);
    }
}
int LoRe[3] = {1,2,3};
int LoRePe[3] = {1200,20000,10000};
int LoReUD[3] = {0,1,0}; //Wheter to LP tasks needs Unit dalay or no
int HiRe[3] = {1,2,3};
int HiReExe[3] = {1000000,5000000,8000000};//Execution time for HP tasks
int HiRePe[3] = {500,2000,8000};
K_THREAD_DEFINE(LoRNUD1,LOW_READER_STACK, low_reader, &LoRe[0], &LoRePe[0], &LoReUD[0], LOW_READER_PRIORITY, 0, 0);
K_THREAD_DEFINE(HiR1,HIGH_READER_STACK, high_reader, &HiRe[0], &HiReExe[0], &HiRePe[0], HIGH_READER_PRIORITY, 0, 0);
K_THREAD_DEFINE(LoRNUD2,LOW_READER_STACK, low_reader, &LoRe[1], &LoRePe[1], &LoReUD[1], LOW_READER_PRIORITY, 0, 0);
K_THREAD_DEFINE(HiR2,HIGH_READER_STACK, high_reader, &HiRe[1], &HiReExe[1], &HiRePe[1], HIGH_READER_PRIORITY, 0, 0);
K_THREAD_DEFINE(LoRNUD3,LOW_READER_STACK, low_reader, &LoRe[2], &LoRePe[2], &LoReUD[2], LOW_READER_PRIORITY, 0, 0);
K_THREAD_DEFINE(HiR3,HIGH_READER_STACK, high_reader, &HiRe[2], &HiReExe[2], &HiRePe[2], HIGH_READER_PRIORITY, 0, 0);
K_THREAD_DEFINE(Wri, Writer_STACK, writer, NULL, NULL, NULL, Writer_PRIORITY, 0, 0);
