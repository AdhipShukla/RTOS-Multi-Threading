#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

void Task1(void *parameter){
  while(1){
    int a = 1;
    int array[60];
    for (int i =0; i<60 ; i++){
      array[i]=a+1;
    }
    Serial.println(array[0]);
    //PRINTING OUT REMAINING STACK MEMORY IN WORDS
    Serial.println("High Water Mark Words: " + String(uxTaskGetStackHighWaterMark(NULL)));//This reported in words so to get the number of bytes it has to be multiplied by 4 in a 32 bit system


    //PRINING OUT THE NUMBER OF FREE HEAP MEMORY BYTES AVIAVLABLE BEFORE MALLOC
    Serial.println("Heap befre malloc(bytes)" + String(xPortGetFreeHeapSize()));

    int *ptr= (int*)pvPortMalloc(1024*sizeof(int));
    if(ptr==NULL){
      Serial.println("We ran out of memory");
    }
    else{
      for (int i=0;i<1024;i++){
        ptr[i]=1; //Doing something with the memory so that compiler does not optimize it
      }
    }
    //Checking heap after allocating
    Serial.println("Heap befre malloc(bytes)" + String(xPortGetFreeHeapSize()));
    vPortFree(ptr);//Freeing up the memory used in each while loop
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("______________FreeRTOS Memory Management_____________");
  xTaskCreatePinnedToCore(Task1,
                           "Test Task 1",
                           1024,//768 bytes are overhaead for every task (As per overhead, according to muy test it is more as only 64 more bytes are accomodated)
                           NULL,
                           1,
                           NULL,
                           app_cpu);
  vTaskDelete(NULL);// Deleteing the current tasks
}

void loop() {
  // put your main code here, to run repeatedly:

}
