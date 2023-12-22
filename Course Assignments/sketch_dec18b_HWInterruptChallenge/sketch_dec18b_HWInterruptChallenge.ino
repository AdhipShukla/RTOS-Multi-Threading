/*#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif*/

# define configUSE_CORE_AFFINITY 1
static const BaseType_t app_cpu = 1;
static const BaseType_t pro_cpu = 0;

static hw_timer_t *timer = NULL;
static const uint16_t TimerDivider = 8;
static const uint64_t TimerMax = 1000000; // 0.1 secs auto reload time for interrupt
static const TickType_t task_delay = 2000/portTICK_PERIOD_MS;
static int ADC = A0; //Smapling Pin
static int Tail = 0;
static const int BufLen = 10;
static int Head = BufLen-1;
static int Buf[BufLen]={0};
static float Average;
static SemaphoreHandle_t SemaMutAvg;
static SemaphoreHandle_t SemaNewSetCount;
static int BufActLen = 10;//This describes the number of digits considered for running avereage

void IRAM_ATTR onTimer(){
  //Serial.println("Interrupts Core ID: " + String(xPortGetCoreID()));
  //BaseType_t task_woken = pdTRUE;
  if((Tail+1)%BufLen != Head){
    Buf[Tail]=analogRead(ADC);
    Tail=(Tail+1)%BufLen;
    if(Tail%(BufActLen/2)==0){
      xSemaphoreGive(SemaNewSetCount);
    }
  }
}

void AverageCal(void *para){
  Serial.println("Initializing Timer");
  timer = timerBegin(0, TimerDivider, true);
  timerAttachInterrupt(timer, &onTimer,true);
  timerAlarmWrite(timer, TimerMax, true);
  timerAlarmEnable(timer);
  int Sum=0;
  int Divisor = BufActLen;
  int CurrFive=0;
  int PrevFive=0;
  while(1){
    Serial.println("AverageCal Core ID: " + String(xPortGetCoreID())); 
    if((Head+1)%BufLen != Tail){
      for (int i=0; i<BufActLen; i++){//Running average of last 10 enteries 
        if((Head+1)%BufLen != Tail){
          Sum= Sum+Buf[Head];
          Head=(Head+1)%BufLen;
          //Serial.println("AC Head: " + String(Head));
        }else{
          Divisor=i;
          break;
        }
      }
      while(xSemaphoreTake(SemaMutAvg,0)!=pdTRUE){
        vTaskDelay(10/portTICK_PERIOD_MS);
      }
        Average=(float)Sum/Divisor;
      xSemaphoreGive(SemaMutAvg);
      Sum=0;
    }
    vTaskDelay(task_delay);
  }
}

void AveragePrint(void *para){
  char Avg[5]={'A','v','g',':','\0'};
  char CharIn[5] = {0};
  int i;
  char c;
  float AvgVal;
  while(1){
    Serial.println("AveragePrint Core ID: " + String(xPortGetCoreID())); 
    Serial.println("Head: " + String(Head));
    Serial.println("Tail: " + String(Tail));
    i=0;
    if(Serial.available()>0){
      while(Serial.available()>0){
        c=Serial.read();
        if(c=='\n'){
          break;
        }
        CharIn[i]=c;
        i++;
      }
      if(strcmp(CharIn, Avg)==0){
        while(xSemaphoreTake(SemaMutAvg,0)!=pdTRUE){
          vTaskDelay(10/portTICK_PERIOD_MS);
        }
        AvgVal=Average;
        xSemaphoreGive(SemaMutAvg);
        Serial.println("Running average of last 10 values: " + String(Average));
      }else{
        Serial.println(CharIn);
      }
      CharIn[4]='\0';
    }
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115220);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println("_____FREERTOS Hardware Interrupt Challenge_____"); 
  //UBaseType_t uxCoreAffinityMask=((0<<0)|(0<<1));
  //vTaskCoreAffinitySet = (NULL, uxCoreAffinityMask);
  SemaMutAvg=xSemaphoreCreateMutex();
  SemaNewSetCount = xSemaphoreCreateCounting(BufLen/BufActLen, 0);
  if(SemaMutAvg==NULL){
    Serial.println("Can't allocate semaphore");
     ESP.restart();
  }
  xTaskCreatePinnedToCore(AverageCal,
                          "Average_Calculation",
                          1024,
                          NULL,
                          2,
                          NULL,
                          pro_cpu);
  xTaskCreatePinnedToCore(AveragePrint,
                          "Average_Print",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);
  
  
}
void loop() {
  Serial.println("LOOP Core ID: " + String(xPortGetCoreID())); 
  vTaskDelay(1000/portTICK_PERIOD_MS);
}