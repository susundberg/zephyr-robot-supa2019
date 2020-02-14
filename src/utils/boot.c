

#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>
#include <stm32f1xx_ll_rtc.h>

#define SUPA_BOOTLOADER_VALUE 0xF1FA


static void set_bootloader( uint32_t value )
{
     __HAL_RCC_PWR_CLK_ENABLE();
     __HAL_RCC_BKP_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();
    
    LL_RTC_DisableWriteProtection( RTC );
    LL_RTC_BKP_SetRegister( BKP, LL_RTC_BKP_DR1, value);
    LL_RTC_EnableWriteProtection( RTC );
    HAL_PWR_DisableBkUpAccess();
}



static void jump_to_bootloader()
{
    void (*bootloader_fun)(void);
    const uint32_t addr =  0x1FFFF000 ; // Seems to be valid to STM32F1xxx devices - check AN2606

    /**
     * Step: Set jump memory location for system memory
     *       Use address with 4 bytes offset which specifies jump location where program starts
     */
    bootloader_fun = (void (*)(void)) (*((uint32_t *)(addr + 4)));
    
    /**
     * Step: Set main stack pointer.
     *       This step must be done last otherwise local variables in this function
     *       don't have proper value since stack pointer is located on different position
     *
     *       Set direct address location which specifies stack pointer in SRAM location
     */
    __set_MSP(*(uint32_t *)addr);
    
    /**
     * Step: Actually call our function to jump to set location
     *       This will start system memory execution
     */
    bootloader_fun();
}


uint32_t supa_bootloader_get()
{
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_BKP_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();
    
    return LL_RTC_BKP_GetRegister( BKP, LL_RTC_BKP_DR1 );  
}



int supa_bootloader_check()
{

    uint32_t bootmode = supa_bootloader_get();
    if ( bootmode == SUPA_BOOTLOADER_VALUE )
    {
        set_bootloader( 0x00 );
        jump_to_bootloader();
    }
    return 0;
}

void supa_bootloader_enable()
{
    set_bootloader( SUPA_BOOTLOADER_VALUE );
}


