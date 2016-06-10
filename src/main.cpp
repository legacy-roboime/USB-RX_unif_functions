/*
 * NOVO:
 * Usar� o pino IRQ do nRF24 e interruptions para lidar com a recep��o de mensagens
 * RECENTE:
 * Em vez de se basear em 3 fun��es diferentes, usar apenas um comando de SPI gen�rico
 * Mant�m-se a configura��o de SPI divida em duas partes:
 * 		uma parte fica no construtor da classe SPI e faz a inicializa��o do SCK,MOSI e MISO
 * 		a outra parte fica no construtor da classe NRF, e inicializa o CSN
 * o pino VDD do NRF fica ligado direto no VDD da discovery
 * a fun��o NRF::begin() � chamada dentro do construtor do nRF
 *
 * Author: Renan P�coli
 */

/* Includes */
#include "main.h"

#include "own_libraries/NRF24.h"
#include "own_libraries/CONFIG.h"
#include "own_libraries/SPI_interface.h"

uint32_t TimingDelay;

//void Delay (uint32_t nTime);

void TimingDelay_Decrement(void);

extern "C" {
 void SysTick_Handler(void);
 void OTG_FS_IRQHandler(void);
 void OTG_FS_WKUP_IRQHandler(void);

 //handlers que podem ser chamados
 void EXTI0_IRQHandler(NRF* radio_ptr);
 void EXTI1_IRQHandler(NRF* radio_ptr);
 void EXTI2_IRQHandler(NRF* radio_ptr);
 void EXTI3_IRQHandler(NRF* radio_ptr);
 void EXTI4_IRQHandler(NRF* radio_ptr);
 void EXTI9_5_IRQHandler(NRF* radio_ptr);
 void EXTI15_10_IRQHandler(NRF* radio_ptr);
}


__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/

uint8_t USB_receive_and_put(NRF* radio_ptr);

int main(void)
{
  SysTick_Config(SystemCoreClock/1000);
  /**
  *  IMPORTANT NOTE!
  *  The symbol VECT_TAB_SRAM needs to be defined when building the project
  *  if code has been located to RAM and interrupts are used. 
  *  Otherwise the interrupt table located in flash will be used.
  *  See also the <system_*.c> file and how the SystemInit() function updates 
  *  SCB->VTOR register.  
  *  E.g.  SCB->VTOR = 0x20000000;  
  */

  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);

  NRF radio;//inicializa o NRF com os pinos default, deixa em POWER_UP
  radio.RX_configure();

  //inicializa��o do USB
  USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_CDC_cb, &USR_cb);

  /* Infinite loop */
  while (1)
  {
  }
}

uint8_t USB_receive_and_put(NRF* radio_ptr){
	static uint8_t data[]={0,0,0,0,0};
	if(radio_ptr->RECEIVE(data)){
		VCP_send_buffer(data,5);
		return 1;
	}
	else{
		return 0;
	}
}

void EXTI0_IRQHandler(NRF* radio_ptr){
	USB_receive_and_put(radio_ptr);
	EXTI_ClearITPendingBit(EXTI_Line0);
	STM_EVAL_LEDToggle(LED6);//indicador de sucesso
	return;
}

void EXTI1_IRQHandler(NRF* radio_ptr){
	USB_receive_and_put(radio_ptr);
	EXTI_ClearITPendingBit(EXTI_Line1);
	STM_EVAL_LEDToggle(LED6);//indicador de sucesso
	return;
}

void EXTI2_IRQHandler(NRF* radio_ptr){
	USB_receive_and_put(radio_ptr);
	EXTI_ClearITPendingBit(EXTI_Line2);
	STM_EVAL_LEDToggle(LED6);//indicador de sucesso
	return;
}

void EXTI3_IRQHandler(NRF* radio_ptr){
	USB_receive_and_put(radio_ptr);
	EXTI_ClearITPendingBit(EXTI_Line3);
	STM_EVAL_LEDToggle(LED6);//indicador de sucesso
	return;
}

void EXTI4_IRQHandler(NRF* radio_ptr){
	USB_receive_and_put(radio_ptr);
	EXTI_ClearITPendingBit(EXTI_Line4);
	STM_EVAL_LEDToggle(LED6);//indicador de sucesso
	return;
}

void EXTI9_5_IRQHandler(NRF* radio_ptr){
	STM_EVAL_LEDToggle(LED5);
	USB_receive_and_put(radio_ptr);
	//cont�m 1 na posi��o correspondente �s linhas que t�m IT para tratar
	EXTI_ClearITPendingBit(EXTI_Line(radio_ptr->IRQ_Pin()));
	STM_EVAL_LEDToggle(LED6);//indicador de sucesso
	Delay_ms(100);//TODO; remover
	STM_EVAL_LEDToggle(LED5);
	return;
}

void EXTI15_10_IRQHandler(NRF* radio_ptr){
	USB_receive_and_put(radio_ptr);
	//cont�m 1 na posi��o correspondente �s linhas que t�m IT para tratar
	EXTI_ClearITPendingBit(EXTI_Line(radio_ptr->IRQ_Pin()));
	STM_EVAL_LEDToggle(LED6);//indicador de sucesso
	return;
}

/*void Delay( uint32_t nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0); //gustavo
}*/

/*
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  {
    TimingDelay--;
  }
}
extern "C" {
void SysTick_Handler(void)
{
  TimingDelay_Decrement();
}
}
*/

void OTG_FS_IRQHandler(void)
{
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}

void OTG_FS_WKUP_IRQHandler(void)
{
  if(USB_OTG_dev.cfg.low_power)
  {
    *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
    SystemInit();
    USB_OTG_UngateClock(&USB_OTG_dev);
  }
  EXTI_ClearITPendingBit(EXTI_Line18);
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
