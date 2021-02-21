# ESP32-CAM
Arduino projects with the ESP32-CAM

## Introduction
You need
 - Hardware [ESP32-CAM](https://nl.aliexpress.com/item/1005001818136526.html)
 - Board [schematics](https://github.com/SeeedDocument/forum_doc/raw/master/reg/ESP32_CAM_V1.6.pdf)
 - Either a stand-alone [USB-Serial adapter](https://nl.aliexpress.com/item/4000016600649.html), 
   or a [dedicated board](https://nl.aliexpress.com/item/1005001810692306.html)
 - Wiring instructions are [here](https://randomnerdtutorials.com/program-upload-code-esp32-cam/) - do NOT connect 5V from FTDI to 3V3 of ESP32
 - Using the espressif [library](https://github.com/espressif/esp32-camera/tree/master/driver)
 
## Focus
You can change the focal length of the camera, by turning the cap, see [focus](focus) page.

## Harware mods
I applied [two hardware mods](hwmods): adding a low-power LED and an adapter board.
I later realized that the ESP32 has multiple [PWM channels](pwm), that you can map to any pin.
That's much easier, so I undid my low-power LED mod.

## Projects
My intention is to make a water-meter reader with a camera.
Let's see How far I get.

### Bringup
The first project, [esp32cam-ascii](esp32cam-ascii), captures an (grayscale QVGA) image, 
subsamples that (5x horizontal and vertical) and renders the results as ASCII art over serial.
This is the output while waving at the camera.

![Screenshot](esp32cam-ascii/screenshot.png)

### Collecting images
The second project, [esp32cam-cmd](esp32cam-cmd), consists of two parts.
The Arduino sketch captures an image and sends it to the host over USB/serial.
A [PC/Python](py-hex2png) program converts the serial dump to a png image.

![Captured png](py-hex2png/img.png)

There is a second [Python pogram](py-capture) that periodically sends capture commands 
to the Arduino Sketch, and automatically converts and saves the incoming bytes as an image.

(end)
 
