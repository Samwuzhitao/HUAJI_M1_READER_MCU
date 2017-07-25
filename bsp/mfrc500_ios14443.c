/**
  ******************************************************************************
  * @file   	mfrc500_iso14443.c
  * @author  	Sam.wu
  * @version 	V1.0.0.0
  * @date   	2016.12.29
  * @brief   	hal function for nrf moulde
  ******************************************************************************
  */
 
#include "main.h"
#include "mfrc500.h"

/*******************************************************************************
  * @brief  Ѱ��
  * @param  req_code:Ѱ����ʽ
  *         	//0x52 = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
  *             0x26 = Ѱδ��������״̬�Ŀ�
  * @retval pTagType=��Ƭ���ʹ���
  *             0x4400 = Mifare_UltraLight
  *             0x0400 = Mifare_One(S50)
  * @retval status=����״̬
  * @note 	None	  
*******************************************************************************/
uint8_t PcdRequest(uint8_t req_code,uint8_t *pTagType)
{
	uint8_t status;

	struct TranSciveBuffer {uint8_t MfCommand;
						   uint8_t MfLength;
						   uint8_t MfData[2];
						   }MfComData;
	struct TranSciveBuffer *pi;

	pi=&MfComData;
	PcdSetTmo(106);                         //����RC500��ʱ

	WriteRC(RegChannelRedundancy,0x03);		  //����ÿ���ֽں���У��
	ClearBitMask(RegControl,0x08);					//�ر�Crypto1 ����
	WriteRC(RegBitFraming,0x07);						//���һ���ֽڷ���7λ
		 
	MfComData.MfCommand=PCD_TRANSCEIVE;
	MfComData.MfLength=1;
	MfComData.MfData[0]=req_code;
							 
	status = ReadRC(RegChannelRedundancy);	
	status = ReadRC(RegBitFraming);						   
						   
	status=PcdComTransceive(pi);

	if ((status == MI_OK) && (MfComData.MfLength == 0x10))
	{    
		*pTagType     = MfComData.MfData[0];
		*(pTagType+1) = MfComData.MfData[1];
	}
	else
	{
		status = MI_BITCOUNTERR; 
	}
	return status;
}

/*******************************************************************************
  * @brief  ѡ����Ƭ
  * @param  pSnr[IN]:��Ƭ���кţ�4�ֽ�
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	None	  
*******************************************************************************/
uint8_t PcdSelect1(uint8_t *snr, uint8_t *res)
{
    uint8_t i;
    uint8_t status;
    uint8_t snr_check=0;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[7];
                                }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(106);
    WriteRC(RegChannelRedundancy,0x0F);
    ClearBitMask(RegControl,0x08);

    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength=7;
    MfComData.MfData[0]=PICC_ANTICOLL1;
    MfComData.MfData[1]=0x70;
    for(i=0;i<4;i++)
    {
    	snr_check^=*(snr+i);
    	MfComData.MfData[i+2]=*(snr+i);
    }
    MfComData.MfData[6]=snr_check;
    status=PcdComTransceive(pi);
	
    if(status==MI_OK)
    {   
			memcpy(res, pi->MfData, 10); 		
			if(MfComData.MfLength!=0x8)
      {
				status = MI_BITCOUNTERR;
      }
   }
   return status;
}

/*******************************************************************************
  * @brief  ����ײ
  * @param  Snr[OUT]:�õ��Ŀ�Ƭ���кţ�4�ֽ�
  * @retval status=MI_OK:�ɹ�
  * @note 	Ѱ���ɹ���ͨ���˺������������ڿ�Ƭ���ͷ���ײ����������������м��ſ�
  *         �˺���ֻ�õ�һ�ſ�Ƭ�����кţ�����Pcdselect()����ѡ�����ſ��������к���
  *         ������Դ˿���������Ϻ���PcdHalt()����˿���������״̬����Ѱδ��������
  *         ״̬�Ŀ����ɽ���������Ƭ�Ĳ���None	  
*******************************************************************************/
uint8_t PcdAnticoll(uint8_t antiFlag, uint8_t *snr)
{
    uint8_t i;

    uint8_t snr_check=0;
    uint8_t status=MI_OK;
    struct TranSciveBuffer{		uint8_t MfCommand;
                                uint8_t MfLength;
                                uint8_t MfData[5];
                                }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(106);
    WriteRC(RegDecoderControl,0x28);
    ClearBitMask(RegControl,0x08);
    WriteRC(RegChannelRedundancy,0x03);

    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength=2;
								
	if(PICC_ANTICOLL1 == antiFlag)
		MfComData.MfData[0]=PICC_ANTICOLL1;
	else if(PICC_ANTICOLL2 == antiFlag)
		MfComData.MfData[0]=PICC_ANTICOLL2;
	
    MfComData.MfData[1]=0x20;
    status=PcdComTransceive(pi);
    if(!status)
    {
    	 for(i=0;i<4;i++)
         {
             snr_check^=MfComData.MfData[i];
         }
         if(snr_check!=MfComData.MfData[i])
         {
             status=MI_SERNRERR;
         }
         else
         {
             for(i=0;i<4;i++)
             {
             	*(snr+i)=MfComData.MfData[i];
             }
         }

    }
    ClearBitMask(RegDecoderControl,0x20);
    return status;
}
/*******************************************************************************
  * @brief  ѡ����Ƭ
  * @param  pSnr[IN]:��Ƭ���кţ�4�ֽ�
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	None	  
*******************************************************************************/
uint8_t PcdSelect2(uint8_t *snr, uint8_t *res)
{
    uint8_t i;
    uint8_t status;
    uint8_t snr_check=0;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[7];
                                }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(106);
    WriteRC(RegChannelRedundancy,0x0F);
    ClearBitMask(RegControl,0x08);

    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength=7;
    MfComData.MfData[0]=PICC_ANTICOLL2;
    MfComData.MfData[1]=0x70;
    for(i=0;i<4;i++)
    {
    	snr_check^=*(snr+i);
    	MfComData.MfData[i+2]=*(snr+i);
    }
    MfComData.MfData[6]=snr_check;
    status=PcdComTransceive(pi);

    if(status==MI_OK)
    {  
		  memcpy(res, pi->MfData, 7);
		  if(MfComData.MfLength!=0x8)
      {
			  status = MI_BITCOUNTERR;
      }
   }
   return status;
}

extern C_APDU 							Command;
extern uint8_t 							uM24SRbuffer[];
/*******************************************************************************
  * @brief  
  * @param  
  * @param  
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	
*******************************************************************************/
uint8_t PcdRATS(uint8_t *res, uint8_t *len)
{
    uint8_t status;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[15];
                                }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(106);
    WriteRC(RegChannelRedundancy,0x03);
    ClearBitMask(RegControl,0x08);
								
    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength= 4;
    MfComData.MfData[0] = 0xE0;
	MfComData.MfData[1] = 0x40;
	MfComData.MfData[2] = GETLSB( M24SR_ComputeCrc(MfComData.MfData, 2) );							
    MfComData.MfData[3] = GETMSB( M24SR_ComputeCrc(MfComData.MfData, 2) );
								
    status=PcdComTransceive(pi);
	if(MI_OK == status)
	{
		memcpy(res, pi->MfData, 15); 
		return MI_OK;
	}

   return status;
}

/*******************************************************************************
  * @brief  
  * @param  
  * @param  
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	
*******************************************************************************/
uint8_t PcdPPS(uint8_t *res, uint8_t *len)
{
    uint8_t status;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[15];
                                }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(106);
    WriteRC(RegChannelRedundancy,0x03);
    ClearBitMask(RegControl,0x08);
								
    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength= 5;
    MfComData.MfData[0] = 0xD0;
	MfComData.MfData[1] = 0x11;
	MfComData.MfData[2] = 0x00;
	MfComData.MfData[3] = GETLSB( M24SR_ComputeCrc(MfComData.MfData, 3) );							
    MfComData.MfData[4] = GETMSB( M24SR_ComputeCrc(MfComData.MfData, 3) );
   
    status=PcdComTransceive(pi);
	if(MI_OK == status)
	{
		memcpy(res, pi->MfData, 15); 
		return MI_OK;
	} 

   return status;
}

/*******************************************************************************
  * @brief  
  * @param  
  * @param  
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	
*******************************************************************************/
uint8_t PcdSelectApplication(uint8_t *res, uint8_t *len)
{
    uint8_t status;
	uint8_t ret_len = 0;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[30];
                                }MfComData;
	struct TranSciveBuffer *pi;

	pi=&MfComData;
	PcdSetTmo(106);
	WriteRC(RegChannelRedundancy,0x07);
	ClearBitMask(RegControl,0x08);

	M24SR_InitStructure();
	M24SR_SelectApplication(&ret_len);
								
	MfComData.MfCommand=PCD_TRANSCEIVE;
	MfComData.MfLength= ret_len;
	memcpy(MfComData.MfData, uM24SRbuffer, ret_len);
   
  status=PcdComTransceive(pi);
  if(MI_OK == status)
	{
		memcpy(res, pi->MfData, 30); 
		return MI_OK;
	}

   return status;
}

/*******************************************************************************
  * @brief  
  * @param  
  * @param  
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	
*******************************************************************************/
uint8_t PcdSelectCCfile(uint8_t *res, uint8_t *len)
{
	uint8_t ret_len = 0;
    uint8_t status;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[30];
                                }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(106);
    WriteRC(RegChannelRedundancy,0x07);
    ClearBitMask(RegControl,0x08);

	M24SR_InitStructure();
	M24SR_SelectCCfile(&ret_len);
								
    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength= ret_len;
    memcpy(MfComData.MfData, uM24SRbuffer, ret_len);
   
    status=PcdComTransceive(pi);
    if(MI_OK == status)
	{
		memcpy(res, pi->MfData, 30);  
		return MI_OK;
	}
   return status;
}
/*******************************************************************************
  * @brief  
  * @param  
  * @param  
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	
*******************************************************************************/
uint8_t PcdReadCCfileLength(uint8_t *res, uint8_t *len)
{
	uint8_t ret_len = 0;
    uint8_t status;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[30];
                                }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(106);
    WriteRC(RegChannelRedundancy,0x07);
    ClearBitMask(RegControl,0x08);

	M24SR_InitStructure();
	M24SR_ReadBinary(0x0000, 0x02, &ret_len);
								
    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength= ret_len;
    memcpy(MfComData.MfData, uM24SRbuffer, ret_len);
   
    status=PcdComTransceive(pi);
	if(MI_OK == status)
	{
		memcpy(res, pi->MfData, 30); 
		return MI_OK;
	}
   return status;
}

/*******************************************************************************
  * @brief  
  * @param  
  * @param  
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	
*******************************************************************************/
uint8_t PcdReadCCfile(uint16_t Offset , uint8_t NbByteToRead, uint8_t *res, uint8_t *len)
{
	uint8_t ret_len = 0;
    uint8_t status;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[30];
                                }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(106);
    WriteRC(RegChannelRedundancy,0x07);
    ClearBitMask(RegControl,0x08);

	M24SR_InitStructure();
	M24SR_ReadBinary(Offset, NbByteToRead, &ret_len);
								
    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength= ret_len;
    memcpy(MfComData.MfData, uM24SRbuffer, ret_len);
   
    status=PcdComTransceive(pi);
	if(MI_OK == status)
	{
		memcpy(res, pi->MfData, 30); 
		return MI_OK;
	}
   return status;
}

/*******************************************************************************
  * @brief  
  * @param  
  * @param  
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	
*******************************************************************************/
uint8_t PcdSelectNDEFfile(uint8_t *res, uint8_t *len)
{
	uint8_t ret_len = 0;
    uint8_t status;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[30];
                                }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(106);
    WriteRC(RegChannelRedundancy,0x07);
    ClearBitMask(RegControl,0x08);

	M24SR_InitStructure();
	M24SR_SelectNDEFfile(0x0001, &ret_len);
								
    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength= ret_len;
    memcpy(MfComData.MfData, uM24SRbuffer, ret_len);
   
    status=PcdComTransceive(pi);
	if(MI_OK == status)
	{
		memcpy(res, pi->MfData, 30); 
		return MI_OK;
	}
   return status;
}
/*******************************************************************************
  * @brief  
  * @param  
  * @param  
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	
*******************************************************************************/
uint8_t PcdReadNDEFfileLength(uint8_t *res, uint8_t *len)
{
	uint8_t ret_len = 0;
	uint8_t status;
	struct TranSciveBuffer{uint8_t MfCommand;
															 uint8_t MfLength;
															 uint8_t MfData[30];
															}MfComData;
	struct TranSciveBuffer *pi;

	pi=&MfComData;
	PcdSetTmo(106);
	WriteRC(RegChannelRedundancy,0x07);
	ClearBitMask(RegControl,0x08);

	M24SR_InitStructure();
	M24SR_ReadBinary(0x0000, 0x02, &ret_len);
								
	MfComData.MfCommand=PCD_TRANSCEIVE;
	MfComData.MfLength= ret_len;
	memcpy(MfComData.MfData, uM24SRbuffer, ret_len);
   
	status=PcdComTransceive(pi);
	if(MI_OK == status)
	{
		memcpy(res, pi->MfData, 30);  
		return MI_OK;
	}
   return status;
}

/*******************************************************************************
  * @brief  
  * @param  
  * @param  
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	
*******************************************************************************/
uint8_t PcdReadNDEFfile(uint16_t Offset , uint8_t NbByteToRead, uint8_t *res, uint8_t *len)
{
	uint8_t ret_len = 0;
	uint8_t status;
	struct TranSciveBuffer{uint8_t MfCommand;
															 uint8_t MfLength;
															 uint8_t MfData[BUF_LEN];
															}MfComData;
	struct TranSciveBuffer *pi;

	pi=&MfComData;
	PcdSetTmo(206);
	WriteRC(RegChannelRedundancy,0x07);
	ClearBitMask(RegControl,0x08);

	M24SR_InitStructure();
	M24SR_ReadBinary(Offset, NbByteToRead, &ret_len);
								
	MfComData.MfCommand=PCD_TRANSCEIVE;
	MfComData.MfLength= ret_len;
	memcpy(MfComData.MfData, uM24SRbuffer, ret_len);

  status=PcdComTransceive(pi);

	if(MI_OK == status)
	{
		memcpy(res, pi->MfData, BUF_LEN); 
		return MI_OK;
	}
   return status;
}
/*******************************************************************************
  * @brief  
  * @param  
  * @param  
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	
*******************************************************************************/
uint8_t PcdWriteNDEFfile(uint16_t Offset , uint8_t NbByteToWrite , uint8_t *pDataToWrite, uint8_t *res, uint8_t *len)
{
	uint8_t ret_len = 0;
    uint8_t status;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[BUF_LEN];
                                }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(4);
    WriteRC(RegChannelRedundancy,0x07);
    ClearBitMask(RegControl,0x08);

	M24SR_InitStructure();
	M24SR_UpdateBinary(Offset, NbByteToWrite, pDataToWrite, &ret_len);
								
    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength= ret_len;
    memcpy(MfComData.MfData, uM24SRbuffer, ret_len);
   
    status=PcdComTransceive(pi);
	if(MI_OK == status)
	{
		memcpy(res, pi->MfData, BUF_LEN); 
		return MI_OK;
	}
	return status;
}

/*******************************************************************************
  * @brief  
  * @param  
  * @param  
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	
*******************************************************************************/
uint8_t PcdSelectSystemfile(uint8_t *res, uint8_t *len)
{
	uint8_t ret_len = 0;
    uint8_t status;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[30];
                                }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(106);
    WriteRC(RegChannelRedundancy,0x07);
    ClearBitMask(RegControl,0x08);

	M24SR_InitStructure();
	M24SR_SelectSystemfile(&ret_len);
								
    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength= ret_len;
    memcpy(MfComData.MfData, uM24SRbuffer, ret_len);
   
    status=PcdComTransceive(pi);
	if(MI_OK == status)
	{
		memcpy(res, pi->MfData, 30);
		return MI_OK;
	}
   return status;
}

/*******************************************************************************
  * @brief  
  * @param  
  * @param  
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	
*******************************************************************************/
uint8_t PcdReadSystemfile(uint16_t Offset , uint8_t NbByteToRead, uint8_t *res, uint8_t *len)
{
	uint8_t ret_len = 0;
    uint8_t status;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[30];
                                }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(106);
    WriteRC(RegChannelRedundancy,0x07);
    ClearBitMask(RegControl,0x08);

	M24SR_InitStructure();
	M24SR_ReadBinary(Offset, NbByteToRead, &ret_len);
								
    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength= ret_len;
    memcpy(MfComData.MfData, uM24SRbuffer, ret_len);
   
    status=PcdComTransceive(pi);
	if(MI_OK == status)
	{
		memcpy(res, pi->MfData, 30);
		return MI_OK;
	}
   return status;
}

/*******************************************************************************
  * @brief  
  * @param  
  * @param  
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	
*******************************************************************************/
uint8_t PcdSendInterrupt (uint8_t *res, uint8_t *len)
{
	uint8_t ret_len = 0;
    uint8_t status;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[30];
                                }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(106);
    WriteRC(RegChannelRedundancy,0x07);
    ClearBitMask(RegControl,0x08);

	M24SR_InitStructure();
	M24SR_SendInterrupt(&ret_len);
								
    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength= ret_len;
    memcpy(MfComData.MfData, uM24SRbuffer, ret_len);
   
    status=PcdComTransceive(pi);
	if(MI_OK == status)
	{
		memcpy(res, pi->MfData, 30);
		return MI_OK;
	}
   return status;
}

/*******************************************************************************
  * @brief  
  * @param  
  * @param  
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	
*******************************************************************************/
uint8_t PcdDeselect (uint8_t *res, uint8_t *len)
{
    uint8_t status;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[15];
                                }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(106);
    WriteRC(RegChannelRedundancy,0x03);
    ClearBitMask(RegControl,0x08);
								
    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength= 3;
    MfComData.MfData[0] = 0xC2;			
	MfComData.MfData[1] = 0xE0;
	MfComData.MfData[2] = 0xB4;
   
    status=PcdComTransceive(pi);
	if(MI_OK == status)
	{
		memcpy(res, pi->MfData, 15);
		return MI_OK;
	} 

   return status;
}
/*******************************************************************************
  * @brief  ��λMifare_Pro(X)
  * @param  param[IN]:FSDI+CID��ISO14443-4
  * @param  pLen[OUT]:��λ��Ϣ�ֽڳ���
  * @param  pData[OUT]:��λ��Ϣ
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	None	  
*******************************************************************************/
uint8_t MifareProRst(uint8_t param,uint8_t *pLen,uint8_t *pData)
{

  uint8_t status;

  struct TranSciveBuffer{uint8_t MfCommand;
															 uint8_t MfLength;
															 uint8_t MfData[2];
															}MfComData;
	struct TranSciveBuffer *pi;
	pi=&MfComData;
	
	PcdSetTmo(15);
  
	MfComData.MfCommand = PCD_TRANSCEIVE;
	MfComData.MfLength = 2;
	MfComData.MfData[0] = PICC_RESET;
	MfComData.MfData[1] = param;

	status=PcdComTransceive(pi);
    
	if ((*pLen = MfComData.MfLength/8) <=1 )  
		memcpy(pData, MfComData.MfData, 2);     
	else  
		status = MI_COM_ERR;

	return status;
}

/*******************************************************************************
  * @brief  �����������״̬
  * @param  None
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	None	  
*******************************************************************************/
uint8_t PcdHalt(void)
{
  uint8_t status=MI_OK;

	struct TranSciveBuffer{uint8_t MfCommand;
															 uint8_t MfLength;
															 uint8_t MfData[2];
															}MfComData;
	struct TranSciveBuffer *pi;
	pi=&MfComData;

	PcdSetTmo(106);
	MfComData.MfCommand=PCD_TRANSCEIVE;
	MfComData.MfLength=2;
	MfComData.MfData[0]=PICC_HALT;
	MfComData.MfData[1]=0;

	status=PcdComTransceive(pi);
	if (status)
	{
			if(status==MI_NOTAGERR||status==MI_ACCESSTIMEOUT)
			status = MI_OK;
	}
	WriteRC(RegCommand,PCD_IDLE);
	return status;
}

/*******************************************************************************
  * @brief  ��Mifare_One����Կת��ΪRC500���ո�ʽ
  * @param  �uncoded[IN]:6�ֽ�δת������Կ
  *           coded[OUT]:12�ֽ�ת�������Կ
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	None	  
*******************************************************************************/
uint8_t ChangeCodeKey(uint8_t *uncoded,uint8_t *coded)
{
   uint8_t cnt=0;
   uint8_t ln=0;
   uint8_t hn=0;

   for(cnt=0;cnt<6;cnt++)
   {
      ln=uncoded[cnt]&0x0F;
      hn=uncoded[cnt]>>4;
      coded[cnt*2+1]=(~ln<<4)|ln;
      coded[cnt*2]=(~hn<<4)|hn;
   }
   return MI_OK;
}

/*******************************************************************************
  * @brief  ����ת����ʽ�����Կ�͵�RC500��FIFO��
  * @param  ��pKey[IN]:��Կ
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	None	  
*******************************************************************************/
uint8_t PcdAuthKey(uint8_t *keys)
{
    uint8_t status;
    uint8_t i;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[12];
                                }MfComData;
    struct TranSciveBuffer *pi;
    pi=&MfComData;
    PcdSetTmo(106);
    MfComData.MfCommand=PCD_LOADKEY;
    MfComData.MfLength=12;
    for(i=0;i<12;i++)
    {
        MfComData.MfData[i]=*(keys+i);
    }
    status=PcdComTransceive(pi);
    return status;
}
 
/*******************************************************************************
  * @brief  ����֤��Ƭ����
  * @param  : auth_mode[IN]: ������֤ģʽ
  *                  0x60 = ��֤A��Կ
  *                  0x61 = ��֤B��Կ 
  *          addr[IN]�����ַ
  *          pSnr[IN]����Ƭ���кţ�4�ֽ�
  * @retval status=����״̬,�ɹ�����MI_OK
  * @note 	None	  
*******************************************************************************/
uint8_t PcdAuthState(uint8_t auth_mode,uint8_t block,uint8_t *snr)
{
    uint8_t status=MI_OK;
    uint8_t i;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[6];
                                }MfComData;
    struct TranSciveBuffer *pi;
    pi=&MfComData;

    WriteRC(RegChannelRedundancy,0x07);
    if(status==MI_OK)
    {
        PcdSetTmo(106);
        MfComData.MfCommand=PCD_AUTHENT1;
        MfComData.MfLength=6;
        MfComData.MfData[0]=auth_mode;
        MfComData.MfData[1]=block;
        for(i=0;i<4;i++)
        {
	      MfComData.MfData[i+2]=*(snr+i);
        }
        if((status=PcdComTransceive(pi))==MI_OK)
        {
            if (ReadRC(RegSecondaryStatus)&0x07) 
            {
                status = MI_BITCOUNTERR;
            }
            else
            {
                MfComData.MfCommand=PCD_AUTHENT2;
                 MfComData.MfLength=0;
                if((status=PcdComTransceive(pi))==MI_OK)
                {
                    if(ReadRC(RegControl)&0x08)
                        status=MI_OK;
                    else
                        status=MI_AUTHERR;
                }
             }
         }
   }
   return status;
 }

uint8_t PcdRead(uint8_t addr,uint8_t *readdata)
{
    uint8_t status;
    uint8_t i;
    struct TranSciveBuffer
	{
		uint8_t MfCommand;
        uint8_t MfLength;
        uint8_t MfData[16];
    }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(4);
    WriteRC(RegChannelRedundancy,0x0F);
    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength=2;
    MfComData.MfData[0]=PICC_READ;
    MfComData.MfData[1]=addr;

    status=PcdComTransceive(pi);
    if(status==MI_OK)
    {
        if(MfComData.MfLength!=0x80)
        {
            status = MI_BITCOUNTERR;
        }
        else
        {
            for(i=0;i<16;i++)
            {
                *(readdata+i)=MfComData.MfData[i];
            }
        }
    }
    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�д���ݵ�M1��һ��
//����˵��: addr[IN]�����ַ
//          pData[IN]��д������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////   
uint8_t PcdWrite(uint8_t addr,uint8_t *writedata)
{
    uint8_t status;
    uint8_t i;
    struct TranSciveBuffer{uint8_t MfCommand;
                                 uint8_t MfLength;
                                 uint8_t MfData[16];
                                }MfComData;
    struct TranSciveBuffer *pi;

    pi=&MfComData;
    PcdSetTmo(4);
//  WriteRC(RegChannelRedundancy,0x0F); 
    WriteRC(RegChannelRedundancy,0x07);
    MfComData.MfCommand=PCD_TRANSCEIVE;
    MfComData.MfLength=2;
    MfComData.MfData[0]=PICC_WRITE;
    MfComData.MfData[1]=addr;

    status=PcdComTransceive(pi);
    if(status!=MI_NOTAGERR)
    {
        if(MfComData.MfLength!=4)
        {
           status=MI_BITCOUNTERR;
        }
        else
        {
           MfComData.MfData[0]&=0x0f;
           switch(MfComData.MfData[0])
           {
              case 0x00:
                 status=MI_NOTAUTHERR;
                 break;
              case 0x0a:
                 status=MI_OK;
                 break;
              default:
                 status=MI_CODEERR;
                 break;
           }
        }
     }
     if(status==MI_OK)
     {
        PcdSetTmo(3);
        MfComData.MfCommand=PCD_TRANSCEIVE;
        MfComData.MfLength=16;
        for(i=0;i<16;i++)
        {
            MfComData.MfData[i]=*(writedata+i);
        }
        status=PcdComTransceive(pi);
        if(status!=MI_NOTAGERR)
        {
            MfComData.MfData[0]&=0x0f;
            switch(MfComData.MfData[0])
            {
               case 0x00:
                  status=MI_WRITEERR;
                  break;
               case 0x0a:
                  status=MI_OK;
                  break;
               default:
                  status=MI_CODEERR;
                  break;
           }
        }
     }
  return status;
}

/**************************************END OF FILE****************************/
