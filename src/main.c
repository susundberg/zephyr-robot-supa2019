/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file Sample app to demonstrate PWM.
 *
 * This app uses PWM[0].
 */

#include <stdlib.h>
#include <string.h>

#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>

#include <drivers/pwm.h>
#include <drivers/gpio.h>

#include <logging/log.h>
#include <shell/shell.h>
#include <shell/shell_uart.h>

#include "motor/motors.h"


LOG_MODULE_REGISTER(app);


#define SHE_INF( fmt, ...) \
  shell_fprintf(shell, SHELL_NORMAL, fmt, ##__VA_ARGS__)

#define SHE_ERR( fmt, ...) \
  shell_fprintf(shell, SHELL_ERROR, fmt, ##__VA_ARGS__)

  


void main(void)
{
    u32_t cnt = 0;
    struct device* dev;

    dev = device_get_binding( DT_ALIAS_LED0_GPIOS_CONTROLLER );
    gpio_pin_configure(dev, DT_ALIAS_LED0_GPIOS_PIN, GPIO_DIR_OUT);
    
    
    LOG_INF("Robot control task started!");
    while (1) 
    {
      gpio_pin_write(dev, DT_ALIAS_LED0_GPIOS_PIN, cnt % 2);
      cnt++;
      k_sleep( 250 );
    }
}

static int parse_long(const char *str, long int *result)
{
    char *end;
    long int val;

    val = strtol(str, &end, 0);

    if (*str == '\0' || *end != '\0') {
        return -EINVAL;
    }

    *result = val;
    return 0;
}


static int parse_u32(const char *str, u32_t *result)
{
    long val;

    if (parse_long(str, &val) || val > UINT32_MAX || val < 0 ) 
    {
        return -EINVAL;
    }
    *result = (u32_t)val;
    return 0;
}

static int parse_i32(const char *str, int32_t *result)
{
    long val;

    if (parse_long(str, &val) || val < INT32_MIN || val > INT32_MAX ) 
    {
        return -EINVAL;
    }
    *result = (int32_t)val;
    return 0;
}





static int cmd_motor_stop(const struct shell *shell, size_t argc, char **argv)
{
    (void)argc;
    (void)argv;
    (void)shell;
    motors_send_cmd( MOTOR_CMD_STOP, NULL, 0 );
    return 0; 
}


static int cmd_motor_test(const struct shell *shell, size_t argc, char **argv)
{
    (void)argc;
    (void)argv;
    (void)shell;

   int32_t params[3];
   
    if ( parse_i32(argv[1], &params[0]) || parse_i32(argv[2], &params[1]) || parse_i32(argv[3], &params[2] ) )
    {

        SHE_ERR("Invalid arguments.\n");
        return -EINVAL;
    }
    
    motors_send_cmd( MOTOR_CMD_TEST, params, 3 );
    return 0; 
}

static int cmd_motor_drive(const struct shell *shell, size_t argc, char **argv)
{
   int32_t params[2];
   
    if ( parse_i32(argv[1], &params[0]) || parse_i32(argv[2], &params[1] ) )
    {

        SHE_ERR("Invalid arguments.\n");
        return -EINVAL;
    }
    
    motors_send_cmd( MOTOR_CMD_DRIVE, params, 2 );
    return 0; 
}



SHELL_STATIC_SUBCMD_SET_CREATE(sub_pwm,
        SHELL_CMD_ARG(drive, NULL, "drive <dist_left> <dist_right>", cmd_motor_drive, 3, 0 ),
        SHELL_CMD_ARG(stop, NULL, "stop <>", cmd_motor_stop, 1, 0 ),
        SHELL_CMD_ARG(test, NULL, "test <time_ramp_sec> <time_const_sec> <max_speed cm/sec>", cmd_motor_test, 4, 0 ),

        SHELL_SUBCMD_SET_END
);
/* Creating root (level 0) command "demo" */
SHELL_CMD_REGISTER(motor, &sub_pwm, "MOTOR commands", NULL);


