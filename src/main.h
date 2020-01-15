#pragma once

#include <stdlib.h>
#include <string.h>

#include <zephyr.h>
#include <logging/log.h>
#include <sys/printk.h>


#define FATAL_ERROR( fmt, ...) { \
   __ASSERT( 0,  "FATAL %s:%d: " # fmt, __FILE__, __LINE__, ##__VA_ARGS__  ); }
#define ASSERT_ISR(x) { if ( (x) == false ) { __ASSERT(0, "ASS ISR %s:%d", __FILE__ , __LINE__ ); } };
#define ASSERT(x) { if ( (x) == false ) { __ASSERT(0, "ASS %s:%d", __FILE__ , __LINE__ ); } };

#define HAL_CHECK(x) if ( (x) != HAL_OK) { FATAL_ERROR("HAL call failed!"); };

#define ROUND_INT(x) ( (int)( x + 0.5) )
#define UNLIKELY(expr) ( __builtin_expect(!!(expr), 0))

#define OS_DEFAULT_STACKSIZE 1024
#define OS_DEFAULT_PRIORITY  7

   

