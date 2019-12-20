# zephyr-robot-supa2019
Zephyr based robot


## Pinout

Motor related

**Motor1**

| Function        |    Pin           |   Info  |
| -------------   | ---------------- | ------- |
| Hal input       | PD2 - TIM3_ETR   | (yel) |
| Motor Control A | PE9 - DOUT       | (yel) -> (Pin 9 on motor) |
| Motor Control B | PE10 - DOUT      | (ora) -> (Pin 8 on motor) |
| Motor PWM       | PA15 - TIM2_CH1  | (red) -> (Pin 10 on motor) |
| Bumber 1        | PE7 - ISR7       | (bla) |

**Motor2**

| Function        |    Pin           |   Info  |
| --------------- | ---------------- | ----- |
| Hal input       | PE0  - TIM4_ETR  | (ora) |
| Motor Control A | PE11 - DOUT      | (red) -> (Pin 9 on motor) |
| Motor Control B | PE12 - DOUT      | (gre) -> (Pin 8 on motor) |
| Motor PWM       | PA1  - TIM2_CH2  | (blu) |
| Bumber 2        | PE8 - ISR8       | (bro) |


**Other**

| Function        |    Pin           |   Info  |
| --------------- | ---------------- | ----- |
| IR-Sensor       | PC6 - ISR6       | (blu) |
| Emergency stop  | PD3 - ISR3       |  |

**Big motor**

| Function        |    Pin           |   Info  |
| --------------- | ---------------- | ----- |
| Big Enable      | PD8 - DOUT       |  |
| Big current meas| PC4 - ADC1-14    |  |


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
