#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu=0;
#else
static const BaseType_t app_cpu=1;
#endif
//static int Cnt=0;
static SemaphoreHandle_t Sem[5];
static SemaphoreHandle_t ContSem;
static SemaphoreHandle_t SemBin;
void ThreadFunc(void *para){
  int MyId= *(int *)para;
  xSemaphoreGive(SemBin);
  while(1){
    Serial.println("Id" + String(MyId));
    xSemaphoreTake(ContSem,portMAX_DELAY);
    Serial.println("LCId" + String(MyId));
    xSemaphoreTake(Sem[MyId],portMAX_DELAY);
    Serial.println("L1Id" + String(MyId));
    vTaskDelay(10/portTICK_PERIOD_MS);
    xSemaphoreTake(Sem[(MyId+1)%5],0);
    Serial.println("L2Id" + String(MyId));
    xSemaphoreGive(ContSem);
    
    Serial.println("Both Lock Acquired" + String(MyId));
    vTaskDelay(100/portTICK_PERIOD_MS);
    
    xSemaphoreGive(Sem[(MyId+1)%5]);
    Serial.println("L2RId" + String(MyId));
    xSemaphoreGive(Sem[MyId]);
    Serial.println("L1RId" + String(MyId));
    vTaskDelay(10/portTICK_PERIOD_MS);
  }
}
void setup() {
  Serial.begin(115220);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  char TaskName[20];
  ContSem =xSemaphoreCreateMutex();//Introducing the contention lock(super lock)
  SemBin=xSemaphoreCreateBinary();
  for(int i=0; i<5; i++){
  Sem[i]=xSemaphoreCreateMutex();
  sprintf(TaskName, "Task%d", i);
  xTaskCreatePinnedToCore( ThreadFunc,
                          TaskName,
                          1024,
                          (void *)&i,
                          1,
                          NULL,
                          app_cpu);
  xSemaphoreTake(SemBin, portMAX_DELAY);
  }
  vTaskDelete(NULL);
}

void loop() {

}
