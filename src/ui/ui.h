#pragma once

#include "../main.h"



typedef enum 
{
    IRKEY_VOL_UP   = 0xa05c,
    IRKEY_VOL_DOWN = 0xa458,
    IRKEY_LEFT     = 0xe418,
    IRKEY_RIGHT    = 0xe01c,
    IRKEY_FUNCTION = 0xd628,
    IRKEY_AUTO_ON  = 0xe31c,
    IRKEY_POWER    = 0x8478,
    IRKEY_PLAY     = 0xc13c,
    
    UI_SW_0        = 0x0010,
    UI_SW_1        = 0x0011,
    
} UI_keycode;

typedef void (*UICmd_callback)( UI_keycode code, bool repeated );
void ui_receiver_register( UI_keycode code, UICmd_callback callback );

#define UI_QUEUE_IR_MASK     0x03
#define UI_QUEUE_IR_1        0x01
#define UI_QUEUE_IR_0        0x02
#define UI_QUEUE_BUTTON_0    0x04
#define UI_QUEUE_BUTTON_1    0x08
#define UI_QUEUE_BUTTON_ACT  0x10
#define UI_QUEUE_BUTTON_MASK 0x1C
