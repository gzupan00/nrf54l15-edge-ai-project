# nrf54l15-edge-ai-project

Real-time embedded object detection project developed using the Nordic Semiconductor nRF54L15 DK, OV7670 camera module, TFT display, and Edge Impulse FOMO object detection model.

## Features

- Live image capture using OV7670 camera
- Real-time TFT display output
- Embedded object detection using Edge AI
- Bounding box visualization
- Zephyr RTOS based application

## Hardware

- Nordic Semiconductor nRF54L15 DK
- OV7670 camera module with AL422B FIFO buffer
- 1.8-inch ST7735R TFT display

## Software

- Zephyr RTOS
- nRF Connect SDK
- Edge Impulse
- Visual Studio Code
- C/C++

## Running Inference

After flashing the firmware to the nRF54L15 DK, the TFT display continuously shows the live camera feed from the OV7670 module.

Object detection inference is executed by pressing Button 1 on the development board. After the button is pressed, the captured image is processed by the Edge Impulse FOMO model and the detection results are displayed on the TFT screen together with bounding boxes.

Detection confidence values and bounding box coordinates are also printed through the serial terminal at 115200 baud.

## Dataset

The dataset contains 616 manually collected images captured using the OV7670 camera module. The dataset includes both images containing plastic bottles and images without bottles, including background scenes and other objects.

Different lighting conditions, bottle positions, and backgrounds were included during data collection and testing.

## Repository Contents

- Embedded application source code
- Zephyr configuration files
- Dataset archive
- Project report

## Author

Candidate number: 8503
