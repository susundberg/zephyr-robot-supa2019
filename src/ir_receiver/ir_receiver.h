#pragma once

#include "../main.h"



typedef enum 
{
    KEY_VOL_UP   = 0xa05c,
    KEY_VOL_DOWN = 0xa458,
    KEY_LEFT = 0xe418,
    KEY_RIGHT = 0xe01c,
    KEY_FUNCTION = 0xd628,
} IR_keycode;

typedef void (*IRCmd_callback)( IR_keycode code, bool repeated );

void ir_receiver_register( IR_keycode code, IRCmd_callback callback );

