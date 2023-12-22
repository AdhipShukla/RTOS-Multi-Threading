
void spinLock(int *Lock){
  while(xthal_compare_and_set(Lock, 0, 1)!=0){}
}
void spinUnLock(int *Lock){
  *Lock=0;
}

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu=0;
#else
  static const BaseType_t app_cpu=1;
#endif

//Global variable
static int sharedVar = 0;
static SemaphoreHandle_t mutex;
static int Lok;
void incrementTask(void *parameter){
  int localVar;
  while(1){
    //Serial.println("ESP32 Pointer Size: " + String(sizeof(&Lok)));
    spinLock(&Lok);
    //xTaskGetID();
    localVar = sharedVar;//Critica section start
    localVar++;
    vTaskDelay(random(100,500)/portTICK_PERIOD_MS);
    sharedVar = localVar;//Critica section ends
    spinUnLock(&Lok);
    Serial.println(sharedVar);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.println("_____Intro to Mutex_____");
  randomSeed(0);
  Serial.begin(115220);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  mutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(incrementTask,
                          "incrementCount1",
                          1024,
                          (void *)1,
                          1,
                          NULL,
                          app_cpu);
  xTaskCreatePinnedToCore(incrementTask,
                          "incrementCount2",
                          1024,
                          (void *)2,
                          1,
                          NULL,
                          app_cpu);
  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}