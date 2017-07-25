/**
  ******************************************************************************
  * @file   	app_send_data_process.c
  * @author  	sam.wu
  * @version 	V1.0.0.0
  * @date   	2015.11.05
  * @brief
  * @Changelog :
  *    [1].Date   : 2016_8-26
	*        Author : sam.wu
	*        brief  : 尝试分离数据，降低代码的耦合度
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_card_process.h"

/* Private variables ---------------------------------------------------------*/
extern uint8_t uart_tx_s;
static uint8_t cmd_s  = APP_SERIAL_CMD_STATUS_IDLE;
			 uint8_t cmd_t  = 0;
			 uint8_t err_t  = 0;
static uint8_t beep_s = 0;

/* Private functions ---------------------------------------------------------*/
static void serial_s_cmd_process(void);
static void serial_r_cmd_process(void);
static void app_beep_ctl_process( void );

int8_t app_get_device_info( uart_cmd_typedef *rbuf, uart_cmd_typedef *sbuf );
void   app_load_program   ( uart_cmd_typedef *rbuf, uart_cmd_typedef *sbuf );
int8_t app_m1_write_data  ( uart_cmd_typedef *rbuf, uart_cmd_typedef *sbuf );
int8_t app_m1_read_data   ( uart_cmd_typedef *rbuf, uart_cmd_typedef *sbuf );
int8_t app_m1_clear_data  ( uart_cmd_typedef *rbuf, uart_cmd_typedef *sbuf );
int8_t app_beep_ctl       ( uart_cmd_typedef *rbuf, uart_cmd_typedef *sbuf );
void   app_cmd_unknow     ( uart_cmd_typedef *sbuf, uint8_t cmd_type, uint8_t err_type );
/******************************************************************************
  Function:App_seirial_cmd_process
  Description:
		串口进程处理函数
  Input :None
  Return:None
  Others:None
******************************************************************************/
void App_seirial_cmd_process(void)
{
	/* send process data to pc */
	serial_s_cmd_process();

	/* serial cmd process */
	serial_r_cmd_process();
	
	/* ctl beep */
	app_beep_ctl_process();
}

/******************************************************************************
  Function:serial_send_data_to_pc
  Description:
		串口发送指令函数
  Input :None
  Return:None
  Others:None
******************************************************************************/
static void serial_s_cmd_process(void)
{
	if( uart_tx_s == 0)
	{
		if(BUFFEREMPTY == buffer_get_buffer_status(SEND_RINGBUFFER))
		{
			USART_ITConfig(USART1pos,USART_IT_TXE,DISABLE);
			return;
		}
		else
		{
			/* enable interrupt Start send data*/
			USART_ITConfig(USART1pos, USART_IT_TXE, ENABLE);
		}
  }
}

/******************************************************************************
  Function:serial_cmd_process
  Description:
		串口指令处理进程
  Input :None
  Return:None
  Others:None
******************************************************************************/
static void serial_r_cmd_process(void)
{
	static uart_cmd_typedef rdata,sdata;
	uint8_t buf_s = 0;

  /* 系统空闲提取缓存指令 */
	if( cmd_s == APP_SERIAL_CMD_STATUS_IDLE )
	{
		/* 获取接收缓存的状态 */
		buf_s = buffer_get_buffer_status(REVICE_RINGBUFFER);

		/* 根据状态决定是否读取缓存指令 */
		if(BUFFEREMPTY == buf_s)
			return;
		else
		{
			serial_ringbuffer_read_data(REVICE_RINGBUFFER, &rdata);
			cmd_t   = rdata.TYPE;
			cmd_s = APP_SERIAL_CMD_STATUS_WORK;
		}
	}

	/* 系统不空闲解析指令，生产返回信息 */
	if( cmd_s != APP_SERIAL_CMD_STATUS_IDLE )
	{
		/* 解析指令 */
		switch(cmd_t)
		{
			/* 写数据指令 */
			case 0x10:
				cmd_s = app_m1_write_data( &rdata, &sdata );
			break;

			/* 读数据指令 */
			case 0x11:
				cmd_s = app_m1_read_data( &rdata, &sdata );
			break;

			/* 清卡指令 */
			case 0x12:
				cmd_s = app_m1_clear_data( &rdata, &sdata );
			break;

			/* 蜂鸣器操作指令 */
			case 0x30:
				cmd_s = app_beep_ctl( &rdata, &sdata );
				serial_ringbuffer_write_data(SEND_RINGBUFFER,&sdata);
			break;

      /* 查看设备信息指令 */
			case 0x40:
				cmd_s = app_get_device_info( &rdata, &sdata );
				serial_ringbuffer_write_data(SEND_RINGBUFFER,&sdata);
			break;

			/* 升级程序指令 */
			case 0x50:
				app_load_program( &rdata, &sdata );
			break;

			/* 无法识别的指令 */
			case APP_CTR_UNKNOWN:
				app_cmd_unknow(&sdata,err_t,APP_CTR_UNKNOWN);
				serial_ringbuffer_write_data(SEND_RINGBUFFER,&sdata);
				cmd_s = APP_SERIAL_CMD_STATUS_IDLE;
			break;

			/* 无法识别的指令 */
			default:
				err_t = cmd_t;
				cmd_t = 0xff;
				cmd_s = APP_CTR_UNKNOWN;
			break;
		}
	}
}

/******************************************************************************
  Function:app_get_device_info
  Description:
		打印设备信息
  Input :
		rbuf:串口接收指令的消息指针
		sbuf:串口发送指令的消息指针
  Return:
  Others:None
******************************************************************************/
int8_t app_get_device_info( uart_cmd_typedef *rbuf, uart_cmd_typedef *sbuf )
{
	uint8_t index  = 0,i = 0,j=0;
	uint8_t *pdata = (uint8_t *)(sbuf->DATA);

	sbuf->HEADER = 0x5C;
	sbuf->TYPE   = rbuf->TYPE;
	memcpy(sbuf->SIGN, rbuf->SIGN, 4);

	for(index=0,i=0;(index<4)&&(i<8);index++,i=i+2)
		*(pdata + ( j++ ))=(jsq_uid[i]<<4|jsq_uid[i+1]);

	for(index=0;index<3;index++)
		*(pdata + ( j++ ))=software[index];

	for(index=0,i=0;(index<15)&&(i<30);index++,i=i+2)
		*(pdata + ( j++ ))=(hardware[i]<<4)|(hardware[i+1]);

	for(index=0,i=0;(index<8)&&(i<16);index++,i=i+2)
		*(pdata + ( j++ ))=(company[i]<<4)|(company[i+1]);

	sbuf->LEN = j;
	sbuf->XOR = XOR_Cal((uint8_t *)(&(sbuf->TYPE)), j+6);
	sbuf->END = 0xCA;
	
	return 0;
}

/******************************************************************************
  Function:App_setting_24g_attendence
  Description:
		2.4G考勤设置函数
  Input :
		rbuf:串口接收指令的消息指针
		sbuf:串口发送指令的消息指针
  Return:
  Others:None
******************************************************************************/
void app_load_program( uart_cmd_typedef *rbuf, uart_cmd_typedef *sbuf )
{
	typedef  void (*pFunction)(void);
	
	uint32_t JumpAddress;
	pFunction JumpToBootloader;
	/* Jump to user application */
	JumpAddress = *(__IO uint32_t*) (0x8000000 + 4);
	JumpToBootloader = (pFunction) JumpAddress;
	/* Initialize user application's Stack Pointer */
	__set_MSP(*(__IO uint32_t*) 0x8000000);
	JumpToBootloader();
}

/******************************************************************************
  Function:App_returnErr
  Description:
		打印设备信息
  Input :
		rbuf:串口接收指令的消息指针
		sbuf:串口发送指令的消息指针
  Return:
  Others:None
******************************************************************************/
void app_cmd_unknow( uart_cmd_typedef *sbuf, uint8_t cmd_type, uint8_t err_type )
{
	uint8_t i = 0;
	uint8_t *pdata = (uint8_t *)(sbuf->DATA);

	sbuf->HEADER = 0x5C;
	sbuf->TYPE   = 0xE0;
	sbuf->LEN    = 2;
	memset(sbuf->SIGN, 0xFF, 4);

	/* 操作失败 */
	*( pdata + ( i++ ) ) = err_type;
	/* 错误类型 */
	*( pdata + ( i++ ) ) = cmd_type;

	sbuf->XOR = XOR_Cal((uint8_t *)(&(sbuf->TYPE)), i+6);
	sbuf->END = 0xCA;
}

int8_t app_m1_write_data( uart_cmd_typedef *rbuf, uart_cmd_typedef *sbuf )
{
	uint8_t i = 0;
	uint8_t *pdata = (uint8_t *)(rbuf->DATA+1);

	if(( rbuf->LEN == 17 ) && (rbuf->DATA[0] >= 4))
	{
		/* 拷贝指令数据 */
		memcpy(card_task.sign, rbuf->SIGN, 4);
		card_task.addr       = rbuf->DATA[0];
		card_task.s_cmd_type = rbuf->TYPE;
		card_task.r_cmd_type = rbuf->TYPE;

		for(i=0;(i<rbuf->LEN)&&(i < 16); i++)
			card_task.wdata[i] = *(pdata + ( i ));

		/* 启动寻卡状态机 */
		rf_set_card_status(1,1);
		app_card_timer_init();
		return 0;
	}
	else
		return -1;
}

int8_t app_m1_read_data( uart_cmd_typedef *rbuf, uart_cmd_typedef *sbuf )
{
	uint8_t i = 0;

	if(( rbuf->LEN == 1 ) && (rbuf->DATA[0] >= 4))
	{
		/* 拷贝指令数据 */
		memcpy(card_task.sign, rbuf->SIGN, 4);
		card_task.s_cmd_type = rbuf->TYPE;
		card_task.r_cmd_type = rbuf->TYPE;
		card_task.addr       = rbuf->DATA[0];
		
		for( i = 0; i < 16; i++)
			card_task.wdata[i] = 0x00;

		/* 启动寻卡状态机 */
		rf_set_card_status(1,1);
		app_card_timer_init();
		return 0;
	}
	else
		return -1;
}

int8_t app_m1_clear_data( uart_cmd_typedef *rbuf, uart_cmd_typedef *sbuf )
{
	uint8_t i = 0;

	if(( rbuf->LEN == 1 ) && (rbuf->DATA[0] >= 4))
	{
		/* 拷贝指令数据 */
		memcpy(card_task.sign, rbuf->SIGN, 4);
		card_task.s_cmd_type = rbuf->TYPE;
		card_task.r_cmd_type = rbuf->TYPE;
		card_task.addr       = rbuf->DATA[0];
		
		for( i = 0; i < 16; i++)
			card_task.wdata[i] = 0x00;

		/* 启动寻卡状态机 */
		rf_set_card_status(1,1);
		app_card_timer_init();
		return 0;
	}
	else
		return -1;
}

int8_t app_beep_ctl( uart_cmd_typedef *rbuf, uart_cmd_typedef *sbuf )
{
	if( rbuf->LEN == 1 )
	{
		sbuf->HEADER = 0x5C;
		sbuf->TYPE   = rbuf->TYPE;
		memcpy(sbuf->SIGN, rbuf->SIGN, 4);
		sbuf->DATA[0] = 0;
		sbuf->LEN = 1;
		sbuf->XOR = XOR_Cal((uint8_t *)(&(sbuf->TYPE)), 1+6);
		sbuf->END = 0xCA;

		beep_s = 1;
		return 0;
	}
	else
		return -1;
}

/******************************************************************************
  Function:systick_timer_init
  Description:
  Input :
  Return:
  Others:None
******************************************************************************/
void beep_ctl_timer_init( void )
{
	sw_create_timer(&card_buzzer_timer    , 150, 1, 0,&(beep_s), NULL);
}

static void app_beep_ctl_process( void )
{
	if( beep_s == 1 )
	{
		BEEP_EN();
	}
	else 
	{
		BEEP_DISEN();
	}
}
/**************************************END OF FILE****************************/

