/**
  ******************************************************************************
  * @file    GPIO/IOToggle/stm32f10x_it.c
  * @author  MCD Application Team
  * @version V3.1.2
  * @date    09/28/2009
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
bool 		 gbf_hse_setup_fail    = FALSE;		// �ⲿ16M���������־

/* uart global variables */
uart_cmd_typedef uart_rx_buf,uart_tx_buf;
static uint32_t  uart_rx_timer = 0;
static uint8_t   uart_rx_fg    = 0;
static uint8_t   uart_rx_s     = UartHEADER;
       uint8_t   uart_tx_s     = 0;
/******************************************************************************
  Function:uart_clear_message
  Description:
		���Message�е���Ϣ
  Input :
		Message: Э��Message��ָ��
  Output:
  Return:
  Others:None
******************************************************************************/
void uart_clear_message( uart_cmd_typedef *Message )
{
	uint8_t i;
	uint8_t *pdata = (uint8_t*)(Message);

	for(i=0;i<PACKETSIZE;i++)
	{
		*pdata = 0;
		pdata++;
	}
}

/******************************************************************************
  Function:uart_revice_data_state_mechine
  Description:
		�������ݽ��պ�������ȡ��Ч���ݴ��뻺��
  Input:None
  Output:
  Return:
  Others:None
******************************************************************************/
void uart_revice_data_state_mechine( uint8_t data )
{
	static uint8_t	uart_rx_cnt     = 0;
	static uint8_t  sign_cnt   = 0;

	switch(uart_rx_s)
		{
			case UartHEADER:
				{
					//�������ͷΪ0x5C��ʼ���գ����򲻽���
					if(UART_SOF == data)
					{
						uart_rx_buf.HEADER = data;
						uart_rx_s =  UartTYPE;
						uart_rx_fg = 1;
					}
				}
				break;

			case UartTYPE:
				{
					uart_rx_buf.TYPE = data;
					uart_rx_s = UartSIGN;
					sign_cnt = 0;
				}
				break;

			case UartSIGN:
				{
					uart_rx_buf.SIGN[sign_cnt++] = data;
					if( sign_cnt == 4 )
					{
						uart_rx_s = UartLEN;
					}
				}
				break;

			case UartLEN:
				{
					uart_rx_buf.LEN = data;

					/*  �����ݳ��ȴ��� 236 */
					if(uart_rx_buf.LEN > UART_NBUF)
					{
						uart_rx_s =  UartHEADER;
						/* ��� uart_rx_buf ������Ϣ */
						uart_clear_message(&uart_rx_buf);
						uart_rx_fg = 0;
					}
					else if(uart_rx_buf.LEN > 0)	//  DATA��Ϊ��
					{
						uart_rx_s = UartDATA;
						uart_rx_cnt = 0;
					}
					else//  DATAΪ��
					{
						uart_rx_s = UartXOR;
					}
				}
				break;

			case UartDATA:
				{
						uart_rx_buf.DATA[uart_rx_cnt++] = data;
						/* ���ݽ������ */
						if(uart_rx_cnt == uart_rx_buf.LEN)
								uart_rx_s = UartXOR;
				}
				break;

			case UartXOR:
				{
						uart_rx_buf.XOR = data;
						uart_rx_s = UartEND;
				}
				break;

			case UartEND:
				{
					if(UART_EOF == data)
					{
						uint8_t UartMessageXor = XOR_Cal((uint8_t *)(&uart_rx_buf.TYPE),
												uart_rx_buf.LEN + 6 );
						uart_rx_buf.END = data;

						if( uart_rx_buf.XOR == UartMessageXor)
						{
							/* ��У��ͨ�������������OK���� */
							if(BUFFERFULL != buffer_get_buffer_status(REVICE_RINGBUFFER))
							{
								serial_ringbuffer_write_data(REVICE_RINGBUFFER,&uart_rx_buf);
							}
							uart_rx_fg = 0;
							uart_rx_s = UartHEADER;
							uart_clear_message(&uart_rx_buf);
						}
						else
						{
							uart_clear_message(&uart_rx_buf);
						}
					}
					else
					{
						uart_rx_s = UartHEADER;
						uart_clear_message(&uart_rx_buf);
						uart_rx_fg = 0;
					}
				}
				break;

			default:
				break;
		}
}

/******************************************************************************
  Function:uart_send_data_state_machine
  Description:
		���������ͺ������ӻ�������ȡ���ݷ��͵���λ��
  Input :
		status: uart tx status
  Output:
  Return:
  Others:None
******************************************************************************/
void uart_send_data_state_machine( void )
{
	static uint8_t uart_tx_cnt = 0;
	static uint8_t *pdata;

	switch( uart_tx_s )
	{
		case 0:
			{
					if(BUFFEREMPTY == buffer_get_buffer_status(SEND_RINGBUFFER))
					{
						USART_ITConfig(USART1pos,USART_IT_TXE,DISABLE);
						return;
					}
					else
					{
						serial_ringbuffer_read_data(SEND_RINGBUFFER, &uart_tx_buf);
						pdata = (uint8_t *)(&uart_tx_buf);
						uart_tx_s = 1;
						uart_tx_cnt = *(pdata+6) + 7;
						uart_tx_s = 1;
					}
			}
			break;

		case 1:
			{
				USART_SendData(USART1pos,*pdata);
				uart_tx_cnt--;
				pdata++;
				if( uart_tx_cnt == 0 )
				{
					pdata = &(uart_tx_buf.XOR);
					uart_tx_cnt = 2;
					uart_tx_s = 2;
				}
			}
			break;

		case 2:
			{
				USART_SendData(USART1pos,*pdata);
				uart_tx_cnt--;
				pdata++;
				if( uart_tx_cnt == 0 )
				{
					uart_clear_message(&uart_tx_buf);
					uart_tx_s = 0;
				}
			}
			break;

		default:
			break;
	}
}

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/
#if defined(__CC_ARM) /*------------------RealView Compiler -----------------*/
__asm void GenerateSystemReset(void)
{
//	MOV R0, #1         	//;
//	MSR FAULTMASK, R0  	//; ���FAULTMASK ��ֹһ���жϲ���
//	LDR R0, =0xE000ED0C //;
//	LDR R1, =0x05Fa0004 //;
//	STR R1, [R0]       	//; ϵͳ�����λ
//deadloop
//    B deadloop        //; ��ѭ��ʹ�������в�������Ĵ���
}
#elif (defined(__ICCARM__)) /*------------------ ICC Compiler -------------------*/
//#pragma diag_suppress=Pe940
void GenerateSystemReset(void)
{
	__ASM("MOV R0, #1");
	__ASM("MSR FAULTMASK, R0");
	SCB->AIRCR = 0x05FA0004;
	for (;;);
}
#endif

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
	while(1)
	{
		GenerateSystemReset();
	}
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
		GenerateSystemReset();
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
		GenerateSystemReset();
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
		GenerateSystemReset();
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSV_Handler exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	Timer_list_handler();

	TimingDelay_Decrement();

	if(uart_rx_fg)												//���ڽ��ճ�ʱ������
	{
		uart_rx_timer++;
		if(uart_rx_timer>5)										//5ms��ʱ�����¿�ʼ����
		{
			uart_clear_message(&uart_rx_buf);
			uart_rx_fg = 0;
			uart_rx_s = UartHEADER;
		}
	}
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
/**
  * @brief  This function handles External lines 15 to 10 interrupt request.
  * @param  None
  * @retval None
  */
void USART1pos_IRQHandler(void)
{
	uint8_t uart_temp = 0;

	if(USART_GetITStatus(USART1pos, USART_IT_PE) != RESET)
	{
		// Enable SC_USART RXNE Interrupt (until receiving the corrupted byte)
		USART_ITConfig(USART1pos, USART_IT_RXNE, ENABLE);
		// Flush the SC_USART DR register
		USART_ReceiveData(USART1pos);
	}

	if(USART_GetITStatus(USART1pos, USART_IT_RXNE) != RESET)
	{
	  uart_temp = USART_ReceiveData(USART1pos);

		/* store it to uart_rx_buf */
		uart_revice_data_state_mechine( uart_temp );

		uart_rx_timer = 0;
	}


	if(USART_GetITStatus(USART1pos, USART_IT_TXE) != RESET)
  {
    uart_send_data_state_machine( );
	}
}

/**
  * @}
  */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
