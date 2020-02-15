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
#include <power/reboot.h>
#include <device.h>
#include <fatal.h>
#include <init.h>


#include <drivers/pwm.h>
#include <drivers/gpio.h>

#include <logging/log.h>
#include <logging/log_ctrl.h>

#include <shell/shell.h>
#include <shell/shell_uart.h>

#define SUPA_MODULE "mai"
#include "main.h"
#include "motor/motors.h"
#include "ir_receiver/ir_receiver.h"
#include "logic/logic.h"

#include "utils/utils.h"
#include "utils/boot.h"

LOG_MODULE_REGISTER(app);


#define SHE_INF( fmt, ...) \
  shell_fprintf(shell, SHELL_NORMAL, fmt, ##__VA_ARGS__)

#define SHE_ERR( fmt, ...) \
  shell_fprintf(shell, SHELL_ERROR, fmt, ##__VA_ARGS__)

  
void supa_fatal_handler( const char* module, int line )
{
    printk("\nFATAL ERROR %s:%d\n", module, line );
    k_sys_fatal_error_handler(0xFFFF, NULL );
}
  
static void ircmd_move( IR_keycode code, bool repeated );
static void ircmd_function( IR_keycode code, bool repeated );
static void ircmd_auto_on( IR_keycode code, bool repeated );
static void ircmd_power( IR_keycode code, bool repeated );

void k_sys_fatal_error_handler(unsigned int reason, const z_arch_esf_t *esf)
{
    ARG_UNUSED(esf);
    motors_abort();
    LOG_PANIC();
    sys_reboot( SYS_REBOOT_COLD );
    CODE_UNREACHABLE;
}

void main(void)
{
    u32_t cnt = 0;
    struct device* dev;

    dev = device_get_binding( DT_ALIAS_LED0_GPIOS_CONTROLLER );
    RET_CHECK( gpio_pin_configure( dev, DT_ALIAS_LED0_GPIOS_PIN, GPIO_OUTPUT | DT_ALIAS_LED0_GPIOS_FLAGS ) );
    
    
    ir_receiver_register( KEY_VOL_UP, ircmd_move );
    ir_receiver_register( KEY_VOL_DOWN, ircmd_move );
    ir_receiver_register( KEY_LEFT, ircmd_move);
    ir_receiver_register( KEY_RIGHT, ircmd_move );    
    ir_receiver_register( KEY_FUNCTION, ircmd_function );
    ir_receiver_register( KEY_AUTO_ON, ircmd_auto_on );
    ir_receiver_register( KEY_POWER, ircmd_power );
    
    LOG_INF("Robot control task started!");
    while (1) 
    {
      gpio_pin_set(dev, DT_ALIAS_LED0_GPIOS_PIN, cnt % 2);
      cnt++;
      k_sleep( 250 );
    }
}



static void ircmd_move( IR_keycode code, bool repeated )
{
    float params[3] = {0};
    
    static const float MOTOR_TEST_TURN_ANGLE     = 90.0f;
    static const float MOTOR_TEST_DRIVE_DISTANCE = 20.0f;
    
    if (repeated)
        return;
    
    uint32_t opcode = 0;
    switch (code)
    {
        case KEY_VOL_UP:
            params[0] = MOTOR_TEST_DRIVE_DISTANCE;
            params[1] = MOTOR_TEST_DRIVE_DISTANCE;
            params[2] = MOTOR_MAX_SPEED_CM_S;
            opcode    = MOTOR_CMD_DRIVE;
            break;
        case KEY_VOL_DOWN:
            params[0] = -MOTOR_TEST_DRIVE_DISTANCE;
            params[1] = -MOTOR_TEST_DRIVE_DISTANCE;
            params[2] = MOTOR_MAX_SPEED_CM_S;
            opcode    = MOTOR_CMD_DRIVE;
            break;            
        case KEY_LEFT:
            params[0] = -MOTOR_TEST_TURN_ANGLE;
            opcode    = MOTOR_CMD_ROTATE;
            break;        
        case KEY_RIGHT:
            params[0] = +MOTOR_TEST_TURN_ANGLE;
            opcode    = MOTOR_CMD_ROTATE;
            break; 
        default:
            ASSERT(0);
            return;
    }
    motors_send_cmd( opcode, params, 3 );
}

static void ircmd_function( IR_keycode code, bool repeated )
{
    static bool value = false;
    
    if ( repeated ) 
        return;

    value = !value;
    motors_control_function( value );
}

static void ircmd_power( IR_keycode code, bool repeated )
{
    if ( repeated ) 
        return;

    motors_send_cmd( MOTOR_CMD_STOP, NULL, 0 );
    motors_control_function( false ); 
       
}

static void ircmd_auto_on( IR_keycode code, bool repeated )
{
    if ( repeated ) 
        return;
    
    logic_toggle();
}

static int cmd_reboot(const struct shell *shell, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    ARG_UNUSED(shell);
    SHE_INF("Reboot requested now!\n");
    sys_reboot( SYS_REBOOT_COLD );
    return 0;
}

static int cmd_bootloader(const struct shell *shell, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    ARG_UNUSED(shell);
    SHE_INF("Reboot BOOTLOADER!\n");
    supa_bootloader_enable();
    sys_reboot( SYS_REBOOT_COLD );
    return 0;
}




static int cmd_motor_stop(const struct shell *shell, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    ARG_UNUSED(shell);
    motors_send_cmd( MOTOR_CMD_STOP, NULL, 0 );
    return 0; 
}


static int cmd_motor_test(const struct shell *shell, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    ARG_UNUSED(shell);

   int32_t params[3];
   
    if ( parse_i32(argv[1], &params[0]) || parse_i32(argv[2], &params[1]) || parse_i32(argv[3], &params[2] ) )
    {

        SHE_ERR("Invalid arguments.\n");
        return -EINVAL;
    }
    
    motors_send_cmd( MOTOR_CMD_TEST, params, 3 );
    return 0; 
}

static int cmd_motor_pid(const struct shell *shell, size_t argc, char **argv)
{
   int params[3];
   ARG_UNUSED(argc);
   
   if ( parse_i32(argv[1], &params[0]) || parse_i32(argv[2], &params[1]) || parse_i32(argv[3], &params[2] ) )
   {

       SHE_ERR("Invalid arguments.\n");
       return -EINVAL;
   }
   float params_flt[3] = { params[0] / 100.0f, params[1] / 100.0f, params[2] / 100.0f }; 
    
   SHE_INF("Setting PID=%d %d %d /100\n", params[0],  params[1], params[2] );
   motors_set_pid( 0, params_flt );
   motors_set_pid( 1, params_flt );
   return 0;
}


static int cmd_motor_drive(const struct shell *shell, size_t argc, char **argv)
{
   int params[3];
   ARG_UNUSED(argc);
    if ( parse_i32(argv[1], &params[0]) || parse_i32(argv[2], &params[1]) || parse_i32(argv[3], &params[2] ) )
    {

        SHE_ERR("Invalid arguments.\n");
        return -EINVAL;
    }
    float params_flt[3] = { params[0] / 10.0f, params[1] / 10.0f, params[2] / 10.0f }; 
    
    motors_send_cmd( MOTOR_CMD_DRIVE, params_flt, 3 );
    return 0; 
}



SYS_INIT( supa_bootloader_check, PRE_KERNEL_1, 0 );

SHELL_STATIC_SUBCMD_SET_CREATE(sub_pwm,
        SHELL_CMD_ARG(drive, NULL, "drive <dist_left mm> <dist_right mm> <speed mm/sec>", cmd_motor_drive, 4, 0 ),
        SHELL_CMD_ARG(stop, NULL, "stop <>", cmd_motor_stop, 1, 0 ),
        SHELL_CMD_ARG(pid, NULL, "pid <>", cmd_motor_pid, 4, 0 ),                       
        SHELL_CMD_ARG(test, NULL, "test <time_ramp_sec> <time_const_sec> <max_speed cm/sec>", cmd_motor_test, 4, 0 ),
        SHELL_SUBCMD_SET_END
);
/* Creating root (level 0) command "demo" */
SHELL_CMD_REGISTER(motor, &sub_pwm, "MOTOR commands", NULL);
SHELL_CMD_ARG_REGISTER(reset, NULL, "RESTART system", cmd_reboot, 0, 0);
SHELL_CMD_ARG_REGISTER(bootloader, NULL, "REBOOT system to bootloader", cmd_bootloader, 0, 0);

