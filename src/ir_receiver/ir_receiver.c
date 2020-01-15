#include <zephyr.h>


#include <drivers/gpio.h>

#include "ir_receiver.h"

LOG_MODULE_REGISTER(ir);

static TIM_HandleTypeDef LOCAL_tim_ir;

static const int   LOCAL_pin_pins[2] = { DT_GPIO_LEDS_IR_INPUT_GPIOS_PIN, DT_GPIO_LEDS_IR_OUTPUT_GPIOS_PIN };
struct device*     LOCAL_pin_dev[2];
static const char* LOCAL_pin_names[] = { DT_GPIO_LEDS_IR_INPUT_GPIOS_CONTROLLER, DT_GPIO_LEDS_IR_OUTPUT_GPIOS_CONTROLLER };

#define   LOCAL_isr_buffer_n 12*4
static u8_t      LOCAL_isr_buffer[LOCAL_isr_buffer_n];
static bool LOCAL_isr_state = 0;
static int  LOCAL_isr_loop  = 0;
#define LOCAL_isr_queue_n 4
static char __aligned(4) LOCAL_ir_queue_buffer[ LOCAL_isr_queue_n * LOCAL_isr_buffer_n ];
static struct k_msgq LOCAL_ir_queue;

static u32_t DEBUG_nsamples = 0;


bool timers_check_update_event( TIM_HandleTypeDef* htim    )
{
  if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_UPDATE) != RESET)
  {
    if (__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_UPDATE) != RESET)
    {
      __HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE);
      return true;
    }
  }
  return false;
}


static void ir_send_isr_code( u8_t* code )
{
    k_msgq_put( &LOCAL_ir_queue, code, K_NO_WAIT );
}

typedef enum
{
    STATE_IDLE = 0,
    STATE_OVERFLOW, 
    STATE_MARK,
    STATE_SPACE,
    STATE_STOP,
    STATE_STOP2,
} IrStateMachineState;

typedef struct
{
    u32_t timer;
    u32_t rawlen;
    u8_t rawbuf[LOCAL_isr_buffer_n];
    
    IrStateMachineState rcvstate;
    bool overflow;
    
} IrStateMachine;


IrStateMachine LOCAL_ir_state;

static void ir_state_machine( uint32_t irdata )
{

    // Read if IR Receiver -> SPACE [xmt LED off] or a MARK [xmt LED on]
    // digitalRead() is very slow. Optimisation is possible, but makes the code unportable
    const u32_t MARK  = 0;
    const u32_t SPACE = 1;
    const u32_t GAP_TICKS = 100;

    LOCAL_ir_state.timer++;  // One more 50uS tick
    
    if ( LOCAL_ir_state.rcvstate != STATE_STOP )
    {
       if ( LOCAL_ir_state.rawlen >= LOCAL_isr_buffer_n )
           LOCAL_ir_state.rcvstate = STATE_OVERFLOW ;  // Buffer overflow
    }   
    
    ASSERT_ISR ( irdata == MARK || irdata == SPACE ) ;
    
    switch(LOCAL_ir_state.rcvstate) 
    {
        //......................................................................
        case STATE_IDLE: // In the middle of a gap
            if (irdata == MARK) {
                if (LOCAL_ir_state.timer < GAP_TICKS)  {  // Not big enough to be a gap.
                    LOCAL_ir_state.timer = 0;

                } else {
                    // Gap just ended; Record duration; Start recording transmission
                    LOCAL_ir_state.overflow                  = false;
                    LOCAL_ir_state.rawlen                    = 0;
                    LOCAL_ir_state.rawbuf[LOCAL_ir_state.rawlen++] = LOCAL_ir_state.timer;
                    LOCAL_ir_state.timer                     = 0;
                    LOCAL_ir_state.rcvstate                  = STATE_MARK;
                }
            }
            break;
        //......................................................................
        case STATE_MARK:  // Timing Mark
            if (irdata == SPACE) {   // Mark ended; Record time
                LOCAL_ir_state.rawbuf[LOCAL_ir_state.rawlen++] = LOCAL_ir_state.timer;
                LOCAL_ir_state.timer                     = 0;
                LOCAL_ir_state.rcvstate                  = STATE_SPACE;
            }
            break;
        //......................................................................
        case STATE_SPACE:  // Timing Space
            if (irdata == MARK) {  // Space just ended; Record time
                LOCAL_ir_state.rawbuf[LOCAL_ir_state.rawlen++] = LOCAL_ir_state.timer;
                LOCAL_ir_state.timer                     = 0;
                LOCAL_ir_state.rcvstate                  = STATE_MARK;

            } else if (LOCAL_ir_state.timer > GAP_TICKS) {  // Space
                    // A long Space, indicates gap between codes
                    // Flag the current code as ready for processing
                    // Switch to STOP
                    // Don't reset timer; keep counting Space width
                    LOCAL_ir_state.rcvstate = STATE_STOP;
                    ir_send_isr_code( LOCAL_ir_state.rawbuf );
                    
            }
            break;
        //......................................................................
        case STATE_STOP:  // Waiting; Measuring Gap
            memset( &LOCAL_ir_state, 0x00, sizeof(LOCAL_ir_state));
            break;
        //......................................................................
        case STATE_OVERFLOW:  // Flag up a read overflow; Stop the State Machine
            LOCAL_ir_state.overflow = true;
            LOCAL_ir_state.rcvstate = STATE_STOP;
            ir_send_isr_code( LOCAL_ir_state.rawbuf );
            break;
            
        default:
            break;
    }
    

}

static int DEBUG_val = 0;

void TIM1_TRG_COM_TIM11_IRQHandler(void)
{

   ASSERT_ISR( timers_check_update_event( &LOCAL_tim_ir ) == true );

   u32_t val = 0U;
   
   ASSERT_ISR( gpio_pin_read( LOCAL_pin_dev[0], LOCAL_pin_pins[0], &val) == 0 );
   
   ir_state_machine( val );
//    if ( LOCAL_ir_state.rcvstate == STATE_STOP )
//    {
//        ir_send_isr_code( LOCAL_isr_buffer );
//        LOCAL_ir_state.rcvstate = STATE_STOP2;
//    }
   
//    DEBUG_val += (1 - val);
//    
//    
//  
//    if ( ( val == 1 ) && (LOCAL_isr_state == false ) )
//        return;
//    
//    if ( LOCAL_isr_state == false )
//    {
//       // We are at beginning of transmission
//        LOCAL_isr_loop = 0;
//        memset( LOCAL_isr_buffer, 0x00, sizeof(LOCAL_isr_buffer));
//        LOCAL_isr_state = true;
//    }
//    
//    u32_t bit_shift = (LOCAL_isr_loop & 0x07 );
//    u32_t bit_locat = (LOCAL_isr_loop & 0xF8 );
//    
//    u8_t bit_mask  = (val << bit_shift);
//    
//    LOCAL_isr_buffer[ bit_locat ] |= bit_mask; 
//    
//    if ( LOCAL_isr_loop >= LOCAL_isr_buffer_n*8 )
//    {
//        LOCAL_isr_state = false;
//        ir_send_isr_code( LOCAL_isr_buffer );
//    }
//    
//    LOCAL_isr_loop += 1;
   
   
}

  
static void ir_pins_init()
{
    for (int loop = 0; loop < 2; loop ++ )
    {
       LOCAL_pin_dev[loop] = device_get_binding( LOCAL_pin_names[loop] );
       
       if ( LOCAL_pin_dev[loop] == NULL )
       {
           FATAL_ERROR("Cannot find device: %s", LOCAL_pin_names[loop] );
           return;
       }
    }
    
    int loop = 0;
    if ( gpio_pin_configure( LOCAL_pin_dev[loop], LOCAL_pin_pins[loop], GPIO_DIR_IN ) != 0 )
    {
           FATAL_ERROR("Cannot configure device: %s-%d", LOCAL_pin_names[loop], LOCAL_pin_pins[loop] );
           return;
    }
       
    loop = 1;
    if ( gpio_pin_configure( LOCAL_pin_dev[loop], LOCAL_pin_pins[loop], GPIO_DIR_OUT ) != 0 )
    {
           FATAL_ERROR("Cannot configure device: %s-%d", LOCAL_pin_names[loop], LOCAL_pin_pins[loop] );
           return;
    }
    ASSERT( gpio_pin_write( LOCAL_pin_dev[loop], LOCAL_pin_pins[loop], 0 ) == 0 );    
}



static void ir_timer_init()
{
    
   __HAL_RCC_TIM11_CLK_ENABLE();

  HAL_NVIC_SetPriority( TIM1_TRG_COM_TIM11_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ( TIM1_TRG_COM_TIM11_IRQn );
    
  LOCAL_tim_ir.Instance               = TIM11;
  LOCAL_tim_ir.Init.Prescaler         = 99; // clock is 100 000khz / 100 = 1Mhz
  LOCAL_tim_ir.Init.CounterMode       = TIM_COUNTERMODE_UP;
  LOCAL_tim_ir.Init.Period            = 50; // we want 50 us 
  LOCAL_tim_ir.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  LOCAL_tim_ir.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  
  
  HAL_CHECK( HAL_TIM_Base_Init(&LOCAL_tim_ir) );
   
}

#define TIM11_IRQ    TIM1_TRG_COM_TIM11_IRQn
#define TIM11_PRIOR  2 // Not sure

static void ir_receiver_main()
{
   
   k_msgq_init( &LOCAL_ir_queue, LOCAL_ir_queue_buffer, LOCAL_isr_buffer_n, LOCAL_isr_queue_n );

   ASSERT( LOCAL_isr_buffer_n == sizeof(u32_t)*12 );
   
   ir_timer_init();
   ir_pins_init();
    
   LOG_INF("IR thread started!");
   
   IRQ_CONNECT( TIM11_IRQ, TIM11_PRIOR, TIM1_TRG_COM_TIM11_IRQHandler, NULL, 0x00);
   irq_enable( TIM11_IRQ );
   
   k_sleep(500);
   
   printk("Start IT NOW!");
   HAL_CHECK( HAL_TIM_Base_Start_IT(&LOCAL_tim_ir) );
   uint32_t loop = 0;
   while(1)
   {
      u32_t code[4*3];

      if ( k_msgq_get(&LOCAL_ir_queue, code, 100 ) == 0  )
      {
         printk("Received: 0x%04X%04X%04X%04X\n", code[0],code[1],code[2],code[3] );   
         printk("          0x%04X%04X%04X%04X\n", code[4],code[5],code[6],code[7] ); 
         printk("          0x%04X%04X%04X%04X\n", code[4+4],code[4+5],code[4+6],code[4+7] ); 
      }
       
      gpio_pin_write( LOCAL_pin_dev[1], LOCAL_pin_pins[1], loop % 2);
      loop += 1;
      
          // K_FOREVER);
      
   }
}



K_THREAD_DEFINE( ir_thread, OS_DEFAULT_STACKSIZE*2, ir_receiver_main, NULL, NULL, NULL,
                 OS_DEFAULT_PRIORITY, 0, K_NO_WAIT);


