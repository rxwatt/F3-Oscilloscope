#ifndef __OSCILLOSCOPE_H
#define __OSCILLOSCOPE_H

#include "stm32f3xx_hal.h"

#define ADC_BUFFER_LENGTH 8000
#define ADC_CLK_PARAM_NUMBER 16

#define OSCIL_CH1 0x01
#define OSCIL_CH2 0x02
#define OSCIL_CH3 0x04
#define OSCIL_CH4 0x08

#define TRIGGER_TYPE_RISING   0
#define TRIGGER_TYPE_FALLING  1

#define GENERATOR_FREQUENCY 17  // 16 Hz - 250 kHz

struct _config_struct {
	uint8_t channel_mask;            //  0 0 0 0 CH4 CH3 CH2 CH1
	uint8_t	resolution;              //  0 (500ns) ... 15 (50ms)
	uint8_t trigger_channel;         //  1..4
	uint8_t trigger_level;           //  0..255 
	uint8_t trigger_position;        //  0..10
	uint8_t trigger_type;            //  rising/falling 
};

#endif
