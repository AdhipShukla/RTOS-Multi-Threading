#define CAS(P, O, N) xthal_compare_and_set((P), (O), (N))
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu=0;
#else
static const BaseType_t app_cpu=1;
#endif

TickType_t MedTime = 5000;
TickType_t LowTick = 1;
TickType_t HighTick = 1;
int Val = 1;

void TaskH(void *Para){
  TickType_t TickStamp;
  int i;
  int NowVal;
  int MyVal;
  while(1){
    Serial.println("Inside TaskH");
    while(1){
      NowVal = Val;
      MyVal = Val-10;
      i=0;
      TickStamp = xTaskGetTickCount();
      while(xTaskGetTickCount()-TickStamp<HighTick){
        if(i<3){
        Serial.println("Doing Rocket Science with Val in TaskH");
        i++;
        }
      }
      Serial.println("High Priority Task Trying to CAS");
      if(CAS(&Val, NowVal, MyVal)==NowVal){
        Serial.println("High Priority Task Completing the CAS. Val: " + String(Val));
        break;
      }
      Serial.println("TaskH CAS Failed");
    }
    vTaskDelay(5000/portTICK_PERIOD_MS); //Sleep
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
  TickType_t TickStamp;
  int i;
  int NowVal;
  int MyVal;
  while(1){
    Serial.println("Inside TaskL");
    while(1){
      NowVal = Val;
      MyVal = Val+2;
      i=0;
      TickStamp = xTaskGetTickCount();
      while((xTaskGetTickCount())-TickStamp<LowTick){
        if(i<3){
        Serial.println("Doing Rocket Science with Val in TaskL");
        i++;
        }
      }
      Serial.println("Low Priority Task Trying to CAS");
      if(CAS(&Val, NowVal, MyVal)==NowVal){
        Serial.println("Low Priority Task Completing the CAS. Val: " + String(Val));
        break;
      }
      Serial.println("TaskL CAS Failed");
    }
    vTaskDelay(50/portTICK_PERIOD_MS); //Sleep
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println("_____FreeRTOS Priority Inversion_____");
  xTaskCreatePinnedToCore( TaskL,
                          "Low Priority Task",
                          1024,
                          NULL,
                          1,
                          NULL,
                          1);
  vTaskDelay(1/portTICK_PERIOD_MS);
  xTaskCreatePinnedToCore( TaskH,
                          "High Priority Task",
                          1024,
                          NULL,
                          3,
                          NULL,
                          1);
  xTaskCreatePinnedToCore( TaskM,
                          "Med Priority Task",
                          1024,
                          NULL,
                          2,
                          NULL,
                          1);
  vTaskDelete(NULL);
}

void loop() {
  
}