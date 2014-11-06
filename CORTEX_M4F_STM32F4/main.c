#define USE_STDPERIPH_DRIVER
#include "stm32f4xx.h"
#include "stm32_p103.h"
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>
#include "fio.h"
#include "clib.h"
#include "game.h"
#include "stm32f429i_discovery_lcd.h"

/* _sromfs symbol can be found in main.ld linker script
 * it contains file system structure of test_romfs directory
 */
//extern const unsigned char _sromfs;

//static void setup_hardware();

volatile xSemaphoreHandle serial_tx_wait_sem = NULL;
/* Add for serial input */
volatile xQueueHandle serial_rx_queue = NULL;

/* IRQ handler to handle USART2 interruptss (both transmit and receive
 * interrupts). */
void USART1_IRQHandler()
{
	static signed portBASE_TYPE xHigherPriorityTaskWoken;

	/* If this interrupt is for a transmit... */
	if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET) {
		/* "give" the serial_tx_wait_sem semaphore to notfiy processes
		 * that the buffer has a spot free for the next byte.
		 */
		xSemaphoreGiveFromISR(serial_tx_wait_sem, &xHigherPriorityTaskWoken);

		/* Diables the transmit interrupt. */
		USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
		/* If this interrupt is for a receive... */
	}else if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){
		char msg = USART_ReceiveData(USART1);

		/* If there is an error when queueing the received byte, freeze! */
		if(!xQueueSendToBackFromISR(serial_rx_queue, &msg, &xHigherPriorityTaskWoken))
			while(1);
	}
	else {
		/* Only transmit and receive interrupts should be enabled.
		 * If this is another type of interrupt, freeze.
		 */
		while(1);
	}

	if (xHigherPriorityTaskWoken) {
		taskYIELD();
	}
}

void send_byte(char ch)
{
	/* Wait until the RS232 port can receive another byte (this semaphore
	 * is "given" by the RS232 port interrupt when the buffer has room for
	 * another byte.
	 */
	while (!xSemaphoreTake(serial_tx_wait_sem, portMAX_DELAY));

	/* Send the byte and enable the transmit interrupt (it is disabled by
	 * the interrupt).
	 */
	USART_SendData(USART1, ch);
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

char recv_byte()
{
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	char msg;
	while(!xQueueReceive(serial_rx_queue, &msg, portMAX_DELAY));
	return msg;
}
void
prvInit()
{
     //LCD init
     LCD_Init();
     IOE_Config();
     LTDC_Cmd( ENABLE );
 
     LCD_LayerInit();
     LCD_SetLayer( LCD_FOREGROUND_LAYER );
     LCD_Clear( LCD_COLOR_BLACK );
     LCD_SetTextColor( LCD_COLOR_WHITE );
}



void Game(void *pvParameter){
	init();
	while(1){
		update();
		render();
	}
}

int main()
{
	prvInit();
	init_rs232();
	enable_rs232_interrupts();
	enable_rs232();
	
	fs_init();
	fio_init();
	/* Create the queue used by the serial task.  Messages for write to
	 * the RS232. */
	vSemaphoreCreateBinary(serial_tx_wait_sem);
	/* Add for serial input 
	 * Reference: www.freertos.org/a00116.html */
	serial_rx_queue = xQueueCreate(1, sizeof(char));
	
	xTaskCreate(Game,(signed portCHAR *)"game",512,NULL,tskIDLE_PRIORITY+2,NULL);

	/* Start running the tasks. */
	vTaskStartScheduler();

	return 0;
}

void vApplicationTickHook()
{
}
