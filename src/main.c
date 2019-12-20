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

#include "motors.h"


LOG_MODULE_REGISTER(app);

static const char* MOTOR_PWM_DRIVER[] = { DT_ALIAS_PWM_MOTOR_LEFT_PWMS_CONTROLLER, DT_ALIAS_PWM_MOTOR_RIGHT_PWMS_CONTROLLER };
static const u32_t MOTOR_PWM_CHANNEL[] = { DT_ALIAS_PWM_MOTOR_LEFT_PWMS_CHANNEL, DT_ALIAS_PWM_MOTOR_RIGHT_PWMS_CHANNEL };
static const u32_t MOTOR_PWM_PERIOD_US = (1000000/200); // 200Hz
typedef struct
{
    struct device* dev;
    u32_t channel;
    u32_t value; // 0 - 1000
} PWM_Motor;

typedef struct
{
   PWM_Motor motors[2];
   u32_t     period_us; // usec
} Motors;

Motors GLOBAL_motors;
uint32_t GLOBAL_error = 0;

#define FATAL_ERROR( fmt, ...)\
   LOG_ERR(fmt, ##__VA_ARGS__ );\
   k_fatal_halt(0xFFFF);
   


#define SHE_INF( fmt, ...) \
  shell_fprintf(shell, SHELL_NORMAL, fmt, ##__VA_ARGS__)

#define SHE_ERR( fmt, ...) \
  shell_fprintf(shell, SHELL_ERROR, fmt, ##__VA_ARGS__)

  
void Error_Handler()
{
    FATAL_ERROR("HAL Error_Handler()");
}


void motor_set_pwm( u32_t dev, u32_t value )
{
    
    u32_t pwm_value = (GLOBAL_motors.period_us/1000) * value;
    LOG_INF("PWM SET %u/%u", GLOBAL_motors.period_us, pwm_value );
    if (pwm_pin_set_usec(GLOBAL_motors.motors[dev].dev, GLOBAL_motors.motors[dev].channel,
                GLOBAL_motors.period_us, pwm_value )) 
    {
        FATAL_ERROR("Pwm Pin '%s/%d set failed", MOTOR_PWM_DRIVER[dev], MOTOR_PWM_CHANNEL[dev] );
        return;
    }
    
    GLOBAL_motors.motors[dev].value = value;
    
    
}
void motor_init()
{
    memset( &GLOBAL_motors, 0x00, sizeof(GLOBAL_motors));
    GLOBAL_motors.period_us = MOTOR_PWM_PERIOD_US;
    for ( int loop = 0; loop < 2; loop ++ )
    {
       GLOBAL_motors.motors[loop].dev = device_get_binding( MOTOR_PWM_DRIVER[loop] );
       GLOBAL_motors.motors[loop].channel = MOTOR_PWM_CHANNEL[loop];
       
       if (!GLOBAL_motors.motors[loop].dev)
       {
           FATAL_ERROR("Cannot get device '%s'", MOTOR_PWM_DRIVER[loop] );
           return;
       }
       
       motor_set_pwm( loop, 0 );
    }
    
}

void main(void)
{
    u32_t cnt = 0;
    struct device* dev;

    dev = device_get_binding( DT_ALIAS_LED0_GPIOS_CONTROLLER );
    
    gpio_pin_configure(dev, DT_ALIAS_LED0_GPIOS_PIN, GPIO_DIR_OUT);
    
    motor_init();
    motor_pos_init();
    
    LOG_INF("Robot control task started!");
    while (1) 
    {
      // Set pin to HIGH/LOW every 1 second */
      gpio_pin_write(dev, DT_ALIAS_LED0_GPIOS_PIN, cnt % 2);
      cnt++;
      k_sleep( 250 );
    }
}

static int parse_ul(const char *str, unsigned long *result)
{
    char *end;
    unsigned long val;

    val = strtoul(str, &end, 0);

    if (*str == '\0' || *end != '\0') {
        return -EINVAL;
    }

    *result = val;
    return 0;
}


static int parse_u32(const char *str, u32_t *result)
{
    unsigned long val;

    if (parse_ul(str, &val) || val > 0xFFFFFFFF) {
        return -EINVAL;
    }
    *result = (u32_t)val;
    return 0;
}


static int cmd_pwm_ramp(const struct shell *shell, size_t argc, char **argv)
{
   u32_t step;
   u32_t sleep_ms;

    if ( ( parse_u32(argv[1], &step) || parse_u32(argv[2], &sleep_ms))
         || ( step >= 10000 ) || sleep_ms > 100000 )
    {

        SHE_ERR("Invalid arguments.\n");
        return -EINVAL;
    }
    
    
    while (true)
    {
        for ( u32_t loop = 0; loop <= 1000; loop += step )
        {
           motor_set_pwm( 1, loop );    
           k_sleep( sleep_ms );
        }
        
        for ( u32_t loop = 1000; loop >= step ; loop -= step )
        {
           motor_set_pwm( 1, loop );
           k_sleep( sleep_ms );
        }
        
        
    }

 
    return 0; 

}



static int cmd_pwm_count(const struct shell *shell, size_t argc, char **argv)
{
    (void)argc;
    (void)argv;
    
    while(1)
    {
        u32_t left = 1 ;
        u32_t right = 1 ;
        
        motor_pos_get( &left, &right );
        SHE_INF("Counters are %u %u\n", left, right );
         k_sleep( 100 );
    }
}


static int cmd_pwm_set(const struct shell *shell, size_t argc, char **argv)
{
   u32_t channel;
   u32_t period;
   SHE_INF( "Got commands: %d\n", argc );
   
    if ( ( parse_u32(argv[1], &channel) || parse_u32(argv[2], &period))
         || ( channel >= 2 ) || period > 1000 )
    {

        SHE_ERR("Invalid arguments.\n");
        return -EINVAL;
    }
    
    SHE_INF("Setting PWM %s/%d value %d/1000\n", MOTOR_PWM_DRIVER[channel], MOTOR_PWM_CHANNEL[channel], period );
    motor_set_pwm( channel, period );

 
    return 0; 

}


SHELL_STATIC_SUBCMD_SET_CREATE(sub_pwm,
        SHELL_CMD_ARG(set, NULL, "set <channel> <period>", cmd_pwm_set, 3, 0 ),
        SHELL_CMD_ARG(ramp, NULL, "ramp <step> <sleep_ms>", cmd_pwm_ramp, 3, 0 ),
        SHELL_CMD_ARG(count, NULL, "count", cmd_pwm_count, 1, 0 ),                       
        SHELL_SUBCMD_SET_END
);
/* Creating root (level 0) command "demo" */
SHELL_CMD_REGISTER(pwm, &sub_pwm, "PWM commands", NULL);


