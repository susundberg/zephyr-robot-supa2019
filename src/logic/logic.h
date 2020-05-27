
#pragma once

#define LOGIC_DISTANCE_MAX_CM 1000 // 10m should be enough for everyone..
#define LOGIC_DISTANCE_BACKUP_CM 20
#define LOGIC_DISTANCE_MAX_MAKE_SURE 40
#define LOGIC_ROTATE_ANGLE 175
#define LOGIC_DRIVE_SPEED_CM_S 12.0f
#define LOGIC_DISTANCE_STUCK_DRIVE_CM 10.0f
#define LOGIC_TRY_STUCK_CLEAR 3

typedef enum
{
    LOGIC_CMD_INVALID = 0,
    LOGIC_CMD_START_LEFT,
    LOGIC_CMD_START_RIGHT,
    LOGIC_CMD_STOP,
    LOGIC_EVENT_MOTOR,
} Logic_event_type;

void logic_send( Logic_event_type event );



