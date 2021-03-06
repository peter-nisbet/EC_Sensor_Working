/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "fsl_uart_freertos.h"
#include "fsl_uart.h"

#include "pin_mux.h"
#include "clock_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
//#define DEMO_UART UART1
#define DEMO_UART UART0		//Changed from UART1 to UART0	//EC
#define PH_UART UART2		//Changed from UART1 to UART0	//pH
#define DEMO_UART_CLKSRC SYS_CLK
#define DEMO_UART_CLK_FREQ CLOCK_GetFreq(SYS_CLK)
#define PH_UART_CLK_FREQ CLOCK_GetFreq(BUS_CLK)
//#define DEMO_UART_RX_TX_IRQn UART1_RX_TX_IRQn
#define DEMO_UART_RX_TX_IRQn UART0_RX_TX_IRQn	//Changed from UART1 IRQ to UART0 IRQ	//EC
#define PH_UART_RX_TX_IRQn UART2_RX_TX_IRQn	//Changed from UART1 IRQ to UART2 IRQ	//pH
/* Task priorities. */
#define uart_task_PRIORITY (configMAX_PRIORITIES - 1)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void uart_task(void *pvParameters);

/*******************************************************************************
 * Variables
 ******************************************************************************/
//const char *to_send = "FreeRTOS UART driver example!\r\n";
/*const char *send_ring_overrun = "\r\nRing buffer overrun!\r\n";
const char *send_hardware_overrun = "\r\nHardware buffer overrun!\r\n";*/

//******EC UART******//
uint8_t background_buffer[40];
uint8_t recv_buffer[1];

uart_rtos_handle_t handle;
struct _uart_handle t_handle;

uart_rtos_config_t uart_config = {
    //.baudrate = 115200,
	.baudrate = 9600,
    .parity = kUART_ParityDisabled,
    .stopbits = kUART_OneStopBit,
    .buffer = background_buffer,
    .buffer_size = sizeof(background_buffer),
};

//******PH UART******//
uint8_t background_pHbuffer[40];
uint8_t recv_pHbuffer[1];

uart_rtos_handle_t pHhandle;
struct _uart_handle t_pHhandle;

uart_rtos_config_t pHuart_config = {
    //.baudrate = 115200,
	.baudrate = 9600,
    .parity = kUART_ParityDisabled,
    .stopbits = kUART_OneStopBit,
    .buffer = background_pHbuffer,
    .buffer_size = sizeof(background_pHbuffer),
};


//Added by Peter//
char inputstring[30];
char inputstringpH[30];
bool endofDataFlag = 0;
char* EC;
char* TDS;
char* SAL;
char* GRAV;
int i = 0;

void pars_data(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    NVIC_SetPriority(DEMO_UART_RX_TX_IRQn, 8);
    NVIC_SetPriority(PH_UART_RX_TX_IRQn, 8);

    xTaskCreate(uart_task, "Uart_task", /*configMINIMAL_STACK_SIZE + 30*/ 1000, NULL, uart_task_PRIORITY, NULL);

    vTaskStartScheduler();
    for (;;)
        ;
}

/*!
 * @brief Task responsible for printing of "Hello world." message.
 */
/*static void uart_task(void *pvParameters)
{
    int error;
    size_t n;

    uart_config.srcclk = DEMO_UART_CLK_FREQ;
    uart_config.base = DEMO_UART;

    if (0 > UART_RTOS_Init(&handle, &t_handle, &uart_config))
    {
        vTaskSuspend(NULL);
    }

    /* Send some data */
   /* if (0 > UART_RTOS_Send(&handle, (uint8_t *)to_send, strlen(to_send)))
    {
        vTaskSuspend(NULL);
    }

    /* Send data */
  /*  do
    {
        error = UART_RTOS_Receive(&handle, recv_buffer, sizeof(recv_buffer), &n);
        if (error == kStatus_UART_RxHardwareOverrun)
        {
            /* Notify about hardware buffer overrun */
         /*   if (kStatus_Success !=
                UART_RTOS_Send(&handle, (uint8_t *)send_hardware_overrun, strlen(send_hardware_overrun)))
            {
                vTaskSuspend(NULL);
            }
        }
        if (error == kStatus_UART_RxRingBufferOverrun)
        {
            /* Notify about ring buffer overrun */
        /*    if (kStatus_Success != UART_RTOS_Send(&handle, (uint8_t *)send_ring_overrun, strlen(send_ring_overrun)))
            {
                vTaskSuspend(NULL);
            }
        }
        if (n > 0)
        {
            /* send back the received data */
     /*       UART_RTOS_Send(&handle, (uint8_t *)recv_buffer, n);
        }
        vTaskDelay(1000);
    } while (kStatus_Success == error);

    UART_RTOS_Deinit(&handle);

    vTaskSuspend(NULL);
}*/

/*static void uart_task(void *pvParameters)
{
	int error;
	size_t n;

	uart_config.srcclk = DEMO_UART_CLK_FREQ;
	uart_config.base = DEMO_UART;

    if (0 > UART_RTOS_Init(&handle, &t_handle, &uart_config))
    {
        vTaskSuspend(NULL);
    }

    do
    {
    	error = UART_RTOS_Receive(&handle, recv_buffer, sizeof(recv_buffer), &n);
    	PRINTF("Value of recv_buffer= ");
    	PRINTF(recv_buffer);
    	PRINTF("\n");
    } while (kStatus_Success == error);

    UART_RTOS_Deinit(&handle);

    vTaskSuspend(NULL);
}*/

static void uart_task(void *pvParameters)
{
	int error;
	size_t n;
	bool SensorToggle = 0;

	uart_config.srcclk = DEMO_UART_CLK_FREQ;
	uart_config.base = DEMO_UART;

    if (0 > UART_RTOS_Init(&handle, &t_handle, &uart_config))
    {
        vTaskSuspend(NULL);
    }

	pHuart_config.srcclk = PH_UART_CLK_FREQ;
	pHuart_config.base = PH_UART;

    if (0 > UART_RTOS_Init(&pHhandle, &t_pHhandle, &pHuart_config))
    {
        vTaskSuspend(NULL);
    }

    do
    {
    	if (SensorToggle == 1){

    		error = UART_RTOS_Receive(&pHhandle, recv_pHbuffer, sizeof(recv_pHbuffer), &n);

    		inputstringpH[i]=recv_pHbuffer[0];
    		i++;
    		if(i>29)
    		{
    			i=0;
    		}
    		if(strncmp(recv_pHbuffer, "\r", 1) == 0)
    		{
    			endofDataFlag = 1;
    			i=0;
    		}
    		if(endofDataFlag == 1)
    		{
    			endofDataFlag=0;
    			if((inputstringpH[0]>=48)&&(inputstringpH[0]<=57))
    			{
    				PRINTF("pH: %s\n", inputstringpH);
    				//pars_data();
    				SensorToggle = 0;
    			}
    			else
    			{
    				PRINTF("Data Received Not pH value! \n");
    			}
    		}
    		memset(recv_pHbuffer, 0, sizeof(recv_pHbuffer));
    	}

    	else if (SensorToggle == 0){

    		error = UART_RTOS_Receive(&handle, recv_buffer, sizeof(recv_buffer), &n);

    		inputstring[i]=recv_buffer[0];
    		i++;
    		if(i>29)
    		{
    			i=0;
    		}
    		if(strncmp(recv_buffer, "\r", 1) == 0)
    		{
    			endofDataFlag = 1;
    			i=0;
    		}
    		if(endofDataFlag == 1)
    		{
    			endofDataFlag=0;
    			if((inputstring[0]>=48)&&(inputstring[0]<=57))
    			{
    				pars_data();
    				SensorToggle = 1;
    			}
    			else
    			{
    				PRINTF("Data Received Not EC value! \n");
    			}
    		}
    		memset(recv_buffer, 0, sizeof(recv_buffer));
    	}

   	} while (kStatus_Success == error);

    UART_RTOS_Deinit(&handle);
    UART_RTOS_Deinit(&pHhandle);

    vTaskSuspend(NULL);
}

void pars_data(void)
{
	EC = strtok(inputstring, ",");
	TDS = strtok(NULL, ",");
	SAL = strtok(NULL, ",");
	GRAV = strtok(NULL, ",");

	PRINTF("EC: ");
	PRINTF(EC);
	PRINTF("\n");
}
