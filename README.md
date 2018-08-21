# STM32F4Discovery as a USB audio card

This project is based on examples provided by ST in USB device library. I wanted to keep code simple so I completely stripped 'STM32 Standard Peripheral Library' and replaced it with direct register writes/reads. Also the device class code is condensed into one place so it's easier to understand how the USB library is working.

## Building

To build this project make sure you have an ARM GCC Toolchain (i really recommend [bleeding-edge-toolchain](https://github.com/FreddieChopin/bleeding-edge-toolchain) by FreddieChopin), make, git and coreutils. To start first clone this repository then compile it:
```
git clone https://github.com/Polprzewodnikowy/stm32f4_usb_to_audio
cd stm32f4_usb_to_audio
make
```
When build is finished you can flash uC on the board with your favourite tool (i.e. STM32 ST-Link Utility or openocd). After connecting board to the PC with USB user port you should be able to see new audio card in device manager on Windows and whatever else there is on Linux. Hope you enjoy it ;)
