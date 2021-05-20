/* Includes ------------------------------------------------------------------*/
#include "uart.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ring_buffer.h"
#include "core.h"
#include "usart.h"

/* Private definitions -------------------------------------------------------*/

/* Definition for USARTx clock resources */
#define MY_UART                         huart1
#define BUFFER_SIZE                     1024

// UART transmit buffer descriptor
static struct ring_buffer USART_RingBuffer_Tx;
// UART transmit buffer memory pool
static uint8_t RingBufferData_Tx[BUFFER_SIZE];

// UART receive buffer descriptor
static struct ring_buffer USART_RingBuffer_Rx;
// UART receive buffer memory pool
static uint8_t RingBufferData_Rx[BUFFER_SIZE];

static uint8_t tmp_received;

void UART_init(void)
{
    HAL_UART_Receive_IT(&MY_UART, &tmp_received, 1);

    ring_buffer_init(&USART_RingBuffer_Rx, RingBufferData_Rx, BUFFER_SIZE);
    ring_buffer_init(&USART_RingBuffer_Tx, RingBufferData_Tx, BUFFER_SIZE);
}

bool USART_PutChar(uint8_t c){
    CORE_EnterCriticalSection();
    bool temp = ring_buffer_put_val(&USART_RingBuffer_Tx, c);
    CORE_ExitCriticalSection();
  if(temp) {
        __HAL_UART_ENABLE_IT(&MY_UART, UART_IT_TXE);
    }
    return temp;
}


size_t USART_WriteData(const void *data, size_t dataSize){

    assert(data);
    assert(dataSize > 0);

    if((data) && (dataSize > 0))
    {
        size_t i = 0;

        for(i = 0; i < dataSize; i++)
        {
            if(!USART_PutChar(((char*)data)[i])) break;
        }

        return i;
    }

    return 0;
}


size_t USART_WriteString(const char *string){

    return USART_WriteData(string, strlen(string));

}

bool USART_GetChar(char *c){
	
	assert(c);
	
	if(c)
	{		
		if(!ring_buffer_is_empty(&USART_RingBuffer_Rx))
		{
			CORE_EnterCriticalSection();
			ring_buffer_get_val(&USART_RingBuffer_Rx, (uint8_t*)c);
			CORE_ExitCriticalSection();
			return true;
		}
	}
	return false;
}


size_t USART_ReadData(char *data, size_t maxSize){
	
	assert(maxSize > 0);
	
	if(maxSize > 0)
	{
		size_t i = 0;
		for(i = 0; i < maxSize; i++)
		{
			if(!USART_GetChar(data++)) break;
		}
		return i;
	}
	
	return 0;
}

bool uart_check_new_line(void)
{
    return ring_buffer_new_line_check(&USART_RingBuffer_Rx);
}

bool uart_check_esc(void)
{
    return ring_buffer_esc_check(&USART_RingBuffer_Rx);
}

void USART1_IRQHandler(void){

    if (__HAL_UART_GET_FLAG(&MY_UART, UART_FLAG_RXNE)) {
        // the RXNE interrupt has occurred
        if (__HAL_UART_GET_IT_SOURCE(&MY_UART, UART_IT_RXNE)) {
            // the RXNE interrupt is enabled

            ring_buffer_put_val(&USART_RingBuffer_Rx, (uint8_t)MY_UART.Instance->DR);
            USART_PutChar((uint8_t)MY_UART.Instance->DR);
        }
    }

    if (__HAL_UART_GET_FLAG(&MY_UART, UART_FLAG_TXE)) {
      // the TXE interrupt has occurred
    if (__HAL_UART_GET_IT_SOURCE(&MY_UART, UART_IT_TXE)) {
        // the TXE interrupt is enabled

        if(!ring_buffer_get_val(&USART_RingBuffer_Tx, (uint8_t*)&MY_UART.Instance->DR))
        {
         __HAL_UART_DISABLE_IT(&MY_UART, UART_IT_TXE);
        }

      }
  }
}

