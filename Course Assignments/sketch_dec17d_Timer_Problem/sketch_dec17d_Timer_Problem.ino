#define led_pin 14
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu =0;
#else
static const BaseType_t app_cpu =0;
#endif
static TimerHandle_t LEDTime = NULL;
static int LED;
void LED_OFF(TimerHandle_t xTimer){
  LED=0;
  digitalWrite(led_pin, LOW);
  Serial.println("LED Status: " + String(LED));
}
void LEDTimer(void *para){
  char c={0};
  pinMode(led_pin, OUTPUT);
  xTimerStart(LEDTime, portMAX_DELAY);
  while(1){
    while(Serial.available()>0){
      LED=1;
      digitalWrite(led_pin, HIGH);
      xTimerStart(LEDTime, portMAX_DELAY);//Reset the timer upon entering again
      c=Serial.read();
      Serial.print(c);
      if(c=='\n'){
        Serial.println("LED Status: " + String(LED));
        Serial.println("Enter Something to glow LED: ");
        break;
      }
    }
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(600);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println("_____Timer Challenge_____");
  Serial.println("Enter Something to glow LED: ");
  LED=1;
  Serial.println("Initial LED Status: " + String(LED));
  LEDTime = xTimerCreate( "LED-Timer",
                5000/portTICK_PERIOD_MS,
                pdFALSE,
                (void *)0,
                LED_OFF);
  xTaskCreatePinnedToCore(LEDTimer,
                          "LEDTimeManage",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);
  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}
