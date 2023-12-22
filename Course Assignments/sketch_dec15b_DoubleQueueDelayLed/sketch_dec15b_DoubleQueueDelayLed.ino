#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif
static int LED =14;
static const uint8_t TQueLen = 10;
static const uint8_t MQueLen = 20;
static QueueHandle_t TimerQueue;
static QueueHandle_t MessageQueue;

void MesPrint(void *parameter){
  //Serial.println("TASK11");
  char NewMes[10];
  char Delay[7] = {'D','e','l','a','y',':','\0'};
  char DelayIn[7]={'\0'};
  int DelayVal=0;
  bool isDelayMes=1;
  char EchoChar;
  int MesLen =-1;
  bool ThisCycDel=0;
  bool ThisCycDelVal=0;
  while(1){
    //Serial.println("TASK12");
    if(xQueueReceive(MessageQueue,(void *)NewMes,1)==pdTRUE){
      Serial.println();
      Serial.println("Message from Queue2");
      Serial.println(NewMes);
    }
    while(Serial.available()>0){
      //Serial.println("TASK13");
      //Serial.println(isDelayMes);
      MesLen++;
      //Serial.println(MesLen);
      EchoChar = Serial.read();
      //Serial.println(EchoChar);
      if(isDelayMes){
        if (EchoChar==Delay[MesLen] && MesLen<6){
          //Serial.println("TASK15");
          DelayIn[MesLen] = EchoChar;
          ThisCycDel=1;
        }
        else if(EchoChar>=48 && EchoChar<=57 && MesLen<10 && MesLen>=6){
          //Serial.println("TASK16");
          DelayVal = DelayVal*10+EchoChar-48;
          ThisCycDelVal=1;
        }
        else{
          //Serial.println("TASK17");
          if(ThisCycDel==1){
            Serial.print(DelayIn);
          }
          if(ThisCycDelVal==1){
            Serial.print(DelayVal);
          }
          Serial.print(EchoChar);
          if(EchoChar=='\n'){
          //Serial.println("TASK14");
          break;
          }
          isDelayMes=0;
        }
      }
      else{
        //Serial.println("TASK18");
        Serial.print(EchoChar);
      }
    }
    if(isDelayMes==1 && MesLen>6){
      xQueueSend(TimerQueue, (void *)&DelayVal,10);
    }
    isDelayMes=1;
    MesLen=-1;
    DelayVal=0;
    ThisCycDelVal=0;
    ThisCycDel=0;
    for(int i=0;i<7;i++){
      DelayIn[i]='\0';
    }
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}


void LEDBlink(void *parameter){
  int DelayTime =1000;
  int cnt=0;
  const char* newMes = "NewMes";
  while(1){
    if(xQueueReceive(TimerQueue, &DelayTime, 1)==pdTRUE){
      Serial.println();
      Serial.print("NewDelay");
      Serial.print(DelayTime);
    }
    digitalWrite(LED, HIGH);
    vTaskDelay(DelayTime/portTICK_PERIOD_MS);
    digitalWrite(LED, LOW);
    vTaskDelay(DelayTime/portTICK_PERIOD_MS);
    //Serial.println("LEDBlink");
    cnt++;
    if(cnt==30){
      cnt=0;
      xQueueSend(MessageQueue, newMes, 0);
    }
  }
}

void setup() {
  vTaskDelay(1000/portTICK_PERIOD_MS);
  pinMode(LED, OUTPUT);
  Serial.begin(600);
  Serial.println("_____Custom LED Frequency_____");
  MessageQueue=xQueueCreate(MQueLen,sizeof(char)*6);// Keep a note to add the number of bytes you want yout queue receive to read here
  TimerQueue=xQueueCreate(TQueLen,sizeof(int));
  xTaskCreatePinnedToCore(MesPrint,
                          "MessagePrint",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);
  xTaskCreatePinnedToCore(LEDBlink,
                          "LEDBlink",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);
}

void loop() {
  // put your main code here, to run repeatedly:

}
