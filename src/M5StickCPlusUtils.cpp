#include "M5StickCPlusUtils.h"
uint8_t BUFFER[READ_LEN] = {0};
uint16_t oldy[240];
int16_t *adcBuffer = NULL;
int8_t doorState;

void showSignal()
{
  int y;
  for (int n = 0; n < 240; n++)
  {
    y = adcBuffer[n] * GAIN_FACTOR * H_SCALING;
    y = (map(y, INT16_MIN, INT16_MAX, 10, 70))*V_SCALING;
    for (int i = 0; i < 3; i++)
      M5.Lcd.drawPixel(n, oldy[n] + i, RED);
    for (int i = 0; i < 3; i++)
      M5.Lcd.drawPixel(n, y + i, BLACK);
    oldy[n] = y;
  }
}
float calculateRMS(int16_t *buffer, int length)
{
  // Function to calculate RMS value from the ADC buffer float calculateRMS (int16_t *buffer, int length) {
  long sumOfSquares = 0;
  for (int i = 0; i < length; i++)
  {
    sumOfSquares += buffer[i] * buffer[i];
  }
  float meanSquare = (float)sumOfSquares / length;
  return sqrt(meanSquare);
}

void showData(float rmsValue)
{
  // Display sound intensity
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.fillScreen(WHITE); // Clear the screen
  M5.Lcd.setTextColor(BLACK, WHITE);
  M5.Lcd.setTextSize(2); // Set the text size to 2 (adjust as needed)
  M5.Lcd.printf(" Sound Intensity: \n %f\n", rmsValue);
  M5.Lcd.printf(" Door State: \n");
  M5.Lcd.setTextColor(BLACK, (doorState ? GREENYELLOW : RED));
  M5.Lcd.printf(" %s\n", (doorState ? "closed" : "open"));
  Serial.print("RMS Value: ");
  Serial.println(rmsValue);
  Serial.print("doorState Value: ");
  Serial.println(doorState);
}

void monitor_task(void *arg)
{
  void (*zabSend)(float, int8_t) = (void (*)(float, int8_t))arg;
  size_t bytesread;
  unsigned long before = millis();
#define DT (millis() - before)
  while (1)
  {
    i2s_read(I2S_NUM_0, (char *)BUFFER, READ_LEN, &bytesread, (100 / portTICK_RATE_MS));
    adcBuffer = (int16_t *)BUFFER;
    // Calculate RMS value and map to a percentage
    float rmsValue = calculateRMS(adcBuffer, READ_LEN / 2); // /2 because READ_LEN is in byte
    doorState = (int8_t)digitalRead(PIN_DOOR);
    if (DT > 100){
      zabSend(rmsValue, doorState);
      before = millis();  
    }
    showData(rmsValue);
    showSignal();
    Serial.println(rmsValue);
    digitalWrite(PIN_LED, (rmsValue <= 1000));
  }
}
void i2sInit()
{
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
      .sample_rate = 44100,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(4, 1, 0)
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
#else
      .communication_format = I2S_COMM_FORMAT_I2S,
#endif
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 2,
      .dma_buf_len = 128,
  };
  i2s_pin_config_t pin_config;
#if (ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(4, 3, 0))
  pin_config.mck_io_num = I2S_PIN_NO_CHANGE;
#endif
  pin_config.bck_io_num = I2S_PIN_NO_CHANGE;
  pin_config.ws_io_num = PIN_CLK;
  pin_config.data_out_num = I2S_PIN_NO_CHANGE;
  pin_config.data_in_num = PIN_DATA;
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}
void setupM5StickCPlus(void *zab_s)
{
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setTextColor(BLACK, WHITE);
  M5.Lcd.println("mic test");
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_DOOR, INPUT_PULLUP);
  i2sInit();
  xTaskCreate(monitor_task, "mic_record_task", 2048, zab_s, 1, NULL);
}