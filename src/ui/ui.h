#pragma once

#include "../main.h"



typedef enum 
{
    KEY_VOL_UP   = 0xa05c,
    KEY_VOL_DOWN = 0xa458,
    KEY_LEFT = 0xe418,
    KEY_RIGHT = 0xe01c,
    KEY_FUNCTION = 0xd628,
    KEY_AUTO_ON  = 0xe31c,
    KEY_POWER    = 0x8478,
    KEY_PLAY     = 0xc13c,
    
} IR_keycode;

typedef void (*IRCmd_callback)( IR_keycode code, bool repeated );
void ir_receiver_register( IR_keycode code, IRCmd_callback callback );

#define UI_QUEUE_IR_MASK     0x03
#define UI_QUEUE_IR_1        0x01
#define UI_QUEUE_IR_0        0x02
#define UI_QUEUE_BUTTON_0    0x04
#define UI_QUEUE_BUTTON_1    0x08
#define UI_QUEUE_BUTTON_ACT  0x10
#define UI_QUEUE_BUTTON_MASK 0x1C
