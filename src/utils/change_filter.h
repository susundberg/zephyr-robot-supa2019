#pragma once

#include <zephyr.h>

typedef struct 
{
    s32_t value_last;
    s32_t value_th;
} ChangeFilter;

bool change_filtered( s32_t new_value, ChangeFilter* filter );

