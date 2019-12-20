#pragma once

#include <stdlib.h>
#include <string.h>

#include <zephyr.h>
#include <logging/log.h>
#include <sys/printk.h>


#define FATAL_ERROR( fmt, ...) { \
   printk( "FATAL: " # fmt, ##__VA_ARGS__ );\
   k_fatal_halt(0xFFFF); }
   
   
   
   
   

