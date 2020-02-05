# zephyr-robot-supa2019
Zephyr based robot


## Pinout

Motor related

**Motor1 - left **

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
| --------------- | ----------------  | ----- |
| UART1-TX        |  PA9              | (gre) |
| UART1-RX        |  PA10             | (yel) |
| IR-Sensor       |  PA2 - ISR2       | (blu) |
| Emergency stop  |  PA1 - ISR1       |       |  
| I2C1 Data       | PB7 - I2C1_SDA    |       |
| I2C1 Clock      | PB6 - I2C1_SCL    |       |


**Big motor**

| Function        |    Pin           |   Info  |
| --------------- | ---------------- | ----- |
| Big Enable      | PA15 - DOUT       |  |
| Big current meas| PA3 - ADC1-3    |  |


## Motor control

Robotdyn shield: https://robotdyn.com/motor-shield-2a-l298p-2-motors-for-arduino.html

| Function        | A  |  B | PWM  |
| --------------- | -- | -- | ---  |
| Forward         | 1  | 0  |  X   |
| Backward        | 0  | 1  |  X   |
| Force stop      | 1  | 1  |  1   |
| Free roll       | 0  | 0  |  0   |




## HW Modifications for the STM32F411E-DISCO
* PE0 INT cutted away from the Gyro (as its required for TIM4 external clock - HAL input )
