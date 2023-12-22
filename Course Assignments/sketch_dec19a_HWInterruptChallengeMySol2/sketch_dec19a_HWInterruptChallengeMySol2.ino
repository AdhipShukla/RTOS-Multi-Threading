#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static hw_timer_t *timer = NULL;
static const uint16_t TimerDivider = 8;
static const uint64_t TimerMax = 1000000; // 0.1 secs auto reload time for interrupt
static const TickType_t task_delay = 2000/portTICK_PERIOD_MS;
static int ADC = A0; //Smapling Pin
static int Tail = 0;
static const int BufLen = 20;
static int Head = BufLen-1;
static int Buf[BufLen]={0};
static float Average;
static SemaphoreHandle_t SemaMutAvg;
static SemaphoreHandle_t SemaNewSetCount;
static int BufActLen = 10;//This describes the number of digits considered for running avereage

void IRAM_ATTR onTimer(){
  //BaseType_t task_woken = pdTRUE;
  if(uxSemaphoreGetCount(SemaNewSetCount)<BufLen/BufActLen){
    Buf[Tail]=analogRead(ADC);
    Tail=(Tail+1)%BufLen;
    if(Tail%(BufActLen/2)==0){
      Serial.println("NewSetAvailable");
      xSemaphoreGive(SemaNewSetCount);
    }
  }
}

void AverageCal(void *para){
  //int Sum=0;
  int Divisor = BufActLen;
  int CurrFive=0;
  int PrevFive=0;
  while(1){
    if(xSemaphoreTake(SemaNewSetCount,0)==pdTRUE){//This will only be true if hald the size of number of elemets for which we are calculating the average area newly available
      for (int i=0; i<BufActLen/2; i++){//Running average of last 10 enteries 
        Head=(Head+1)%BufLen;
        CurrFive= CurrFive+Buf[i];
      }
      while(xSemaphoreTake(SemaMutAvg,0)!=pdTRUE){
        vTaskDelay(10/portTICK_PERIOD_MS);
      }
        Average=(float)(PrevFive+CurrFive)/Divisor;
      xSemaphoreGive(SemaMutAvg);
      PrevFive = CurrFive;
      CurrFive=0;
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
                          app_cpu);
  xTaskCreatePinnedToCore(AveragePrint,
                          "Average_Print",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);
  
  timer = timerBegin(0, TimerDivider, true);
  timerAttachInterrupt(timer, &onTimer,true);
  timerAlarmWrite(timer, TimerMax, true);
  timerAlarmEnable(timer);
}

void loop() {
  vTaskDelay(1000/portTICK_PERIOD_MS);
}