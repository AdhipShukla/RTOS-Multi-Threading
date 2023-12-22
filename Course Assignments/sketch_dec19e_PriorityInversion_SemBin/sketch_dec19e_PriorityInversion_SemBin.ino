#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu=0;
#else
static const BaseType_t app_cpu=1;
#endif
static SemaphoreHandle_t SemBin;
TickType_t MedTime = 5000;
TickType_t LowTime = 250;
TickType_t HighTime = 250;
static portMUX_TYPE SL;

void TaskH(void *Para){
  TickType_t TimeStamp;
  int i;
  while(1){
    i=0;
    Serial.println();
    Serial.println("High Priority Task Trying to Acquire");
    TimeStamp = xTaskGetTickCount()*portTICK_PERIOD_MS;
    xSemaphoreTake(SemBin, portMAX_DELAY); //Acuire Lock
    Serial.print("Time for High to acquire the lock: ");
    Serial.println(String((xTaskGetTickCount()*portTICK_PERIOD_MS)-TimeStamp)); 
    //Serial.println("Doing Some Work");
    TimeStamp = xTaskGetTickCount()*portTICK_PERIOD_MS;
    while((xTaskGetTickCount()*portTICK_PERIOD_MS)-TimeStamp<HighTime){
      if (i<10){
        Serial.println("Inside HighP CS");
        i++;
      }
    }//Critical Section
    Serial.println("High Priority Task Releasing the Lock...");
    xSemaphoreGive(SemBin);
    vTaskDelay(500/portTICK_PERIOD_MS); //Sleep
  }
}
void TaskM(void *Para){
  TickType_t TimeStamp;
  int i;
  while(1){
    i=0;
    Serial.println();
    Serial.println("Med Priority Task Starting");
    TimeStamp = xTaskGetTickCount()*portTICK_PERIOD_MS;
    while((xTaskGetTickCount()*portTICK_PERIOD_MS)-TimeStamp<MedTime){
      if(i<3){
      Serial.println("Inside MedP...");
      i++;
      }
    }
    Serial.println("Med Task Done");
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}
void TaskL(void *Para){
  TickType_t TimeStamp;
  TickType_t TimeStamp1;
  TickType_t TimeStamp2;
  TickType_t JumpTime=0;
  int i;
  while(1){
    i=0;
    Serial.println();
    Serial.println("Low Priority Task Trying to Acquire");
    TimeStamp = xTaskGetTickCount()*portTICK_PERIOD_MS;
    Serial.print("Time for Low to acquire the lock: ");
    xSemaphoreTake(SemBin, portMAX_DELAY); //Acuire Lock
    Serial.println(String((xTaskGetTickCount()*portTICK_PERIOD_MS)-TimeStamp)); 
    //Serial.println("Low P Doing Some Work");
    TimeStamp = xTaskGetTickCount()*portTICK_PERIOD_MS;
    while(TimeStamp1-TimeStamp-JumpTime<LowTime || i==0){//It is importatnt to note here that even if the this task is prempted by some other task the xTaskGetTickCount will be running and it will return the total ticks from the time this task was called
      if (i<10){
        Serial.println("Inside LowP CS");
        i++;
      }
      TimeStamp2=xTaskGetTickCount()*portTICK_PERIOD_MS;
      //Serial.println(TimeStamp2-TimeStamp1);
      if (i==1){
        TimeStamp1=TimeStamp2;
      }
      if((TimeStamp2-TimeStamp1)>15){//Detecting an anomaly if time stamp suddenly increses beyond expected
        JumpTime=TimeStamp2-TimeStamp1;
        //Serial.println("NewJump:" + String(JumpTime));
      }
      TimeStamp1=TimeStamp2;
      //Serial.println("TimeS1:" + String(TimeStamp1));
    }//Critical Section
    Serial.println("Low Priority Task Releasing the Lock");
    xSemaphoreGive(SemBin);
    Serial.println("LowP work after lock");
    vTaskDelay(500/portTICK_PERIOD_MS); //Sleep
  }
}
void setup() {
Serial.begin(115220);
vTaskDelay(1000/portTICK_PERIOD_MS);
Serial.println("_____FreeRTOS Priority Inversion_____");
SemBin= xSemaphoreCreateBinary();
xSemaphoreGive(SemBin); //Setting the semaphore binary
xTaskCreatePinnedToCore( TaskL,
                        "Low Priority Task",
                        1024,
                        NULL,
                        1,
                        NULL,
                        app_cpu);
vTaskDelay(1/portTICK_PERIOD_MS);
xTaskCreatePinnedToCore( TaskH,
                        "High Priority Task",
                        1024,
                        NULL,
                        3,
                        NULL,
                        app_cpu);
xTaskCreatePinnedToCore( TaskM,
                        "Med Priority Task",
                        1024,
                        NULL,
                        2,
                        NULL,
                        app_cpu);
vTaskDelete(NULL);
}
void loop() {
  
}