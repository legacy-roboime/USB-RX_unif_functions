/*
 * NOVO:
 * Usará o pino IRQ do nRF24 e interruptions para lidar com a recepção de mensagens
 * RECENTE:
 * Em vez de se basear em 3 funções diferentes, usar apenas um comando de SPI genérico
 * Mantém-se a configuração de SPI divida em duas partes:
 * 		uma parte fica no construtor da classe SPI e faz a inicialização do SCK,MOSI e MISO
 * 		a outra parte fica no construtor da classe NRF, e inicializa o CSN
 * o pino VDD do NRF fica ligado direto no VDD da discovery
 * a função NRF::begin() é chamada dentro do construtor do nRF
 *
 * Author: Renan Pícoli
 */

/* Includes */
#include "main.h"

#include "NRF24.h"
#include "CONFIG.h"
#include "SPI_interface.h"

uint32_t TimingDelay;

//void Delay (uint32_t nTime);

void TimingDelay_Decrement(void);

extern "C" {
 void SysTick_Handler(void);
 void OTG_FS_IRQHandler(void);
 void OTG_FS_WKUP_IRQHandler(void);

 //handlers que podem ser chamados
 void EXTI0_IRQHandler();
 void EXTI1_IRQHandler();
 void EXTI2_IRQHandler();
 void EXTI3_IRQHandler();
 void EXTI4_IRQHandler();
 void EXTI9_5_IRQHandler();
 void EXTI15_10_IRQHandler();
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

void test_nRF();	//testa se a comunicação entre uC e nRF funciona
void print_nRF();	//imprime no terminal todos os registradores do nRF24
void u8_to_binary(uint8_t number,uint8_t* ptr);	//representa number como um binário de 8 algarismos
void u8_to_hex(uint8_t number,uint8_t* hex);	//representa number como um hexa de 2 algarismos
uint8_t dectohex(uint8_t u);//converte um uint8_t entre 0 e 9 para o char que o representa em hexadecimal
//int nbits(int dec);

NRF* radio_ptr;

int main(void)
{
  SysTick_Config(SystemCoreClock/1000000);
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
  radio_ptr=&radio;
  radio.RX_configure();
  radio.start_listen();

  //inicialização do USB
  USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_CDC_cb, &USR_cb);

  Delay_s(20);
  print_nRF();
  STM_EVAL_LEDOn(LED3);
  STM_EVAL_LEDOn(LED4);
  STM_EVAL_LEDOn(LED5);
  STM_EVAL_LEDOn(LED6);

  /* Infinite loop */
  while (1)
  {
	  //TODO testar
	#ifdef USE_AUTOACK
		radio.stop_listen();//in order to prevent the simultaneous usage of the SPI to write ack and read payload
		uint8_t ack[]={'h','e','l','l','o'};
		//radio.FLUSH_TX();
		radio.W_ACK_PAYLOAD(0,ack,5);
		radio.start_listen();//in order to prevent the simultaneous usage of the SPI to write ack and read payload
	#endif
	  /*	  se IRQ=high,led laranja acende e verde fica apagado,
	  além disso, o led vermelho acende se e só se a interrupção estiver ativada
	  se IRQ=low, apaga o led laranja e acende o verde*/
	  if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5)){
		  STM_EVAL_LEDOn(LED3);
		  STM_EVAL_LEDOff(LED4);
		  if(EXTI_GetITStatus(EXTI_Line5)!=RESET)
			  STM_EVAL_LEDOn(LED5);
		  else
			  STM_EVAL_LEDOff(LED5);
	  }
	  else{
		  STM_EVAL_LEDOff(LED3);
		  STM_EVAL_LEDOn(LED4);
	  }
  }
}

uint8_t USB_receive_and_put(NRF* radio_ptr){
	static uint8_t data[]={0,0,0,0,0};
	//passa por aqui

	if(radio_ptr->RECEIVE(data)){
		VCP_send_buffer(data,5);


		//TODO: testar se ainda há pacotes para ler, COMO O MANUAL MANDA
/*		if(radio_ptr->DATA_READY()){
			STM_EVAL_LEDOn(LED5);
		}*/
		return 1;
	}
	else{
		return 0;
	}
}

//testa se a comunicação entre uC e nRF funciona
void test_nRF(NRF* ptr){

	return;
}

//imprime no terminal todos os registradores do nRF24
void print_nRF(){
	radio_ptr->REFRESH();//a partir de agora, ptr aponta para os valores atualizados dos registradores

	VCP_put_char('\n');VCP_put_char('\n');VCP_put_char('\r');

	uint8_t i,j,reg_size=0;
	REGISTER* current_register = &(radio_ptr->CONFIG);
	uint8_t hex_addr[]={'0','0'};
	uint8_t binary_content[]={'0','0','0','0','0','0','0','0'};
	for(i=0x00;i<=0x19;i++){
		//R_REGISTER(current_register->get_address(), current_register->get_size(),current_register->content);
		VCP_put_char('0');VCP_put_char('x');
		u8_to_hex(current_register->get_address(),hex_addr);
		VCP_send_buffer(hex_addr,2);//imprime o endereço
		VCP_put_char('\t');
		u8_to_binary((current_register->content)[0],binary_content);
		reg_size=current_register->get_size();
		VCP_send_buffer(binary_content,8);
		VCP_put_char('\n');VCP_put_char('\r');

		if(reg_size>1){//imprime os demais bytes de um registrador, caso eles tenham mais de 1 byte
			for(j=1;j<reg_size;j++){
				VCP_put_char('\t');
				u8_to_binary((current_register->content)[j],binary_content);
				VCP_send_buffer(binary_content,8);
				VCP_put_char('\n');VCP_put_char('\r');
			}
		}
		
		current_register++;
	}

	VCP_put_char('\n');VCP_put_char('\n');VCP_put_char('\r');

	return;
}

//representa os bits de number em ptr
void u8_to_binary(uint8_t number,uint8_t* ptr){
	for(int i=7;i>=0;i--){
		ptr[i]='0'+(number%2);
		number=number/2;
	}
}

//representa number como um hexa de 2 algarismos
void u8_to_hex(uint8_t number,uint8_t* hex){

	for(int i=1;i>=0;i--){
		hex[i]=dectohex(number%16);
		number = number/16;
	}
	return;
}

//converte um uint8_t entre 0 e 9 para o char que o representa em hexadecimal
uint8_t dectohex(uint8_t u){
	uint8_t c;

	if(u>=0 && u<=9){
		c='0'+u;
	}else{
		if(u>=10 && u<=15){
			c='a'+(u-10);
		}else{
			c=0;
		}
	}

	return c;
}
/*//número mínimo de bits para representar o decimal dec
int nbits(int dec){
	int n=1;
	while((dec/2)>0){
		dec=dec/2;
		n++;
	}
	return n;
}*/

void EXTI0_IRQHandler(){
	USB_receive_and_put(radio_ptr);
	EXTI_ClearITPendingBit(EXTI_Line0);
	STM_EVAL_LEDToggle(LED6);//indicador de sucesso
	return;
}

void EXTI1_IRQHandler(){
	USB_receive_and_put(radio_ptr);
	EXTI_ClearITPendingBit(EXTI_Line1);
	STM_EVAL_LEDToggle(LED6);//indicador de sucesso
	return;
}

void EXTI2_IRQHandler(){
	USB_receive_and_put(radio_ptr);
	EXTI_ClearITPendingBit(EXTI_Line2);
	STM_EVAL_LEDToggle(LED6);//indicador de sucesso
	return;
}

void EXTI3_IRQHandler(){
	USB_receive_and_put(radio_ptr);
	EXTI_ClearITPendingBit(EXTI_Line3);
	STM_EVAL_LEDToggle(LED6);//indicador de sucesso
	return;
}

void EXTI4_IRQHandler(){
	USB_receive_and_put(radio_ptr);
	EXTI_ClearITPendingBit(EXTI_Line4);
	STM_EVAL_LEDToggle(LED6);//indicador de sucesso
	return;
}

void EXTI9_5_IRQHandler(){
	//passa por aqui
	USB_receive_and_put(radio_ptr);
	//NÃO passa por aqui
	//contém 1 na posição correspondente às linhas que têm IT para tratar
	EXTI_ClearITPendingBit(EXTI_Line(radio_ptr->IRQ_Pin()));
	STM_EVAL_LEDToggle(LED6);//indicador de sucesso
	return;
}

void EXTI15_10_IRQHandler(){
	USB_receive_and_put(radio_ptr);
	//contém 1 na posição correspondente às linhas que têm IT para tratar
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
