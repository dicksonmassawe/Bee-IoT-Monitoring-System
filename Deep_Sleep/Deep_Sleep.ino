// This test programme for deep sleep
int TIME_TO_SLEEP = 5;
unsigned long long uS_TO_S_FACTOR = 1000000;
RTC_DATA_ATTR int bootCount = 0; // Variable to store how many times we have boot.

void setup() {
  // What to do
  Serial.begin(115200);
  ++bootCount; // bootCounter Incrementer
  Serial.println("Boot Number: " + String(bootCount));
  
  // Set wake_up source
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  // Prepare for sleep
  Serial.flush();
  Serial.println("Sleeping");

  // Enable deep sleep
  esp_deep_sleep_start();

}

void loop() {
  // put your main code here, to run repeatedly:

}
