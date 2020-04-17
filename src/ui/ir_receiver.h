#pragma once

#include "./ui.h"

// Internal header file for IR subsystem of UI

void ir_pins_init( struct k_msgq* msg_queue );
void ir_receiver_code( uint8_t code );
void ui_received_keycode( u16_t keycode, bool is_ir, bool extra_info );

#define MAX_IR_REGISTRY_SIZE 12
#define MAX_IR_CODE_SIZE 64
