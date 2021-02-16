#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

TaskHandle_t xTaskHandle1 = NULL;
TaskHandle_t xTaskHandle2 = NULL;

void vTask1_Handler(void *params);
void vTask2_Handler(void *params);

static void prvSetupHardware(void);
static void prvSetupUart(void);

void printmsg(char *msg);

#define TRUE 1
#define FALSE 0
#define AVAILABLE TRUE
#define NOT_AVAILABLE FALSE

char usr_msg[250];
uint8_t UART_ACCES_KEY = AVAILABLE;
int main(void)
{
	DWT->CTRL |= (1 << 0);

	RCC_DeInit();

	SystemCoreClockUpdate();

	prvSetupHardware();

	sprintf(usr_msg, "This is Hello World application starting\r\n");
	printmsg(usr_msg);

	SEGGER_SYSVIEW_Conf();
	SEGGER_SYSVIEW_Start();

	xTaskCreate(vTask1_Handler, "Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandle1);
	xTaskCreate(vTask2_Handler, "Task 2", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandle2);

	vTaskStartScheduler();
	for(;;);

}

void vTask1_Handler(void *params)
{
	while(1)
	{
		if(UART_ACCES_KEY == AVAILABLE)
		{
			UART_ACCES_KEY = NOT_AVAILABLE;
			printmsg("Hello-World: From Task-1\r\n");
			UART_ACCES_KEY = AVAILABLE;
			SEGGER_SYSVIEW_Print("Task1 is yielding");
			traceISR_EXIT_TO_SCHEDULER();
			taskYIELD();
		}
	}
}

void vTask2_Handler(void *params)
{
	while(1)
	{
		if(UART_ACCES_KEY == AVAILABLE)
		{
			UART_ACCES_KEY = NOT_AVAILABLE;
			printmsg("Hello-World: From Task-2\r\n");
			UART_ACCES_KEY = AVAILABLE;
			SEGGER_SYSVIEW_Print("Task2 is yielding");
			traceISR_EXIT_TO_SCHEDULER();
			taskYIELD();
		}
	}
}

static void prvSetupUart(void)
{
	GPIO_InitTypeDef GPIO_UART_Pin;
	USART_InitTypeDef UART2_Init;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	memset(&GPIO_UART_Pin, 0, sizeof(GPIO_UART_Pin));

	GPIO_UART_Pin.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_UART_Pin.GPIO_Mode = GPIO_Mode_AF;
	GPIO_UART_Pin.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_UART_Pin);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	memset(&UART2_Init, 0, sizeof(UART2_Init));

	UART2_Init.USART_BaudRate = 115200;
	UART2_Init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	UART2_Init.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	UART2_Init.USART_Parity = USART_Parity_No;
	UART2_Init.USART_StopBits = USART_StopBits_1;
	UART2_Init.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &UART2_Init);

	USART_Cmd(USART2, ENABLE);
}

static void prvSetupHardware(void)
{
	prvSetupUart();
}

void printmsg(char *msg)
{
	for(uint32_t i = 0; i < strlen(usr_msg); i++)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
		USART_SendData(USART2, msg[i]);
	}
}
