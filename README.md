[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/Z8Z734F5Y)
# ICS-43434 Sample Code

You can watch the wiring and build video (here)[https://youtu.be/VVGln_-XEgI].

[![Demo Video](https://img.youtube.com/vi/VVGln_-XEgI/0.jpg)](https://www.youtube.com/watch?v=VVGln_-XEgI)

The circuit board schematic and PCB layout are available [here](https://easyeda.com/chris_9044/ics-43434-tindie)

This project demonstrates how to use the ESP32 with the ICS-43434 break out board.

You can buy the board on both ebay and Tindie:

- eBay: https://www.ebay.co.uk/itm/154115095985
- Tindie: coming soon

There are two projects in this repository: `i2s_sampling` and `server`.

## server

This is a simple `node` server that writes the samples received from the ESP32 to a file.

## i2s_sampling

This project demonstrates how to use the I2S peripheral for high-speed sampling using DMA to transfer samples directly to RAM.

The current set of pin assignment in the code are:

### I2S

| Function    | GPIO Pin    | Notes                          |
| ----------- | ----------- | ------------------------------ |
| bck_io_num  | GPIO_NUM_5  | I2S - Serial clock             |
| ws_io_num   | GPIO_NUM_19 | I2S - LRCLK - left right clock |
| data_in_num | GPIO_NUM_18 | I2S - Serial data              |
