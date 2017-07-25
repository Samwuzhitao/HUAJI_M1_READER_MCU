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
Key 16字节密钥
pInData1 8字节data1 输入
pInData2 字节data2len
initData 8字节初始化数据0000000000000000
mac计算后返回的mac指针
****************************************/
#endif
