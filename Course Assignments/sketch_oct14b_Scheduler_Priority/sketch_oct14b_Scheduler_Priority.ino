//Using only one core
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

const char msg[] = "Checking the scheduling of all the tasks";

//Task Handle
static TaskHandle_t task_1 = NULL;
static TaskHandle_t task_2 = NULL;

void Task1Func(void *Parameter){
  // count numner of elements in the string
  UBaseType_t priorityT1= uxTaskPriorityGet(NULL);
  Serial.println("PriorityT1=" + String(priorityT1));
  int msg_len= strlen(msg);
  while(1){
    Serial.println();
    for (int i=0;i<msg_len;i++){
      Serial.print(msg[i]);
      vTaskDelay(10/portTICK_PERIOD_MS);
    }
    Serial.println();
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
  return;
}

void Task2Func(void *Parameter){
  UBaseType_t priorityT2= uxTaskPriorityGet(NULL);
  Serial.println("PriorityT2=" + String(priorityT2));
  int j= 0;
  while(1){
    Serial.print('*');
    vTaskDelay(100/portTICK_PERIOD_MS);
  } 
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("______________________FREERTOS TASK SCHEDULING DEMO________________________");
  Serial.println();
  Serial.print("Setup and Loop task runing on core");
  Serial.print(xPortGetCoreID());
  Serial.print(" with priority");
  Serial.println(uxTaskPriorityGet(NULL));
  Serial.println();

  xTaskCreatePinnedToCore( Task1Func,
    "Task1",
    1024,
    NULL,
    3,
    &task_1,
    app_cpu);
  
  xTaskCreatePinnedToCore( Task2Func,
    "Task2",
    2048,
    NULL,
    2,
    &task_2,// Task Handler that can be used to alter the task
    app_cpu);
}

void loop() { // Task 3
  // put your main code here, to run repeatedly:
for (int i=0; i<3 ; i++){
  Serial.println("Index = " + String(i));
  vTaskSuspend(task_2);// Suspending the task using RTOS fucntion this will put the task in suspended state and can only be resumed by explicitly continuing which will set its state to ready and scheduler can again run it
  vTaskDelay(2000/portTICK_PERIOD_MS);
  vTaskResume(task_2);// Resuming the suspended task and setting its state back to ready from where scheduler can set state to running
  vTaskDelay(2000/portTICK_PERIOD_MS);
  }
  UBaseType_t priority = uxTaskPriorityGet(NULL);
  Serial.println("Current task priority: " + String(priority));
  UBaseType_t priorityTask2 = uxTaskPriorityGet(task_2);
  Serial.println("Priority Task 2: " + String(priorityTask2));
  if(task_1!=NULL){
    Serial.println("Killing Task 1");
    vTaskDelete(task_1);
    task_1=NULL;
  }
}
