#include "main.h"
#include "stm32h7xx_hal.h"
#include "daisy_seed.h"
#include "per/adc.h"
#include "stm32h7xx_hal_adc.h"
#include "adctemp.h"

using namespace daisy;
using namespace daisy::seed;
using namespace std;

DaisySeed		hw;
ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc3;

volatile uint16_t vtempbuff[10] = {0};
uint8_t flag = 0;

int main(){
	hw.Init(false);

	ADC_ChannelConfTypeDef cfg = {0};

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

	cfg.Channel = ADC_CHANNEL_VREFINT;
	cfg.Rank = ADC_REGULAR_RANK_1;
	cfg.SamplingTime = ADC_SAMPLETIME_387CYCLES_5;
	cfg.SingleDiff = ADC_SINGLE_ENDED;
	cfg.OffsetNumber = ADC_OFFSET_NONE;
	cfg.Offset = 0;
	cfg.OffsetSignedSaturation = DISABLE;
	HAL_ADC_ConfigChannel(&hadc3, &cfg);

	cfg.Channel = ADC_CHANNEL_TEMPSENSOR;
	cfg.Rank = ADC_REGULAR_RANK_2;
	HAL_ADC_ConfigChannel(&hadc3, &cfg);

	

	vtempbuff[0] = 1;
	vtempbuff[1] = 2;

	hw.StartLog(true);
	hw.PrintLine("STARTING ADC TEMP TEST");
	

	bool led = 0;
	while (1)
	{
		//if(flag){

			HAL_ADC_Start(&hadc3);
			vtempbuff[0] = HAL_ADC_GetValue(&hadc3);
			HAL_ADC_Start(&hadc3);
			vtempbuff[1] = HAL_ADC_GetValue(&hadc3);
			uint32_t vrefint = __HAL_ADC_CALC_VREFANALOG_VOLTAGE(vtempbuff[0], ADC_RESOLUTION_16B);
			uint32_t temp = __HAL_ADC_CALC_TEMPERATURE(vrefint, vtempbuff[1], ADC_RESOLUTION_16B);

			hw.PrintLine("%u %u", vrefint, temp);
			hw.SetLed(led ^= 1);
			hw.DelayMs(10);
		//}
	}
  
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
    //if(adcHandle->Instance == ADC3)
    //{
	
		__HAL_RCC_ADC3_CLK_ENABLE();

		HAL_NVIC_SetPriority(ADC3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(ADC3_IRQn);
	/*
		hdma_adc3.Instance                 = DMA2_Stream0;
		hdma_adc3.Init.Request             = DMA_REQUEST_ADC3;
		hdma_adc3.Init.Direction           = DMA_PERIPH_TO_MEMORY;
		hdma_adc3.Init.PeriphInc           = DMA_PINC_DISABLE;
		hdma_adc3.Init.MemInc              = DMA_MINC_ENABLE;
		hdma_adc3.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_adc3.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
		hdma_adc3.Init.Mode = DMA_CIRCULAR;
		hdma_adc3.Init.Priority = DMA_PRIORITY_LOW;
		hdma_adc3.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_adc3);

		__HAL_LINKDMA(&hadc3, DMA_Handle, hdma_adc3);
*/


   // }
}

extern "C"
{
    void DMA2_Stream0_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_adc3);}

	
    void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
    {
		vtempbuff[0] = HAL_ADC_GetValue(&hadc3);
		vtempbuff[1] = HAL_ADC_GetValue(&hadc3);
		flag = 1;
    }
		

		    /*
     * For some reason the flags for injected conversions were getting set.
     * When I went to implement a simple callback to clear them it stopped happening
     * So I guess this doesn't need to be here..
     */
    void HAL_Injected_ConvCpltCallback(ADC_HandleTypeDef* hadc)
    {
#if DEBUG
        asm("bkpt 255");
#endif
    }

    void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc)
    {
        if(hadc->Instance == ADC1)
        {
#if DEBUG
            asm("bkpt 255");
#endif
        }
    }
}