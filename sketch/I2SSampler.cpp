#include <Arduino.h>
#include "driver/i2s.h"

#include "I2SSampler.h"

void I2SSampler::addSample(int16_t sample)
{
    // add the sample to the current audio buffer
    m_current_audio_buffer[m_audio_buffer_pos] = sample;
    m_audio_buffer_pos++;
    // have we filled the buffer with data?
    if (m_audio_buffer_pos == m_buffer_size_in_samples)
    {
        // swap to the other buffer
        std::swap(m_current_audio_buffer, m_captured_audio_buffer);
        // reset the buffer position
        m_audio_buffer_pos = 0;
        // tell the writer task to save the data
        xTaskNotify(m_writer_task_handle, 1, eIncrement);
    }
}

void I2SSampler::processI2SData(uint8_t *i2sData, size_t bytes_read)
{
    int32_t *samples = (int32_t *)i2sData;
    for (int i = 0; i < bytes_read / 4; i++)
    {
        // you may need to vary the >> 11 to fit your volume - ideally we'd have some kind of AGC here
        addSample(samples[i] >> 11);
    }
}


void i2sReaderTask(void *param)
{
    I2SSampler *sampler = (I2SSampler *)param;
    while (true)
    {
        // wait for some data to arrive on the queue
        i2s_event_t evt;
        if (xQueueReceive(sampler->m_i2s_queue, &evt, portMAX_DELAY) == pdPASS)
        {
            if (evt.type == I2S_EVENT_RX_DONE)
            {
                size_t bytes_read = 0;
                do
                {
                    // read data from the I2S peripheral
                    uint8_t i2s_data[1024];
                    // read from i2s
                    i2s_read(sampler->getI2SPort(), i2s_data, 1024, &bytes_read, 10);
                    // process the raw data
                    sampler->processI2SData(i2s_data, bytes_read);
                } while (bytes_read > 0);
            }
        }
    }
}

void I2SSampler::start(i2s_port_t i2s_port, i2s_pin_config_t &i2s_pins, i2s_config_t &i2s_config, int32_t buffer_size_in_bytes, TaskHandle_t writer_task_handle)
{
    m_i2s_port = i2s_port;
    m_writer_task_handle = writer_task_handle;
    m_buffer_size_in_samples = buffer_size_in_bytes / sizeof(int16_t);
    m_buffer_size_in_bytes = buffer_size_in_bytes;
    m_audio_buffer1 = (int16_t *)malloc(buffer_size_in_bytes);
    m_audio_buffer2 = (int16_t *)malloc(buffer_size_in_bytes);

    m_current_audio_buffer = m_audio_buffer1;
    m_captured_audio_buffer = m_audio_buffer2;

    //install and start i2s driver
    i2s_driver_install(m_i2s_port, &i2s_config, 4, &m_i2s_queue);
    // set up the I2S pins
    i2s_set_pin(getI2SPort(), &i2s_pins);
    // start a task to read samples from the ADC
    TaskHandle_t reader_task_handle;
    xTaskCreatePinnedToCore(i2sReaderTask, "i2s Reader Task", 4096, this, 1, &reader_task_handle, 0);
}
