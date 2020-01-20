#pragma once

#include <stdlib.h>
#include <string.h>

#include <zephyr.h>
#include <logging/log.h>
#include <sys/printk.h>

void supa_fatal_handler( const char* module, int line );



#ifdef SUPA_MODULE

#define FATAL_ERROR( fmt, ...) { \
   supa_fatal_handler( SUPA_MODULE, __LINE__ ); }
#define ASSERT_ISR(x) { if ( (x) == false ) { supa_fatal_handler( SUPA_MODULE, __LINE__ ); } };
#define ASSERT(x) { if ( (x) == false ) { supa_fatal_handler( SUPA_MODULE, __LINE__ ); } };

#endif

#define HAL_CHECK(x) if ( (x) != HAL_OK) { FATAL_ERROR("HAL call failed!"); };
#define RET_CHECK(x) { int32_t __ret_check = (x); if ( __ret_check != 0 ) { FATAL_ERROR("Call failed: %d", __ret_check ); }};

#define ROUND_INT(x) ( (int)( x + 0.5) )
#define UNLIKELY(expr) ( __builtin_expect(!!(expr), 0))



#define OS_DEFAULT_STACKSIZE 1024
#define OS_DEFAULT_PRIORITY  7

   

