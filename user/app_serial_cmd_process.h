#ifndef	__POS_HANDLE_LAYER_H_
#define	__POS_HANDLE_LAYER_H_
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Private define ------------------------------------------------------------*/

/* Uart to App cmd */
#define APP_CTR_SENT_DATA_VIA_2_4G        0x01 // 0x01: ͨ��2.4G�·����ݰ�����
#define APP_CTR_GET_DATA_VIA_2_4G         0x02 // 0x02: ͨ��2.4G���չ��������ݰ�����
#define APP_CTR_ADD_WHITE_LIST            0x11 // 0x11: ��Ӱ�����
#define APP_CTR_DEL_WHITE_LIST            0x12 // 0x12: ɾ��������
#define APP_CTR_INIT_WHITE_LIST           0x13 // 0x13: ��ʼ��������
#define APP_CTR_OPEN_WHITE_LIST           0x14 // 0x14: ����������
#define APP_CTR_CLOSE_WHITE_LIST          0x15 // 0x15: �رհ�����
#define APP_CTR_OPEN_ATTENDANCE           0x16 // 0x16: ��������
#define APP_CTR_CLOSE_ATTENDANCE          0x17 // 0x17: �رտ���
#define APP_CTR_UPDATE_ATTENDANCE_DATA    0x18 // 0x18: �ϴ�ˢ������
#define APP_CTR_OPEN_PAIR                 0x19 // 0x19: �������
#define APP_CTR_CLOSE_PAIR                0x0C // 0x0c: �ر����
#define APP_CTR_UPATE_PAIR_DATA           0x0D // 0x0d: �ϴ��������
#define APP_CTR_DATALEN_ERR               0xFE // 0xfe: ֡���Ȳ��Ϸ�
#define APP_CTR_UNKNOWN                   0xFF // 0xff: δ��ʶ���֡

#define APP_SERIAL_CMD_STATUS_IDLE        0x00
#define APP_SERIAL_CMD_STATUS_WORK        0x01
#define APP_SERIAL_CMD_STATUS_ERR         0x02
#define APP_SERIAL_CMD_STATUS_IGNORE      0x03
#define APP_SERIAL_CMD_STATUS_WORK_IGNORE 0x04

#define START_SEND_DATA                   0
#define STOP_SEND_DATA                    1

void App_seirial_cmd_process(void);

/* Uart Message configuration */


/* Uart Message frame header and tail */
#define UART_SOF								          (0x5C)							//֡ͷ
#define UART_EOF 								          (0xCA)							//֡β

/* Uart message status */
#define UartOK	 								          (0)									//���ڽ���֡���
#define UartHEADER 							          (1)									//���ڽ���֡֡ͷ
#define UartTYPE 								          (2)									//���ڽ���֡����
#define UartLEN									          (3)									//���ڽ���֡���
#define UartSIGN                          (4)
#define UartDATA 								          (5)									//���ڽ���֡֡β
#define UartXOR									          (6)									//���ڽ���֡���
#define UartEND 								          (7)									//���ڽ���֡֡β

/* Uart Message structure definition */
typedef struct
{
	uint8_t 				HEADER;						  //�жϴ��ڽ���֡ͷ
	uint8_t 				TYPE;								//�жϴ��ڽ��հ�����
	uint8_t         SIGN[4];            //�жϴ��ڽ��ջ��ʶ
	uint8_t 				LEN;								//�жϴ��ڽ������ݳ���
	uint8_t 				DATA[UART_NBUF];		//�жϴ��ڽ�������
	uint8_t 				XOR;								//�жϴ��ڽ������
	uint8_t 				END;								//�жϴ��ڽ���֡β
} uart_cmd_typedef;

typedef struct
{
	uint8_t sign[4];
	uint8_t s_cmd_type;
	uint8_t r_cmd_type;
	uint8_t addr;
	uint8_t wdata[16];
}task_tcb_typedef;

extern task_tcb_typedef card_task;

void beep_ctl_timer_init( void );
#endif // __POS_HANDLE_LAYER_H_
