
#pragma once

typedef enum
{
    LOGIC_CMD_INVALID = 0,
    LOGIC_CMD_START_LEFT,
    LOGIC_CMD_START_RIGHT,
    LOGIC_CMD_STOP,
    LOGIC_EVENT_MOTOR,
} Logic_event_type;



void logic_send( Logic_event_type event );
