#ifndef __MACDES_H__
#define __MACDES_H__
#include <stdint.h>
extern uint8_t Ks[];
extern void des(unsigned char *source,unsigned char * dest,unsigned char * inkey, int flg);
void Macshort(unsigned char *sMacKey,unsigned char *pInData,int datalen,unsigned char *initData,unsigned char *mac);

extern void Mac(unsigned char *Key,unsigned char *pInData2,int data2len,unsigned char *initData,unsigned char *mac);
extern void ThreeDes(uint8_t * src, uint8_t * dest, uint8_t * key, int flag);
extern unsigned char RFMasterKey[];
extern void CalcMasterKey(void);
extern void ThreeDes_ECB(uint8_t * src,uint8_t len, uint8_t * dest, int flag);
extern uint8_t K0[];

void CalcSessionKey(uint8_t * buf,uint8_t * key,uint8_t * keyout) ;
/***************************************
Key 16�ֽ���Կ
pInData1 8�ֽ�data1 ����
pInData2 �ֽ�data2len
initData 8�ֽڳ�ʼ������0000000000000000
mac����󷵻ص�macָ��
****************************************/
#endif
