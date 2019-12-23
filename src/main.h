#pragma once

#include <stdlib.h>
#include <string.h>

#include <zephyr.h>
#include <logging/log.h>
#include <sys/printk.h>


#define FATAL_ERROR( fmt, ...) { \
   printk( "FATAL %s:%d: " # fmt, __FILE__, __LINE__, ##__VA_ARGS__ );\
   k_fatal_halt(0xFFFF); }
   
   
#define ASSERT(x) { if ( (x) == false ) { FATAL_ERROR("Assert failed" ); } };

#define OS_DEFAULT_STACKSIZE 1024
#define OS_DEFAULT_PRIORITY  7

   

