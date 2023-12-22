#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu=0;
#else
  static const BaseType_t app_cpu=1;
#endif
static SemaphoreHandle_t MutexHead;
static SemaphoreHandle_t MutexTail;
static SemaphoreHandle_t SemBin;
static SemaphoreHandle_t SemEmpty;
static SemaphoreHandle_t SemFilled;
static const int prods = 5;
static const int consu = 2;
static const int BufLen = 10;
static int wait;
static int Buf[BufLen] = {-1};
int Head = 9;
int Tail =0;
int cnt=0;
void Producer(void *Para){
  int MyId;
  MyId=*(int *)Para;
  xSemaphoreGive(SemBin);
  /*Serial.println("BW Inside Producer: " + String(MyId));
  while(wait==1){
    Serial.println("Waiting ID_" + String(MyId));
  }
  Serial.println("AW Inside Producer: " + String(MyId));*/
  for(int i=0; i<3; i++){
    if(xSemaphoreTake(MutexTail,1)==pdTRUE){
      if((Tail+1)%BufLen!=Head){
        //while(xSemaphoreGive(SemEmpty)!=pdTRUE){
          Buf[Tail]=MyId;//*3+i;
          //Serial.println("Value Entered: " + String(Buf[Tail]));
          Tail = (Tail+1)%BufLen;
        //}
      }
      else{
        i--;
      }
      xSemaphoreGive(MutexTail);
      vTaskDelay(100/portTICK_PERIOD_MS);
    }
    else{
      i--;
    }
  }
  Serial.println("All values entered by ID" + String(MyId));
  vTaskDelete(NULL);
}
void Consumer(void *Para){
  int MyId;
  MyId=*(int *)Para;
  xSemaphoreGive(SemBin);
  /*Serial.println("BW Inside Consumer: " + String(MyId));
  while(wait==1){
    Serial.println("Waiting ID_" + String(MyId));
  }
  Serial.println("AW Inside Consumer: " + String(MyId));*/
  while(1){
    /*if(cnt==15){
      Serial.println("Work Done ID_" + String(MyId));
      vTaskDelete(NULL);
    }*/  
    while(xSemaphoreTake(MutexHead,1)==pdTRUE){
      if((Head+1)%BufLen!=Tail){
        //while(xSemaphoreTake(SemEmpty)!=pdTRUE){
          Head = (Head+1)%BufLen;
          Serial.println("Values Removed: " + String(Buf[Head]));
          cnt++;
        //}
      }
      xSemaphoreGive(MutexHead);
      vTaskDelay(100/portTICK_PERIOD_MS);
    }
  }
}
void setup() {
  Serial.begin(600);
  Serial.println("_____Mutex and Semaphores Circular Buffer_____");
  vTaskDelay(1000/portTICK_PERIOD_MS);
  MutexHead = xSemaphoreCreateMutex();
  MutexTail = xSemaphoreCreateMutex();
  SemBin = xSemaphoreCreateBinary();
  SemEmpty = xSemaphoreCreateCounting(10,0);
  SemFilled = xSemaphoreCreateCounting(10,0);
  char TaskName[15] = {0};
  wait=1;
  for (int i=0; i<prods+consu; i++){
    if(i<prods){
      sprintf(TaskName, "Producer%d", i);
      xTaskCreatePinnedToCore(
                              Producer,
                              TaskName,
                              1024,
                              (void *)&i,
                              1,
                              NULL,
                              app_cpu);
    }
    else{
      sprintf(TaskName, "Consumer%d", i);
      xTaskCreatePinnedToCore(
                              Consumer,
                              TaskName,
                              1024,
                              (void *)&i,
                              1,
                              NULL,
                              app_cpu);
    }
  xSemaphoreTake(SemBin,portMAX_DELAY);
  }
  wait=0;
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println("Wait: " + String(wait));
  Serial.println("Setup Done!"); 
}

void loop() {
  vTaskDelay(1000/portTICK_PERIOD_MS);
}