#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "I2SSampler.h"

// replace the ip address with your machine's ip address
#define I2S_SERVER_URL "http://192.168.1.184:5003/i2s_samples"
#define PING_URL "http://192.168.1.184:5003/ping"
// replace the SSID and PASSWORD with your WiFi settings
#define SSID "<<YOUR_SSID>>"
#define PASSWORD "<<YOUR_PASSWORD>>"

// i2s config - this is set up to read fro the left channel
i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

// i2s pins
i2s_pin_config_t i2s_pins = {
    .bck_io_num = GPIO_NUM_5,
    .ws_io_num = GPIO_NUM_19,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = GPIO_NUM_18};

I2SSampler *i2s_sampler = NULL;

// Task to write samples to our server
void i2sWriterTask(void *param)
{
  I2SSampler *sampler = (I2SSampler *)param;
  WiFiClient *wifi_client = new WiFiClient();
  HTTPClient *http_client = new HTTPClient();
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
  while (true)
  {
    // wait for some samples to save
    uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    if (ulNotificationValue > 0)
    {
      Serial.println("Sending data");
      http_client->begin(*wifi_client, I2S_SERVER_URL);
      http_client->addHeader("content-type", "application/octet-stream");
      http_client->POST((uint8_t *)sampler->getCapturedAudioBuffer(), sampler->getBufferSizeInBytes());
      http_client->end();
      Serial.println("Sent Data");
    }
  }
}

// Ping task - pings the server every 5 seconds so we know the connection is good
void pingTask(void *param)
{
  WiFiClient *wifi_client_ping = new WiFiClient();
  HTTPClient *http_client_ping = new HTTPClient();
  ;
  const TickType_t wait_time = pdMS_TO_TICKS(5000);
  while (true)
  {
    Serial.println("Ping Server Start");
    http_client_ping->begin(*wifi_client_ping, PING_URL);
    http_client_ping->GET();
    http_client_ping->end();
    Serial.println("Ping Server Done");
    vTaskDelay(wait_time);
  }
}

void setup()
{
  Serial.begin(115200);
  // launch WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.println("Started up");

  // set up i2s to read from our microphone
  i2s_sampler = new I2SSampler();

  // set up the i2s sample writer task
  TaskHandle_t writer_task_handle;
  xTaskCreate(i2sWriterTask, "I2S Writer Task", 4096, i2s_sampler, 1, &writer_task_handle);

  // start sampling from i2s device
  i2s_sampler->start(I2S_NUM_1, i2s_pins, i2s_config, 32768, writer_task_handle);

  // set up the ping task
  TaskHandle_t ping_task_handle;
  xTaskCreate(pingTask, "Ping Task", 4096, nullptr, 1, &ping_task_handle);
}

void loop()
{
  // nothing to do here - it's all driven by the i2s peripheral reading samples
}
