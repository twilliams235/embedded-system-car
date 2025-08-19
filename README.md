### MSP430 Line-Following IoT Car

An embedded systems project for the TI MSP430 that follows a black line with IR sensing and accepts remote drive commands over Wi-Fi from a phone. Firmware includes a wheel-control state machine, PWM motor drive, UART-connected IoT module, ADC-based line detection, and an LCD status display. 

Each "project" built on the last one, project 10 is the final project we had to drive through an obstacle course, then press a button to find and follow a black line.

## Features

- **Autonomous line following** using left/right IR reflectance sensors (ADC A2/A3) with a state machine that seeks the line, aligns, and traces it by modulating motor PWMs. Thresholding logic decides when each wheel advances or pauses. 

- **Phone control over Wi-Fi:** firmware brings up an IoT module and opens a server on port 8091; single-character commands handle forward, reverse, left, right, stop, timed moves, turbo, and “follow line” mode. 

- **Motor PWM on Timer B3** driving right/left forward/reverse channels (P6.1–P6.4). 

- **LCD status** (IP, state, timer) and DAC/analog front-end initialization. 

- **Clocking @ 8 MHz**, watchdog configuration, and low-power safe port unlock. 

- **Calibrate & control via buttons**: SW1/SW2 use debounced interrupts (also used for white/black calibration helpers). 

## Hardware Overview

- **MCU**: TI MSP430 (MCLK/SMCLK = 8 MHz). 

- **IR sensors**: left/right photodiodes on analog inputs A2/A3, sampled in a round-robin ADC ISR (thumb wheel on A5). 

- **Motors**: H-bridge driven by PWM (Timer B3):

P6.1 RIGHT_FORWARD, P6.2 RIGHT_REVERSE, P6.3 LEFT_FORWARD, P6.4 LEFT_REVERSE. 

- **IoT module (UART)**: UCA0 TX/RX routed on Port 1; enable pin P3.7 (IOT_EN). 

- **IR LED drive**: P2.2 IR_LED. 

- **LCD & SPI pins**: configured in port setup. 

(See ports.c for full pin mapping.) 

## How It Works
#Boot & Setup

On reset the firmware initializes ports, clocks, timers, LCD, ADC, DAC, and serial ports; it then toggles IOT_EN and shows status on the LCD (“ENABLING”). 

The IoT process configures the module using AT commands:

- AT+SYSSTORE=0

- AT+CIPMUX=1

- AT+CIPSERVER=1,8091 (opens server on 8091)
It then displays the IP on the LCD once available. 

# Line Following

Wheels_Process() runs a state machine: find white → intercept black → align → trace. While tracing, ADC thresholds gate each wheel (both on, left off, right off) to stay on the line; a special BL CIRCLE stop/hold is displayed for verification. 

# Remote Driving

Incoming bytes from the IoT UART are buffered and parsed. Commands trigger actions like Forward, Backward, Left, Right, Stop, Go, Turbo, Follow Line, and Dismount; timed moves use a countdown handled in the Timer B0 ISR. 
 

# Timing & Control Loops

- Timer B3 generates motor PWMs. 

- Timer B0 handles periodic tasks, wait timers, line-circle delays, IoT bring-up delays, and movement runtimes; it also refreshes an on-screen timer. 

- ADC ISR cycles A2 → A3 → A5 and updates display strings. 

- UART ISRs (UCA0/UCA1) implement ring buffers and pass-through for IoT/USB terminals. 

# Project Structure

- main.c – System bring-up, peripheral init, main loop calling IOT_Process() and Wheels_Process(). 

- buffer.c – IoT ring buffers, AT command bring-up, IP display, command parser → motion/state triggers. 

- wheels.c – Wheel/line-following state machine and PWM decisions from ADC thresholds. 

- timers.c – Timer B0/B3 setup, ADC ISR, UART ISRs, debounce/utility timers, PWM channels. 

- ports.c – Pin muxing for UART/LCD/SPI/ADC/IR LED and H-bridge pins. 

- clocks.c – 8 MHz DCO, ACLK setup, watchdog handling. 

- init.c – LCD update helper and DAC/reference initialization.