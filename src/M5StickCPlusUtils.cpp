#include "M5StickCPlusUtils.h"

// Buffer para armazenar dados lidos e variáveis globais
uint8_t BUFFER[READ_LEN] = {0}; // Buffer para dados de I2S
uint16_t oldy[240]; // Armazena as posições verticais anteriores para a visualização
int16_t *adcBuffer = NULL; // Buffer para os dados do ADC
int8_t doorState; // Estado da porta (aberta/fechada)
float rmsValue; // Valor RMS calculado
bool dataIsReady = false; // Indica se há dados prontos para processamento
int lcdLitButton = 39; // Pin do botão para ativar a tela
volatile int lcdLit = 1; // Estado da luz da tela
volatile unsigned long lcdTime = 15000UL; // Tempo para apagar a tela
volatile unsigned long lcdTimeBefore = 0; // Marca de tempo anterior para controle da tela

/**
 * Controla o brilho da tela LCD.
 * Desliga a tela se o tempo especificado já tiver passado.
 */
void lcdControlBrightness()
{
  if (lcdLit)
    if (((millis() - lcdTimeBefore) > lcdTime))
    {
      M5.Lcd.writecommand(ST7789_DISPOFF);
      M5.Axp.ScreenBreath(0);
      lcdLit = 0;
      Serial.println("Desliga tela");
    }
}

/**
 * Função de tarefa que ativa a tela LCD ao pressionar um botão.
 * Executa em um loop, verificando o estado do botão.
 */
void wakeUpLcd(void *arg)
{
  while (1)
  {
    lcdControlBrightness(); // Controla o brilho da tela
    if (!digitalRead(lcdLitButton)) // Se o botão está pressionado
    {
      lcdLit = 1; // Ativa a tela
      lcdTimeBefore = millis(); // Reinicia o temporizador
      M5.Lcd.writecommand(ST7789_DISPON);
      M5.Axp.ScreenBreath(40);
      Serial.println("Liga tela");
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

/**
 * Mostra o sinal na tela LCD.
 * Desenha o sinal de áudio no gráfico, atualizando a posição vertical.
 */
void showSignal()
{
  int y;
  for (int n = 0; n < 240; n++)
  {
    y = adcBuffer[n] * GAIN_FACTOR * H_SCALING; // Escala o sinal
    y = (map(y, INT16_MIN, INT16_MAX, 10, 70)) * V_SCALING; // Mapeia o sinal à altura da tela
    for (int i = 0; i < 3; i++)
      M5.Lcd.drawPixel(n, oldy[n] + i, RED); // Desenha o sinal anterior
    for (int i = 0; i < 3; i++)
      M5.Lcd.drawPixel(n, y + i, BLACK); // Desenha o novo sinal
    oldy[n] = y; // Atualiza a posição anterior
  }
}

/**
 * Calcula o valor RMS a partir de um buffer de dados do ADC.
 * @param buffer: Ponteiro para o buffer de dados.
 * @param length: Comprimento do buffer.
 * @return O valor RMS calculado.
 */
float calculateRMS(int16_t *buffer, int length)
{
  long sumOfSquares = 0;
  for (int i = 0; i < length; i++)
  {
    sumOfSquares += buffer[i] * buffer[i]; // Soma dos quadrados
  }
  float meanSquare = (float)sumOfSquares / length; // Média dos quadrados
  return sqrt(meanSquare); // Retorna a raiz quadrada
}

/**
 * Converte um valor RMS para decibéis.
 * @param Leq_RMS: Valor RMS.
 * @return O valor em decibéis.
 */
float calculateDB(float Leq_RMS)
{
  float Leq_dB = MIC_OFFSET_DB + MIC_REF_DB + 20 * log10(Leq_RMS / MIC_REF_AMPL);
  return Leq_dB; // Retorna o valor em dB
}

/**
 * Mostra os dados e informações na tela LCD.
 * Executa em um loop, atualizando constantemente a tela.
 */
void displayDataAndInfo(void *arg)
{
  while (1)
  {
    float batVoltage = M5.Axp.GetBatVoltage(); // Obtém a voltagem da bateria
    float batPercentage = (batVoltage < 3.2) ? 0 : (batVoltage - 3.2) * 100; // Calcula o percentual de bateria
    // Mostra a intensidade do som
    M5.Lcd.setCursor(0, 3);
    M5.Lcd.setTextColor(TXT_COLOR, BG_COLOR);
    M5.Lcd.setTextSize(1);
    M5.Lcd.printf(" Bateria: %.0f%c%sWiFi RSSI: %d \n", batPercentage, '%', " ", WiFi.RSSI());
    M5.Lcd.printf(" Carregando: %s\n", ((M5.Axp.GetBatChargeCurrent()) > 0 ? "sim" : "não "));
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf(" Intensidade do Som: \n %f\n", rmsValue);
    M5.Lcd.printf(" Estado da Porta: \n");
    M5.Lcd.setTextColor(BLUE, (doorState ? GREENYELLOW : RED));
    M5.Lcd.printf(" %s\n", (doorState ? "fechada" : "aberta"));
    Serial.print("Valor RMS: ");
    Serial.println(rmsValue);
    Serial.print("Valor do estado da porta: ");
    Serial.println(doorState);
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

/**
 * Função que obtém dados do sensor em um loop.
 * Lê dados do I2S, calcula o valor RMS e verifica o estado da porta.
 */
void getData(void *arg)
{
  size_t bytesread;
  while (1)
  {
    i2s_read(I2S_NUM_0, (char *)BUFFER, READ_LEN, &bytesread, (100 / portTICK_RATE_MS));
    adcBuffer = (int16_t *)BUFFER; // Aponta para o buffer de dados lidos
    rmsValue = calculateRMS(adcBuffer, READ_LEN / 2); // /2 porque READ_LEN está em bytes
    doorState = (int8_t)digitalRead(PIN_DOOR); // Lê o estado da porta
    dataIsReady = true; // Indica que os dados estão prontos
    vTaskDelay(99 / portTICK_RATE_MS);
  }
}

/**
 * Função de monitoramento que envia dados para uma função de callback.
 * Verifica se há dados prontos e executa ações correspondentes.
 */
void monitor_task(void *arg)
{
  void (*zabSend)(float, int8_t) = (void (*)(float, int8_t))arg; // Ponteiro para a função de callback
  while (1)
  {
    if (dataIsReady)
    {
      zabSend(rmsValue, doorState); // Envia os dados para a função de callback
      digitalWrite(PIN_LED, (rmsValue <= 1000)); // Liga ou desliga o LED com base no valor RMS
      dataIsReady = false; // Reseta o sinal de dados prontos
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/**
 * Inicializa o I2S para leitura de áudio.
 * Configura os parâmetros do I2S e instala o driver.
 */
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
  pin_config.ws_io_num = PIN_CLK; // Pin de clock
  pin_config.data_out_num = I2S_PIN_NO_CHANGE;
  pin_config.data_in_num = PIN_DATA; // Pin de dados

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL); // Instala o driver I2S
  i2s_set_pin(I2S_NUM_0, &pin_config); // Configura os pinos I2S
  i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO); // Configura o clock do I2S
}

/**
 * Configura o M5StickC Plus.
 * Inicializa o dispositivo, configura a tela e os pinos.
 */
void setupM5StickCPlus(void *zab_s)
{
  M5.begin(); // Inicializa o M5
  M5.Axp.EnableCoulombcounter(); // Ativa o contador de coulombs
  M5.Lcd.setRotation(3); // Define a rotação da tela
  M5.Lcd.fillScreen(BG_COLOR); // Preenche a tela com a cor de fundo
  M5.Axp.ScreenBreath(40); // Define o brilho da tela
  M5.Lcd.setTextColor(TXT_COLOR, BG_COLOR);
  M5.Lcd.println("teste do microfone"); // Mensagem inicial
  pinMode(PIN_LED, OUTPUT); // Configura o pin do LED como saída
  pinMode(PIN_DOOR, INPUT_PULLUP); // Configura o pin da porta como entrada com pull-up
  pinMode(lcdLitButton, INPUT); // Configura o pin do botão da tela como entrada

  // Inicializa o I2S e cria as tarefas
  i2sInit();
  xTaskCreate(monitor_task, "tarefa de monitoramento", 2 * 2048, zab_s, 1, NULL);
  xTaskCreate(getData, "tarefa de obter dados do sensor", 2 * 2048, NULL, 1, NULL);
  xTaskCreate(wakeUpLcd, "ativar lcd", 2048, NULL, 2, NULL);
  xTaskCreate(displayDataAndInfo, "Mostrar bateria, rms e estado da porta", 3 * 2048, NULL, 2, NULL);
}
