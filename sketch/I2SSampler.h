#ifndef __i2s_sampler_h__
#define __i2s_sampler_h__

#include "driver/i2s.h"

class I2SSampler
{
private:
    // double buffer so we can be capturing samples while sending data
    int16_t *m_audio_buffer1;
    int16_t *m_audio_buffer2;
    // current position in the audio buffer
    int32_t m_audio_buffer_pos = 0;
    // current audio buffer
    int16_t *m_current_audio_buffer;
    // buffer containing samples that have been captured already
    int16_t *m_captured_audio_buffer;
    // size of the audio buffers in bytes
    int32_t m_buffer_size_in_bytes;
    // size of the audio buffer in samples
    int32_t m_buffer_size_in_samples;
    // I2S reader task
    TaskHandle_t m_reader_task_handle;
    // writer task
    TaskHandle_t m_writer_task_handle;
    // i2s reader queue
    QueueHandle_t m_i2s_queue;
    // i2s port
    i2s_port_t m_i2s_port;

protected:
    void configureI2S();
    void processI2SData(uint8_t *i2sData, size_t bytes_read);
    void addSample(int16_t sample);
    i2s_port_t getI2SPort()
    {
        return m_i2s_port;
    }
public:
    int32_t getBufferSizeInBytes()
    {
        return m_buffer_size_in_bytes;
    };
    int16_t *getCapturedAudioBuffer()
    {
        return m_captured_audio_buffer;
    }
    void start(i2s_port_t i2sPort, i2s_pin_config_t &i2s_pins, i2s_config_t &i2s_config, int32_t buffer_size_in_samples, TaskHandle_t writer_task_handle);

    friend void i2sReaderTask(void *param);
};

#endif
