#if CONFIG_FREERTOS_UNICORE 
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif


static SemaphoreHandle_t mutex;
static int wait;

void incVal(void *Parameter){
  int Val = *(int *)Parameter;
  wait=0;
  //xSemaphoreGive(mutex);//Can't give a thread that is took in one thread to give in some other thread 
  Serial.println(Val);
  while(1){
    if(Val!=0){
      Val++;
      Serial.println("Incremented Value:");
      Serial.println(Val);
      vTaskDelay(1000/portTICK_PERIOD_MS);
    }
  }
}

void setup() {
  Serial.println("_____FreeRTOS Keep Paramter Safe Till Fethced_____");
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  int Input;
  Serial.println("Enter the number to increment: ");
  while(Serial.available()<=0);
  Input=Serial.parseInt();
  Serial.println("Input: "+ String(Input));
  
  mutex = xSemaphoreCreateMutex();
  wait = 1;
  //xSemaphoreTake(mutex, portMAX_DELAY);
  xTaskCreatePinnedToCore( incVal,
                            "incrementVlaue",
                            1024,
                            (void *)&Input,
                            1,
                            NULL,
                            app_cpu);
  
  //xSemaphoreTake(mutex,portMAX_DELAY);
  while(wait){//Instead of a wait like this binary semaphores can be used
    Serial.println("Setup Waiting");
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
  Serial.println("Setup Done");
}

void loop() {
  // put your main code here, to run repeatedly:
}
