/**
*****************************************************************************
**
**  File        : main.c
**
**  Abstract    : main function.
**
**  Functions   : main
**
**  Environment : Atollic TrueSTUDIO(R)
**                STMicroelectronics STM32F4xx Standard Peripherals Library
**
**  Distribution: The file is distributed "as is", without any warranty
**                of any kind.
**
**  (c)Copyright Atollic AB.
**  You may use this file as-is or modify it according to the needs of your
**  project. This file may only be built (assembled or compiled and linked)
**  using the Atollic TrueSTUDIO(R) product. The use of this file together
**  with other tools than Atollic TrueSTUDIO(R) is not permitted.
**
*****************************************************************************
*/

/* Includes */
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "my_spi.h"
#include "NRF24.h"

static __IO uint32_t TimingDelay;

extern "C"{
	void SysTick_Handler(void);
}
void TimingDelay_Decrement(void);
void Delay_ms(uint32_t time_ms);

int main(void)
{
  SysTick_Config(SystemCoreClock/1000);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
  NRF24 radio;
  radio.is_rx=true;
  radio.Config();
  uint8_t buf_out[] = {'o', 'l', 'a', 'p', 'i'};
  radio.WriteAckPayload(buf_out, 5);
  radio.NRF_CE->Set();
  while (1){
    if(radio.DataReady()||(!radio.RxEmpty())){
      radio.NRF_CE->Reset();
      radio.CleanDataReady();
      uint8_t data_in[5];
      radio.ReadPayload(data_in, 5);
      STM_EVAL_LEDToggle(LED6);
      radio.WriteAckPayload(buf_out, 5);
      radio.NRF_CE->Set();
    }
  }
}

extern "C"{
  void SysTick_Handler(void){
    TimingDelay_Decrement();
  }
}

void TimingDelay_Decrement(void){
  if(TimingDelay != 0x00){
    TimingDelay--;
  }
}
void Delay_ms(uint32_t time_ms)
{
  TimingDelay = time_ms;
  while(TimingDelay != 0);
}

/*
 * Callback used by stm32f4_discovery_audio_codec.c.
 * Refer to stm32f4_discovery_audio_codec.h for more info.
 */
extern "C" void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size){
  /* TODO, implement your code here */
  return;
}

/*
 * Callback used by stm324xg_eval_audio_codec.c.
 * Refer to stm324xg_eval_audio_codec.h for more info.
 */
extern "C" uint16_t EVAL_AUDIO_GetSampleCallBack(void){
  /* TODO, implement your code here */
  return -1;
}
