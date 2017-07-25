/**
  ******************************************************************************
  * @file   	app_card_process.c
  * @author  	sam.wu
  * @version 	V1.0.0.0
  * @date   	2015.11.05
  * @brief
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_serial_cmd_process.h"
#include "app_card_process.h"
#include "macdes.h"

//#define SHOW_CARD_PROCESS_TIME
/* Private variables ---------------------------------------------------------*/
#ifdef SHOW_CARD_PROCESS_TIME
extern __IO uint32_t     PowerOnTime;
uint32_t StartTime,EndTime;
#endif
static uart_cmd_typedef card_message;
       task_tcb_typedef card_task;
static uint8_t          card_process_status = 0;
static uint8_t          g_uid_len           = 0;
static uint8_t          find_card_ok        = 0;
static uint8_t          re_try_count        = 0; 
       uint8_t          g_cardType[40];	
       uint8_t          respon[BUF_LEN + 20];	
       uint8_t	        uid_len             = 0; // M1卡序列号长度
       uint8_t 	        g_cSNR[10];						   // M1卡序列号
       uint8_t          key_ctl_addr;
		   uint8_t          RData[16]           = {0};
       uint8_t          DefaultKey[6]       = {0xff,0xff,0xff,0xff,0xff,0xff};
       uint8_t          PassWd[6]           = {0};
       uint8_t	        NewKey[16]          = {0};
       uint8_t			    InitKey[16]         = {0xff,0xff,0xff,0xff,0xff,0xff,
												                       0xff,0x07,0x80,0x69,
												                       0xff,0xff,0xff,0xff,0xff,0xff};			 

/******************************************************************************
  Function:rf_set_card_status
  Description:
		修改systick的状态
  Input :
		rf_status: systick的新状态
  Output:
  Return:
  Others:None
******************************************************************************/
void rf_set_card_status( uint8_t new_status, uint8_t mode)
{
  static uint8_t old_status = 0;
	
	if(mode == 0)
	{
		card_process_status = new_status;
		re_try_count ++;
	}

	if(mode == 1)
	{
		card_process_status = new_status;
		re_try_count = 0;
	}

	if ((old_status != new_status) && (re_try_count >10))
	{
		re_try_count = 0;
		card_process_status = 3;
		find_card_ok = 0;
	}
}

/******************************************************************************
  Function:rf_get_card_status
  Description:
		获取systick的状态
  Input :
  Output:systick的新状态
  Return:
  Others:None
******************************************************************************/
uint8_t rf_get_card_status(void)
{
	return card_process_status ;
}

/******************************************************************************
  Function:create_new_key
  Description:
		产生一个新的秘钥
  Input :
  Output:systick的新状态
  Return:
  Others:None
******************************************************************************/
void create_new_key(uint8_t *uid, uint8_t *NewkeyA,uint8_t *NewkeyBuf)
{
	uint8_t IDbuf[8] = {0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff};
	uint8_t Temp[8]  = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
	uint8_t Result[8]= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	memcpy(IDbuf,uid,4);

	des(IDbuf, Result,Temp , 1);

	memcpy(NewkeyA,   Result,  6);	  		
	memcpy(NewkeyBuf, NewkeyA, 6);
	NewkeyBuf[6] = 0xff;
	NewkeyBuf[7] = 0x07;
	NewkeyBuf[8] = 0x80;
	NewkeyBuf[9] = 0x69;
	memcpy(&NewkeyBuf[10],NewkeyA, 6);
}

void debug_show_data(const char *desc, uint8_t addr, uint8_t * databuf)
{
	uint8_t i = 0;
	printf("%10s %02d : ", desc, addr );
	for(i=0;i<16;i++)
	{
		printf(" %02X",databuf[i]);
	}
	printf("\r\n");
}
/******************************************************************************
  Function:App_card_process
  Description:
		App MI Card 轮询处理函数
  Input :
  Return:
  Others:None
******************************************************************************/
void App_card_process(void)
{
	/* 获取当前状态 */
	uint8_t card_current_status = 0;
	card_current_status = rf_get_card_status();

	if( card_current_status == 1 )
	{
		uint8_t status = 0;
		#ifdef SHOW_CARD_PROCESS_TIME
		StartTime = PowerOnTime;
		#endif

		PcdAntennaOn();
		memset(g_cardType, 0, 40);
		/* reqA指令 :请求A卡，返回卡类型，不同类型卡对应不同的UID长度 */
		status = PcdRequest(PICC_REQIDL,g_cardType);

		if( status == MI_OK )
		{
			if( (g_cardType[0] & 0x40) == 0x40)
				g_uid_len = 8;	
			else
				g_uid_len = 4;

			DEBUG_CARD_DEBUG_LOG("U_LEN = %d\r\n",g_uid_len);
		}
		else
			return;
		/* 防碰撞1 */
		status = PcdAnticoll(PICC_ANTICOLL1, g_cSNR);
		DEBUG_CARD_DEBUG_LOG("STATUS:PcdAnticoll = %d \r\n",status);
		#ifdef SHOW_CARD_PROCESS_TIME
		EndTime = PowerOnTime - StartTime;
		printf("UseTime:PcdAnticoll = %d \r\n",EndTime);
		#endif
		if( status != MI_OK )
		{
			return;
		}

		if( g_uid_len == 4 )
			rf_set_card_status(2,0);
		return;
	}

	if( card_current_status == 2 )
	{
		uint8_t status = 0;
		
		sw_unregister_timer(&card_second_find_timer);

		/* 产生新的秘钥 */
		create_new_key(g_cSNR,PassWd,NewKey);
		key_ctl_addr = (card_task.addr / 4 + 1) * 4 - 1;

		/* 认证默认秘钥 */
		if(( card_task.r_cmd_type == 0x10) || ( card_task.r_cmd_type == 0x13 ))
		{
			status = Authentication( key_ctl_addr, DefaultKey );
			DEBUG_CARD_DEBUG_LOG("Authentication status = %d\r\n",status);
			if( status != MI_OK )
			{
				mfrc500_init();
				rf_set_card_status(1,0);
				return;
			}
		}

		if(( card_task.r_cmd_type == 0x12 ) || ( card_task.r_cmd_type == 0x11 ))
		{
			status = Authentication( key_ctl_addr, PassWd);
			DEBUG_CARD_DEBUG_LOG("Authentication status = %d\r\n",status);
			if( status != MI_OK )
			{
				uint8_t i;
				if( card_task.r_cmd_type == 0x12 )
				{
					card_task.r_cmd_type = 0x10;
					for( i = 0; i < 16; i++)
						card_task.wdata[i] = 0x00;
				}
				if( card_task.r_cmd_type == 0x11 )
					card_task.r_cmd_type = 0x13;
				
				mfrc500_init();
				rf_set_card_status(1,0);
				return;
			}
		}

		#ifdef SHOW_CARD_PROCESS_TIME
		EndTime = PowerOnTime - StartTime;
		printf("UseTime:Authentication = %d \r\n",EndTime);
		#endif

		if ( card_task.s_cmd_type != 0x11 )
		{
			/* 写入数据 */
			status = PcdWrite(card_task.addr, card_task.wdata);
		//debug_show_data( "wr_data ",card_task.addr, card_task.wdata );
			if(status != MI_OK)
			{
				mfrc500_init();
				rf_set_card_status(1,0);
				return;
			}
			else
			{
				status = PcdRead(card_task.addr, RData);
			//debug_show_data( "wr_data ",card_task.addr, card_task.wdata );
				if(status != MI_OK)
				{
					mfrc500_init();
					rf_set_card_status(1,0);
					return;
				}
			}

			/* 更新秘钥 */
			if( card_task.r_cmd_type == 0x10 )
				status = PcdWrite(key_ctl_addr, NewKey);
			if( card_task.r_cmd_type == 0x12 )
				status = PcdWrite(key_ctl_addr, InitKey);
		//debug_show_data( "new_key ",key_ctl_addr, card_task.wdata );
			if(status != MI_OK)
			{
				mfrc500_init();
				rf_set_card_status(1,0);
				return;
			}
			else
			{
				status = PcdRead(key_ctl_addr, RData);
			//debug_show_data( "new_key ",key_ctl_addr, RData );
				if(status != MI_OK)
				{
					mfrc500_init();
					rf_set_card_status(1,0);
					return;
				}
			}
			find_card_ok = 1;
			rf_set_card_status(3,0);
			return;
		}
		else
		{
			status = PcdRead(card_task.addr, RData);
		//debug_show_data( "rd_data ",card_task.addr, RData );
			if(status != MI_OK)
			{
				mfrc500_init();
				rf_set_card_status(1,0);
				return;
			}
			find_card_ok = 1;
			rf_set_card_status(3,0);
		}
	}

	if( card_current_status == 3 )
	{
	//printf("r_cmd_type = %02x ",card_task.r_cmd_type);
		if( card_task.s_cmd_type != 0x11 )
		{
	  //printf("result = %d \r\n",find_card_ok);
			card_message.DATA[0] = find_card_ok;
			card_message.LEN     = 1;
		}
		
		if( card_task.s_cmd_type == 0x11 )
		{
		//printf("result = %d \r\n",find_card_ok);
			card_message.DATA[0] = card_task.addr;
			if( find_card_ok == 0 )
			{
				card_message.DATA[1] = find_card_ok;
				card_message.LEN     = 2;
			}
			else
			{
				card_message.LEN     = 17;
				memcpy(card_message.DATA+1,RData,16);
			//debug_show_data( "rd_data ",card_task.addr, RData );
			}
		}

		card_message.HEADER = 0x5C;
		card_message.END    = 0xCA;
		card_message.TYPE   = card_task.s_cmd_type;
		memcpy( card_message.SIGN, card_task.sign, 4 );
		card_message.XOR = XOR_Cal((uint8_t *)(&(card_message.TYPE)), card_message.LEN+6);
		
		if(BUFFERFULL != buffer_get_buffer_status(SEND_RINGBUFFER))
		{
			#ifndef OPEN_CARD_DATA_SHOW 
			serial_ringbuffer_write_data(SEND_RINGBUFFER,&card_message);
			DEBUG_CARD_DATA_LOG("NDEF_DataRead and NDEF_DataWrite Clear!\r\n");
			#endif
		}
		#ifdef SHOW_CARD_PROCESS_TIME
		EndTime = PowerOnTime - StartTime;
		printf("UseTime:SecondFindStart = %d \r\n",EndTime);
		#endif
		Deselect();
		PcdHalt();
		PcdAntennaOff();
		rf_set_card_status(0,0);
	}
}

void app_clear_status(void)
{
	re_try_count = 0;
	find_card_ok = 0;
//printf("Time out!\r\n");
}

void app_card_timer_init( void )
{
	sw_create_timer(&card_second_find_timer, 200, 1, 3, &(card_process_status), app_clear_status);
}
