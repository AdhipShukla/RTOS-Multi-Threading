#define CAS(P, O, N) xthal_compare_and_set((P), (O), (N))
#define nthreds 10
#define QuLen 35
#define JobPerThread 5
static const BaseType_t app_cpu = 1;
static const BaseType_t pro_cpu = 0;
static SemaphoreHandle_t SemBinTaskCreation, SemCount;
typedef struct QueueMetaData{
  int Head;
  int Tail;
}QuM;
static QuM QuMeta;
static int QuLoFr[QuLen]={0};

void EnqueueLockFree(int Value){
  int MyTail;
  while(1){
    MyTail = QuMeta.Tail;
    //Serial.println("Stage1");
    if(MyTail==QuMeta.Tail && (MyTail+1)%QuLen != QuMeta.Head){
      //Serial.println("Stage2");
      if(CAS(&QuMeta.Tail, MyTail, (MyTail+1)%QuLen)==MyTail){
        QuLoFr[MyTail]=Value;
        //Serial.println("Stage3");
        break;
      }
    }
    vTaskDelay(10/portTICK_PERIOD_MS);//Sleep to avoid deadlock
  }
}

int DequeueLockFree(){
  int MyHead;
  int Value;
  while(1){
    MyHead = QuMeta.Head;
    //Serial.println("DStage1");
    if(MyHead==QuMeta.Head && (MyHead+1)%QuLen != QuMeta.Tail){
      //Serial.println("DStage2");
      if(CAS(&QuMeta.Head, MyHead, (MyHead+1)%QuLen)==MyHead){
        Value = QuLoFr[MyHead];
        QuLoFr[MyHead]=-1*Value;
        //Serial.println("DStage3");
        return Value;
      }
    }
    vTaskDelay(10/portTICK_PERIOD_MS);//Sleep to avoid deadlock
  }
}

void Enque(void *Para){
  int MyId= *(int *)Para;
  xSemaphoreGive(SemBinTaskCreation);//Signal that you have saved the parameter in local variable
  xSemaphoreTake(SemCount, portMAX_DELAY);//Wait until all the tasks are not created
  for(int i=0; i<JobPerThread; i++){
    //Serial.println("Stage0");
    EnqueueLockFree(MyId*JobPerThread+i);
    vTaskDelay(random(5,25)/portTICK_PERIOD_MS);//Sleep to avoid deadlock
  }
  Serial.println("Enqueue Task Complete ID: "+String(MyId));
  xSemaphoreGive(SemCount);
  vTaskDelete(NULL);
}

void Deque(void *Para){
  int MyId= *(int *)Para;
  int Val;
  xSemaphoreGive(SemBinTaskCreation); //Signal that you have saved the parameter in local variable
  xSemaphoreTake(SemCount, portMAX_DELAY);//Wait until all the tasks are not created
  for(int i=0; i<JobPerThread; i++){
    //Serial.println("DStage0");
    Val=DequeueLockFree();
    Serial.println("Dequeued Element: "+String(Val));
    vTaskDelay(random(5,25)/portTICK_PERIOD_MS);//Sleep to avoid deadlock
  }
  Serial.println("Dequeue Task Complete ID: "+String(MyId));
  xSemaphoreGive(SemCount);
  vTaskDelete(NULL);
}

void setup() {
  randomSeed(0);
  Serial.begin(115220);
  Serial.println("_____LOCKFREE QUEUE USING CAS_____");
  SemBinTaskCreation = xSemaphoreCreateBinary();
  SemCount = xSemaphoreCreateCounting(nthreds,0);
  Serial.println("Creating the Queue");
  QuMeta.Head=QuLen-1;
  //Serial.println("Stage6");
  QuMeta.Tail=0;
  //Serial.println("Stage7");
  char TaskName[nthreds] = {'\0'};
  for(int i=0; i<nthreds; i++){
    //Serial.println("Stage5");
    if(i<7){
      sprintf(TaskName,"Enque%d", i);
      xTaskCreatePinnedToCore(Enque,
                              TaskName,
                              1024,
                              (void *)&i,
                              1,
                              NULL,
                              i%2);
    } else {
      sprintf(TaskName,"Deque%d", i);
      xTaskCreatePinnedToCore(Deque,
                              TaskName,
                              1024,
                              (void *)&i,
                              1,
                              NULL,
                              i%2);
    }
    xSemaphoreTake(SemBinTaskCreation, portMAX_DELAY); //Wait until the task creation is done
  }
  Serial.println("All tasks created!");
  for(int i=0; i<nthreds-1; i++){
    xSemaphoreGive(SemCount);//Increase the Waitflag so all the threads can start their execution. This is still not same as releasing all the tasks at once
  }
  for(int i=0; i<nthreds-1; i++){
    Serial.println("Print Semaphore Wait: " + String(i));
    xSemaphoreTake(SemCount, portMAX_DELAY); //Wait until the task creation is done
  }
  Serial.println("Time To Print");
  for(int i=0; i<QuLen-1; i++){
    Serial.println("Element Id " + String(i) + " : " + String(QuLoFr[i]));
  }
  vTaskDelete(NULL);
}

void loop() {
  
}
