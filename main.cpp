#include "main.h"
#include "stm32h7xx_hal.h"
#include "daisy_seed.h"

using namespace daisy;
using namespace daisy::seed;

DaisySeed		hw;
ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc3;

int main(){
	hw.Init(false);

	/*
	Initialize ADC3 (Vtemp and Vregint are mapped only to ADC3), same as ADC1 in per/adc lib for daisy except for
	- NbrOfConversion = 2
	- ContinousConvMode = DISABLE
	- ConversionDataManagement = ADC_CONVERSIONDATA_DR
	*/
    hadc3.Instance                  = ADC3;
    hadc3.Init.ClockPrescaler       = ADC_CLOCK_ASYNC_DIV2;
    hadc3.Init.Resolution           = ADC_RESOLUTION_16B;
    hadc3.Init.ScanConvMode         = ADC_SCAN_ENABLE;
    hadc3.Init.EOCSelection         = ADC_EOC_SEQ_CONV;
    hadc3.Init.LowPowerAutoWait     = DISABLE;
    hadc3.Init.NbrOfConversion      = 2;
    hadc3.Init.ExternalTrigConv     = ADC_SOFTWARE_START;
    hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc3.Init.ContinuousConvMode    = DISABLE;
	hadc3.Init.DiscontinuousConvMode = DISABLE;
	hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
	hadc3.Init.Overrun      = ADC_OVR_DATA_PRESERVED;
    hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
	hadc3.Init.OversamplingMode = DISABLE;
	HAL_ADC_Init(&hadc3);   
	HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);

	/*
	Initialize ADC3 channels, since channels are internals no GPIO config is needed.
	Sampling time is adjusted as STM32H750x datasheet
	*/
	ADC_ChannelConfTypeDef cfg = {0};

	cfg.Channel = ADC_CHANNEL_VREFINT; // VREF INTERNAL CHANNEL
	cfg.Rank = ADC_REGULAR_RANK_1;
	cfg.SamplingTime = ADC_SAMPLETIME_387CYCLES_5;
	cfg.SingleDiff = ADC_SINGLE_ENDED;
	cfg.OffsetNumber = ADC_OFFSET_NONE;
	cfg.Offset = 0;
	cfg.OffsetSignedSaturation = DISABLE;
	HAL_ADC_ConfigChannel(&hadc3, &cfg);

	cfg.Channel = ADC_CHANNEL_TEMPSENSOR; // VTEMP INTERNAL CHANNEL
	cfg.Rank = ADC_REGULAR_RANK_2;
	HAL_ADC_ConfigChannel(&hadc3, &cfg);

	// Serial will wait for user connection
	hw.StartLog(true);
	hw.PrintLine("STARTING INTERNAL TEMP AND VREG READ");
	

	bool led = 0;
	uint16_t vtemp, vrefraw;
	uint16_t temp, vref;
	while (1)
	{
		// this is not the best way but it works
		HAL_ADC_Start(&hadc3);
		vrefraw = HAL_ADC_GetValue(&hadc3);
		HAL_ADC_Start(&hadc3);
		vtemp = HAL_ADC_GetValue(&hadc3);

		// these macros help calculate vref and temperature based on internal calibrated values (rm0433, paragraph 25.4.33)
		vref = __HAL_ADC_CALC_VREFANALOG_VOLTAGE(vrefraw, ADC_RESOLUTION_16B);
		temp = __HAL_ADC_CALC_TEMPERATURE(vref, vtemp, ADC_RESOLUTION_16B);

		hw.PrintLine("Vreg: %.2f, TempJ: %u", (float) vref / 1000.f, temp);
		hw.SetLed(led ^= 1);
		hw.DelayMs(50);
	}
  
}

/*
This function is called by HAL_ADC_Init, for now we use it to enable ADC3 clock 
*/ 
void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
    if(adcHandle->Instance == ADC3)
    {
		__HAL_RCC_ADC3_CLK_ENABLE();
    }
}
