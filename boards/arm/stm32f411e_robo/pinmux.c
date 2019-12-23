/*
 * Copyright (c) 2017 Fenix Engineering Solutions
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <device.h>
#include <init.h>
#include <drivers/pinmux.h>
#include <sys/sys_io.h>

#include <pinmux/stm32/pinmux_stm32.h>

/* pin assignments for STM32F411E-DISCO board */
static const struct pin_config pinconf[] = {
#ifdef CONFIG_UART_2
	{STM32_PIN_PA2, STM32F4_PINMUX_FUNC_PA2_USART2_TX},
	{STM32_PIN_PA3, STM32F4_PINMUX_FUNC_PA3_USART2_RX},
#endif	/* CONFIG_UART_2 */
    
// CONFIG_SUPA_MOTOR_PWM
       {STM32_PIN_PA1, (STM32_PINMUX_ALT_FUNC_1 | STM32_PUSHPULL_NOPULL) },
       {STM32_PIN_PA15, (STM32_PINMUX_ALT_FUNC_1 | STM32_PUSHPULL_NOPULL) },

// CONFIG_SUPA_MOTOR_PULSE_COUNT
       {STM32_PIN_PD2, (STM32_PINMUX_ALT_FUNC_2 | STM32_PUSHPULL_NOPULL) },
       {STM32_PIN_PE0, (STM32_PINMUX_ALT_FUNC_2 | STM32_PUSHPULL_NOPULL) },


};

static int pinmux_stm32_init(struct device *port)
{
	ARG_UNUSED(port);

	stm32_setup_pins(pinconf, ARRAY_SIZE(pinconf));
   __HAL_RCC_GPIOE_CLK_ENABLE();
	return 0;
}

SYS_INIT(pinmux_stm32_init, PRE_KERNEL_1,
		CONFIG_PINMUX_STM32_DEVICE_INITIALIZATION_PRIORITY);
