# zephyr-robot-supa2019
Zephyr based robot

For pictures and some details see [Hackaday.io](https://hackaday.io/project/170682-lawm-mowing-robot-with-stm32-blue-pill-and-zephyr)

## Pinout

Motor related

**Motor1 - left**

| Function        |    Pin           |   Info  |
| -------------   | ---------------- | ------- |
| Hal input       | PA12 - TIM1_ETR   | (yel) |
| Motor Control A | PB12 - DOUT       | (yel) -> (Pin 9 on motor) |
| Motor Control B | PB13 - DOUT      | (ora) -> (Pin 8 on motor) |
| Motor PWM       | PA6 - TIM3_CH1  | (red) -> (Pin 10 on motor) |
| Bumber 1        | PA11 - ISR11       | (red) |


**Motor2 - right**

| Function        |    Pin           |   Info  |
| --------------- | ---------------- | ----- |
| Hal input       | PA0  - TIM2_ETR  | (ora) |
| Motor Control A | PB14 - DOUT      | (red) -> (Pin 9 on motor) |
| Motor Control B | PB15 - DOUT      | (gre) -> (Pin 8 on motor) |
| Motor PWM       | PA7  - TIM3_CH2  | (blu) |
| Bumber 2        | PA8 - ISR8       | (blue) |


**Other**

| Function        |    Pin            |   Info  |
| --------------- | ----------------  | -----   |
| UART1-TX        |  PA9              | (gre)   |
| UART1-RX        |  PA10             | (yel)   |
| Imu I2C         | PB7 - I2C1_SDA    |         |
| Imu I2C         | PB6 - I2C1_SCL    |         |
| Imu isr         | PB5 - ISR5        |         |
| UI IR RCV       |  PB3 - ISR3       |         |  
| UI SW1          |  PA2 - ISR2       |                |
| UI SW0          |  PA1 - ISR1       |                |  
| UI LD0          |  PC13             | System led + UI led 0          | 
| UI LD1          |  PC14             | UI led 1- TODO (max sink 3ma!) |


**Big motor**


| Function         |    Pin              |   Info   | Notes         | 
| ---------------  | ------------------- | -------- | ------------- |
| Big Enable       | PB8 - TIM4_CH3 PWM  | (ora)    | Now only GPIO |
| Big current meas | PA3 - ADC1-3        |          | TODO NOT CONN |
| Battery volt meas| PA4 - ADC1-4        |          | TODO NOT CONN |



## Motor control

Robotdyn shield: https://robotdyn.com/motor-shield-2a-l298p-2-motors-for-arduino.html

| Function        | A  |  B | PWM  |
| --------------- | -- | -- | ---  |
| Forward         | 1  | 0  |  X   |
| Backward        | 0  | 1  |  X   |
| Force stop      | 1  | 1  |  1   |
| Free roll       | 0  | 0  |  0   |

