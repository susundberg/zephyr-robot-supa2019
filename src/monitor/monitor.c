#include <zephyr.h>
#include <logging/log.h>
#include <drivers/adc.h>


#define SUPA_MODULE "mon"
LOG_MODULE_REGISTER(monitor);

#include "../main.h"
#include "monitor.h"

#define ADC_RESOLUTION		12
#define ADC_GAIN		ADC_GAIN_1
#define ADC_ACQUISITION_TIME	ADC_ACQ_TIME_DEFAULT
#define ADC_REFERENCE		ADC_REF_INTERNAL
#define VOLTAGE_REFERENCE_MV    3300

static struct device* LOCAL_adc_dev;


typedef struct {
    s32_t  value_scalar_a_f1000;
    s32_t  value_scalar_b_f1000; // value is scaled (a*X_mv + b)/1000 == value_mv
    s32_t  value_max_mv;
    s32_t  value_min_mv;
    s32_t value_filtering_f1000; // 1000 = do not filter, 100 = filter 90%
    s32_t adc_channel;
} AdcMonitorChannel;


static const AdcMonitorChannel CONFIG_channels[2] = {
    {
     .value_min_mv = 8000,   // do not run under 8V
     .value_max_mv = 16000,  // over 16 -> something wrong!
     .value_scalar_a_f1000 = 11000.0,  // (1.0k down 10k up.)
     .value_scalar_b_f1000 = 0.0,
     .value_filtering_f1000 = 800, 
     .adc_channel = DT_IO_CHANNELS_INPUT_BY_NAME( DT_NODELABEL(adc_map),  bvolt )
   },
   
   // (sensor_ref_mv - meas_mv) / 185mV = A
   // 1000*(sensor_ref_mv - meas_mv) / 185 = mA
   // sensor_ref = 2.5v (5V with voltage divide)
   // 
   // B = 2500/185mv * 1000 =  13513
   // A = -1000/185 * 1000 = 5405
    {
     .value_min_mv = -200000,   // do not run under 8V
     .value_max_mv =  200000,  // over 16 -> something wrong!
     .value_scalar_a_f1000 = 5405, // actually -5.4, lets make it larger.
     .value_scalar_b_f1000 = 13513,
     .value_filtering_f1000 = 900, 
     .adc_channel = DT_IO_CHANNELS_INPUT_BY_NAME( DT_NODELABEL(adc_map),  bmotor_curr ) 
   }
};

#define ADC_CHANNEL_N  (sizeof(CONFIG_channels)/sizeof(AdcMonitorChannel))
static s32_t LOCAL_adc_value[ADC_CHANNEL_N];




static void read_one_channel(u8_t channel_id, s32_t* value_mv )
{
    s16_t value_raw = 0;
    const struct adc_sequence sequence = {
        .channels    = BIT(channel_id) ,
        .buffer      = &value_raw,
        .buffer_size = sizeof(s16_t),
        .resolution  = ADC_RESOLUTION,
    };


    RET_CHECK( adc_read(LOCAL_adc_dev, &sequence) );
    *value_mv = value_raw;
    RET_CHECK( adc_raw_to_millivolts( VOLTAGE_REFERENCE_MV, ADC_GAIN, ADC_RESOLUTION, value_mv ) )
}

static s32_t monitor_channel_read( u8_t channel_index )
{
    s32_t value_new = 0;
    const AdcMonitorChannel* channel = &CONFIG_channels[ channel_index ];
    
    read_one_channel( channel->adc_channel, &value_new );
    
    
    s32_t value_conv = ( (value_new * channel->value_scalar_a_f1000 + channel->value_scalar_b_f1000) ) / 1000;
    
    s32_t value_filt  = 
         ( channel->value_filtering_f1000*LOCAL_adc_value[channel_index] 
         + (1000-channel->value_filtering_f1000)*value_conv 
         + 500 ) / 1000;
         
    //LOG_INF("ADC RAW: %d %d %d %d", channel_index, value_new, value_conv, value_filt );
    LOCAL_adc_value[channel_index] = value_filt;
    
    return value_conv;
}

static void monitor_channel( u8_t channel_index )
{

    const AdcMonitorChannel* channel = &CONFIG_channels[ channel_index ];
    monitor_channel_read( channel_index );
    
    if ( LOCAL_adc_value[channel_index] > channel->value_max_mv )
    {
        LOG_ERR("ADC %d value %d over limit (%d)", channel_index, LOCAL_adc_value[channel_index], channel->value_max_mv );
        FATAL_ERROR("ADC overvalue!");
    }
    if ( LOCAL_adc_value[channel_index] <  channel->value_min_mv )
    {
        LOG_ERR("ADC %d value %d under limit (%d)", channel_index, LOCAL_adc_value[channel_index], channel->value_min_mv );
        FATAL_ERROR("ADC undervalue!");
    }    
}





void monitor_init()
{
    const char* adc_device_name 
          = DT_IO_CHANNELS_LABEL_BY_NAME( DT_NODELABEL(adc_map),  bmotor_curr );
    DEV_GET_CHECK( LOCAL_adc_dev, adc_device_name );
    
    
    
    for ( int loop = 0; loop < ADC_CHANNEL_N; loop ++ )
    {
        const struct adc_channel_cfg adc_config = {
                .gain             = ADC_GAIN,
                .reference        = ADC_REFERENCE,
                .acquisition_time = ADC_ACQUISITION_TIME,
                .channel_id       = CONFIG_channels[loop].adc_channel
                };
                
        RET_CHECK( adc_channel_setup(LOCAL_adc_dev, &adc_config) );
        s32_t value_conv = monitor_channel_read( loop );
        LOCAL_adc_value[loop] = value_conv;
        
    }
    
    LOG_INF("ADC setup done!");
}




void monitor_update()
{
    static u8_t loop = 0 ;
    for ( int loop = 0; loop < ADC_CHANNEL_N; loop ++ )
    {
        monitor_channel( loop );
        
    }
    loop++;
    
    if (( loop & 0b111) == 0 )
    {
       LOG_INF("Adc monitor values %d %d",  LOCAL_adc_value[0], LOCAL_adc_value[1] );
    }
}




