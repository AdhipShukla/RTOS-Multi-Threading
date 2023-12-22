static const BaseType_t pro_cpu = 0;
static const BaseType_t app_cpu = 1;
static SemaphoreHandle_t SemBin;
static int LED = 14;
int LED_Freq_MS = 5000; 
void Wait(void *para){
  TickType_t TS1;
  TickType_t TS2;
  TS1= xTaskGetTickCount()*portTICK_PERIOD_MS;
  TS2=TS1;
  while(1){
    Serial.println("LEDOFF Core ID: " + String(xPortGetCoreID()));
    xSemaphoreGive(SemBin);
    while((TS2-TS1)<LED_Freq_MS){//For 500 ms let this processor run on this
      //Serial.println("Wait"); 
      TS2= xTaskGetTickCount()*portTICK_PERIOD_MS;
    }
    TS1=TS2;
    //vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}
void LED_ON(void *para){
  int * ptr1;
  int ptrval =5;
  ptr1 = &ptrval;
  while(1){
    xthal_compare_and_set(ptr1, ptrval, 10);
    Serial.println(*ptr1);
    Serial.println("LEDON Core ID: " + String(xPortGetCoreID())); 
    xSemaphoreTake(SemBin, portMAX_DELAY);
    digitalWrite(LED,!digitalRead(LED));
    //vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}
void setup(){
  Serial.begin(115200);
  pinMode(LED,OUTPUT);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println("FREERTOS_2Core_LED");
  SemBin = xSemaphoreCreateBinary();
  xSemaphoreGive(SemBin);
  xTaskCreatePinnedToCore(Wait,
                          "wait",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);
  xTaskCreatePinnedToCore(LED_ON,
                          "ledon",
                          1024,
                          NULL,
                          1,
                          NULL,
                          pro_cpu);
  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}
