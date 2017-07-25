//#include <stm32f10x_lib.h>
#include <stdint.h>
#include "macdes.h"

uint8_t Ks[16]= {0};
//extern uint8_t IDm[14];

//uint8_t IDm[14]= {0x11,0x22,0x33,0x44,0x55,0x66,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E};
//uint8_t testdata[14] = {0x2a,0x36,0xc0,0x48,0x91,0x35,0x50,0x35,0x50,0x25,0x01,0x80,0x29,0x03};
unsigned char RFMasterKey[16] = {0};
void des(uint8_t *source,uint8_t * dest,uint8_t * inkey, int flg)
{
    unsigned char bufout[64],
    kwork[56], worka[48], kn[48], buffer[64], key[64],
    nbrofshift, temp1, temp2;
    int valindex;
    register int i, j, k, iter;
    /* INITIALIZE THE TABLES */
    /* Table - s1 */
    static unsigned char s1[4][16] = {
    14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7,
    0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8,
    4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0,
    15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13 };
    /* Table - s2 */
    static unsigned char s2[4][16] = {
    15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10,
    3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5,
    0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15,
    13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9 };
    /* Table - s3 */
    static unsigned char s3[4][16] = {
    10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8,
    13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1,
    13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7,
    1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12 };
    /* Table - s4 */
    static unsigned char s4[4][16] = {
    7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15,
    13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9,
    10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4,
    3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14 };
    /* Table - s5 */
    static unsigned char s5[4][16] = {
    2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9,
    14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6,
    4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14,
    11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3 };
    /* Table - s6 */
    static unsigned char s6[4][16] = {
    12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11,
    10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8,
    9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6,
    4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13 };
    /* Table - s7 */
    static unsigned char s7[4][16] = {
    4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1,
    13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6,
    1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2,
    6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12 };
    /* Table - s8 */
    static unsigned char s8[4][16] = {
    13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7,
    1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2,
    7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8,
    2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11 };

    /* Table - Shift */
    static unsigned char shift[16] = {
    1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1 };

    /* Table - Binary */
    static unsigned char binary[64] = {
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1,
    0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1,
    1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1,
    1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1 };
    /* MAIN PROCESS */
    /* Convert from 64-bit key into 64-byte key */
    for (i = 0; i < 8; i++) {
    key[8*i] = ((j = *(inkey + i)) / 128) % 2;
    key[8*i+1] = (j / 64) % 2;
    key[8*i+2] = (j / 32) % 2;
    key[8*i+3] = (j / 16) % 2;
    key[8*i+4] = (j / 8) % 2;
    key[8*i+5] = (j / 4) % 2;
    key[8*i+6] = (j / 2) % 2;
    key[8*i+7] = j % 2;
    }
    /* Convert from 64-bit data into 64-byte data */
    for (i = 0; i < 8; i++) {
    buffer[8*i] = ((j = *(source + i)) / 128) % 2;
    buffer[8*i+1] = (j / 64) % 2;
    buffer[8*i+2] = (j / 32) % 2;
    buffer[8*i+3] = (j / 16) % 2;
    buffer[8*i+4] = (j / 8) % 2;
    buffer[8*i+5] = (j / 4) % 2;
    buffer[8*i+6] = (j / 2) % 2;
    buffer[8*i+7] = j % 2;
    }
    /* Initial Permutation of Data */
    bufout[ 0] = buffer[57];
    bufout[ 1] = buffer[49];
    bufout[ 2] = buffer[41];
    bufout[ 3] = buffer[33];
    bufout[ 4] = buffer[25];
    bufout[ 5] = buffer[17];
    bufout[ 6] = buffer[ 9];
    bufout[ 7] = buffer[ 1];
    bufout[ 8] = buffer[59];
    bufout[ 9] = buffer[51];
    bufout[10] = buffer[43];
    bufout[11] = buffer[35];
    bufout[12] = buffer[27];
    bufout[13] = buffer[19];
    bufout[14] = buffer[11];
    bufout[15] = buffer[ 3];
    bufout[16] = buffer[61];
    bufout[17] = buffer[53];
    bufout[18] = buffer[45];
    bufout[19] = buffer[37];
    bufout[20] = buffer[29];
    bufout[21] = buffer[21];
    bufout[22] = buffer[13];
    bufout[23] = buffer[ 5];
    bufout[24] = buffer[63];
    bufout[25] = buffer[55];
    bufout[26] = buffer[47];
    bufout[27] = buffer[39];
    bufout[28] = buffer[31];
    bufout[29] = buffer[23];
    bufout[30] = buffer[15];
    bufout[31] = buffer[ 7];
    bufout[32] = buffer[56];
    bufout[33] = buffer[48];
    bufout[34] = buffer[40];
    bufout[35] = buffer[32];
    bufout[36] = buffer[24];
    bufout[37] = buffer[16];
    bufout[38] = buffer[ 8];
    bufout[39] = buffer[ 0];
    bufout[40] = buffer[58];
    bufout[41] = buffer[50];
    bufout[42] = buffer[42];
    bufout[43] = buffer[34];
    bufout[44] = buffer[26];
    bufout[45] = buffer[18];
    bufout[46] = buffer[10];
    bufout[47] = buffer[ 2];
    bufout[48] = buffer[60];
    bufout[49] = buffer[52];
    bufout[50] = buffer[44];
    bufout[51] = buffer[36];
    bufout[52] = buffer[28];
    bufout[53] = buffer[20];
    bufout[54] = buffer[12];
    bufout[55] = buffer[ 4];
    bufout[56] = buffer[62];
    bufout[57] = buffer[54];
    bufout[58] = buffer[46];
    bufout[59] = buffer[38];
    bufout[60] = buffer[30];
    bufout[61] = buffer[22];
    bufout[62] = buffer[14];
    bufout[63] = buffer[ 6];
    /* Initial Permutation of Key */
    kwork[ 0] = key[56];
    kwork[ 1] = key[48];
    kwork[ 2] = key[40];
    kwork[ 3] = key[32];
    kwork[ 4] = key[24];
    kwork[ 5] = key[16];
    kwork[ 6] = key[ 8];
    kwork[ 7] = key[ 0];
    kwork[ 8] = key[57];
    kwork[ 9] = key[49];
    kwork[10] = key[41];
    kwork[11] = key[33];
    kwork[12] = key[25];
    kwork[13] = key[17];
    kwork[14] = key[ 9];
    kwork[15] = key[ 1];
    kwork[16] = key[58];
    kwork[17] = key[50];
    kwork[18] = key[42];
    kwork[19] = key[34];
    kwork[20] = key[26];
    kwork[21] = key[18];
    kwork[22] = key[10];
    kwork[23] = key[ 2];
    kwork[24] = key[59];
    kwork[25] = key[51];
    kwork[26] = key[43];
    kwork[27] = key[35];
    kwork[28] = key[62];
    kwork[29] = key[54];
    kwork[30] = key[46];
    kwork[31] = key[38];
    kwork[32] = key[30];
    kwork[33] = key[22];
    kwork[34] = key[14];
    kwork[35] = key[ 6];
    kwork[36] = key[61];
    kwork[37] = key[53];
    kwork[38] = key[45];
    kwork[39] = key[37];
    kwork[40] = key[29];
    kwork[41] = key[21];
    kwork[42] = key[13];
    kwork[43] = key[ 5];
    kwork[44] = key[60];
    kwork[45] = key[52];
    kwork[46] = key[44];
    kwork[47] = key[36];
    kwork[48] = key[28];
    kwork[49] = key[20];
    kwork[50] = key[12];
    kwork[51] = key[ 4];
    kwork[52] = key[27];
    kwork[53] = key[19];
    kwork[54] = key[11];
    kwork[55] = key[ 3];
    /* 16 Iterations */
    for (iter = 1; iter < 17; iter++) {
    for (i = 0; i < 32; i++)
    {
        buffer[i] = bufout[32+i];
    }
    /* Calculation of F(R, K) */
    /* Permute - E */
    worka[ 0] = buffer[31];
    worka[ 1] = buffer[ 0];
    worka[ 2] = buffer[ 1];
    worka[ 3] = buffer[ 2];
    worka[ 4] = buffer[ 3];
    worka[ 5] = buffer[ 4];
    worka[ 6] = buffer[ 3];
    worka[ 7] = buffer[ 4];
    worka[ 8] = buffer[ 5];
    worka[ 9] = buffer[ 6];
    worka[10] = buffer[ 7];
    worka[11] = buffer[ 8];
    worka[12] = buffer[ 7];
    worka[13] = buffer[ 8];
    worka[14] = buffer[ 9];
    worka[15] = buffer[10];
    worka[16] = buffer[11];
    worka[17] = buffer[12];
    worka[18] = buffer[11];
    worka[19] = buffer[12];
    worka[20] = buffer[13];
    worka[21] = buffer[14];
    worka[22] = buffer[15];
    worka[23] = buffer[16];
    worka[24] = buffer[15];
    worka[25] = buffer[16];
    worka[26] = buffer[17];
    worka[27] = buffer[18];
    worka[28] = buffer[19];
    worka[29] = buffer[20];
    worka[30] = buffer[19];
    worka[31] = buffer[20];
    worka[32] = buffer[21];
    worka[33] = buffer[22];
    worka[34] = buffer[23];
    worka[35] = buffer[24];
    worka[36] = buffer[23];
    worka[37] = buffer[24];
    worka[38] = buffer[25];
    worka[39] = buffer[26];
    worka[40] = buffer[27];
    worka[41] = buffer[28];
    worka[42] = buffer[27];
    worka[43] = buffer[28];
    worka[44] = buffer[29];
    worka[45] = buffer[30];
    worka[46] = buffer[31];
    worka[47] = buffer[ 0];
    /* KS Function Begin */
    if (flg) {
    nbrofshift = shift[iter-1];
    for (i = 0; i < (int) nbrofshift; i++) {
        temp1 = kwork[0];
        temp2 = kwork[28];
        for (j = 0; j < 27; j++) {
         kwork[j] = kwork[j+1];
         kwork[j+28] = kwork[j+29];
        }
        kwork[27] = temp1;
        kwork[55] = temp2;
    }
    } else if (iter > 1) {
    nbrofshift = shift[iter-1];
    for (i = 0; i < (int) nbrofshift; i++) {
        temp1 = kwork[27];
        temp2 = kwork[55];
        for (j = 27; j > 0; j--) {
         kwork[j] = kwork[j-1];
         kwork[j+28] = kwork[j+27];
        }
        kwork[0] = temp1;
        kwork[28] = temp2;
    }
    }
    /* Permute kwork - PC2 */
    kn[ 0] = kwork[13];
    kn[ 1] = kwork[16];
    kn[ 2] = kwork[10];
    kn[ 3] = kwork[23];
    kn[ 4] = kwork[ 0];
    kn[ 5] = kwork[ 4];
    kn[ 6] = kwork[ 2];
    kn[ 7] = kwork[27];
    kn[ 8] = kwork[14];
    kn[ 9] = kwork[ 5];
    kn[10] = kwork[20];
    kn[11] = kwork[ 9];
    kn[12] = kwork[22];
    kn[13] = kwork[18];
    kn[14] = kwork[11];
    kn[15] = kwork[ 3];
    kn[16] = kwork[25];
    kn[17] = kwork[ 7];
    kn[18] = kwork[15];
    kn[19] = kwork[ 6];
    kn[20] = kwork[26];
    kn[21] = kwork[19];
    kn[22] = kwork[12];
    kn[23] = kwork[ 1];
    kn[24] = kwork[40];
    kn[25] = kwork[51];
    kn[26] = kwork[30];
    kn[27] = kwork[36];
    kn[28] = kwork[46];
    kn[29] = kwork[54];
    kn[30] = kwork[29];
    kn[31] = kwork[39];
    kn[32] = kwork[50];
    kn[33] = kwork[44];
    kn[34] = kwork[32];
    kn[35] = kwork[47];
    kn[36] = kwork[43];
    kn[37] = kwork[48];
    kn[38] = kwork[38];
    kn[39] = kwork[55];
    kn[40] = kwork[33];
    kn[41] = kwork[52];
    kn[42] = kwork[45];
    kn[43] = kwork[41];
    kn[44] = kwork[49];
    kn[45] = kwork[35];
    kn[46] = kwork[28];
    kn[47] = kwork[31];
    /* KS Function End */
    /* worka XOR kn */
    for (i = 0; i < 48; i++)
    worka[i] = worka[i] ^ kn[i];
    /* 8 s-functions */
    valindex = s1[2*worka[ 0]+worka[ 5]]
    [2*(2*(2*worka[ 1]+worka[ 2])+
    worka[ 3])+worka[ 4]];
    valindex = valindex * 4;
    kn[ 0] = binary[0+valindex];
    kn[ 1] = binary[1+valindex];
    kn[ 2] = binary[2+valindex];
    kn[ 3] = binary[3+valindex];
    valindex = s2[2*worka[ 6]+worka[11]]
    [2*(2*(2*worka[ 7]+worka[ 8])+
    worka[ 9])+worka[10]];
    valindex = valindex * 4;
    kn[ 4] = binary[0+valindex];
    kn[ 5] = binary[1+valindex];
    kn[ 6] = binary[2+valindex];
    kn[ 7] = binary[3+valindex];
    valindex = s3[2*worka[12]+worka[17]]
    [2*(2*(2*worka[13]+worka[14])+
    worka[15])+worka[16]];
    valindex = valindex * 4;
    kn[ 8] = binary[0+valindex];
    kn[ 9] = binary[1+valindex];
    kn[10] = binary[2+valindex];
    kn[11] = binary[3+valindex];
    valindex = s4[2*worka[18]+worka[23]]
    [2*(2*(2*worka[19]+worka[20])+
    worka[21])+worka[22]];
    valindex = valindex * 4;
    kn[12] = binary[0+valindex];
    kn[13] = binary[1+valindex];
    kn[14] = binary[2+valindex];
    kn[15] = binary[3+valindex];
    valindex = s5[2*worka[24]+worka[29]]
    [2*(2*(2*worka[25]+worka[26])+
    worka[27])+worka[28]];
    valindex = valindex * 4;
    kn[16] = binary[0+valindex];
    kn[17] = binary[1+valindex];
    kn[18] = binary[2+valindex];
    kn[19] = binary[3+valindex];
    valindex = s6[2*worka[30]+worka[35]]
    [2*(2*(2*worka[31]+worka[32])+
    worka[33])+worka[34]];
    valindex = valindex * 4;
    kn[20] = binary[0+valindex];
    kn[21] = binary[1+valindex];
    kn[22] = binary[2+valindex];
    kn[23] = binary[3+valindex];
    valindex = s7[2*worka[36]+worka[41]]
    [2*(2*(2*worka[37]+worka[38])+
    worka[39])+worka[40]];
    valindex = valindex * 4;
    kn[24] = binary[0+valindex];
    kn[25] = binary[1+valindex];
    kn[26] = binary[2+valindex];
    kn[27] = binary[3+valindex];
    valindex = s8[2*worka[42]+worka[47]]
    [2*(2*(2*worka[43]+worka[44])+
    worka[45])+worka[46]];
    valindex = valindex * 4;
    kn[28] = binary[0+valindex];
    kn[29] = binary[1+valindex];
    kn[30] = binary[2+valindex];
    kn[31] = binary[3+valindex];
    /* Permute - P */
    worka[ 0] = kn[15];
    worka[ 1] = kn[ 6];
    worka[ 2] = kn[19];
    worka[ 3] = kn[20];
    worka[ 4] = kn[28];
    worka[ 5] = kn[11];
    worka[ 6] = kn[27];
    worka[ 7] = kn[16];
    worka[ 8] = kn[ 0];
    worka[ 9] = kn[14];
    worka[10] = kn[22];
    worka[11] = kn[25];
    worka[12] = kn[ 4];
    worka[13] = kn[17];
    worka[14] = kn[30];
    worka[15] = kn[ 9];
    worka[16] = kn[ 1];
    worka[17] = kn[ 7];
    worka[18] = kn[23];
    worka[19] = kn[13];
    worka[20] = kn[31];
    worka[21] = kn[26];
    worka[22] = kn[ 2];
    worka[23] = kn[ 8];
    worka[24] = kn[18];
    worka[25] = kn[12];
    worka[26] = kn[29];
    worka[27] = kn[ 5];
    worka[28] = kn[21];
    worka[29] = kn[10];
    worka[30] = kn[ 3];
    worka[31] = kn[24];
    /* bufout XOR worka */
    for (i = 0; i < 32; i++) {
        bufout[i+32] = bufout[i] ^ worka[i];
        bufout[i] = buffer[i];
    }
    } /* End of Iter */
    /* Prepare Output */
    for (i = 0; i < 32; i++) {
        j = bufout[i];
        bufout[i] = bufout[32+i];
        bufout[32+i] = j;
    }
    /* Inverse Initial Permutation */
    buffer[ 0] = bufout[39];
    buffer[ 1] = bufout[ 7];
    buffer[ 2] = bufout[47];
    buffer[ 3] = bufout[15];
    buffer[ 4] = bufout[55];
    buffer[ 5] = bufout[23];
    buffer[ 6] = bufout[63];
    buffer[ 7] = bufout[31];
    buffer[ 8] = bufout[38];
    buffer[ 9] = bufout[ 6];
    buffer[10] = bufout[46];
    buffer[11] = bufout[14];
    buffer[12] = bufout[54];
    buffer[13] = bufout[22];
    buffer[14] = bufout[62];
    buffer[15] = bufout[30];
    buffer[16] = bufout[37];
    buffer[17] = bufout[ 5];
    buffer[18] = bufout[45];
    buffer[19] = bufout[13];
    buffer[20] = bufout[53];
    buffer[21] = bufout[21];
    buffer[22] = bufout[61];
    buffer[23] = bufout[29];
    buffer[24] = bufout[36];
    buffer[25] = bufout[ 4];
    buffer[26] = bufout[44];
    buffer[27] = bufout[12];
    buffer[28] = bufout[52];
    buffer[29] = bufout[20];
    buffer[30] = bufout[60];
    buffer[31] = bufout[28];
    buffer[32] = bufout[35];
    buffer[33] = bufout[ 3];
    buffer[34] = bufout[43];
    buffer[35] = bufout[11];
    buffer[36] = bufout[51];
    buffer[37] = bufout[19];
    buffer[38] = bufout[59];
    buffer[39] = bufout[27];
    buffer[40] = bufout[34];
    buffer[41] = bufout[ 2];
    buffer[42] = bufout[42];
    buffer[43] = bufout[10];
    buffer[44] = bufout[50];
    buffer[45] = bufout[18];
    buffer[46] = bufout[58];
    buffer[47] = bufout[26];
    buffer[48] = bufout[33];
    buffer[49] = bufout[ 1];
    buffer[50] = bufout[41];
    buffer[51] = bufout[ 9];
    buffer[52] = bufout[49];
    buffer[53] = bufout[17];
    buffer[54] = bufout[57];
    buffer[55] = bufout[25];
    buffer[56] = bufout[32];
    buffer[57] = bufout[ 0];
    buffer[58] = bufout[40];
    buffer[59] = bufout[ 8];
    buffer[60] = bufout[48];
    buffer[61] = bufout[16];
    buffer[62] = bufout[56];
    buffer[63] = bufout[24];
    j = 0;
    for (i = 0; i < 8; i++) {
        *(dest + i) = 0x00;
        for (k = 0; k < 7; k++)
        *(dest + i) = ((*(dest + i)) + buffer[j+k]) * 2;
        *(dest + i) = *(dest + i) + buffer[j+7];
        j += 8;
    }
}

/* ================================================================
Macshort()
Description: MAC algorithm.
Input:
      pInData[]:待校验数据数组；
	  datalen  :待校验数据数组长度；
	  mca[]    :检验后的数据存放地，8字节；
	  sMacKey[]:加密用的KEY，8字节；
Output:
      None；
================================================================ */
/*void Macshort(unsigned char *sMacKey,unsigned char *pInData,int datalen,unsigned char *initVector,unsigned char *mac)
{
	unsigned char Bufout[8],Databuf[8],Dataend[8];
	int len,i,j;
	len = datalen;


	switch(len%8)
	{
	case 0:
		Dataend[0] = 0x80;
		for(i=1;i<8;i++)
			Dataend[i] = 0x00;
		break;
	case 1:
		Dataend[0] = pInData[len-(len%8)+0];
		Dataend[1] = 0x80;
		for(i=2;i<8;i++)
			Dataend[i] = 0x00;
		break;
	case 2:
		for(i=0;i<2;i++)
		    Dataend[i] = pInData[len-(len%8)+i];
		Dataend[2] = 0x80;
		for(i=3;i<8;i++)
			Dataend[i] = 0x00;
		break;
	case 3:
		for(i=0;i<3;i++)
		    Dataend[i] = pInData[len-(len%8)+i];
		Dataend[3] = 0x80;
		for(i=4;i<8;i++)
			Dataend[i] = 0x00;
		break;
	case 4:
		for(i=0;i<4;i++)
		    Dataend[i] = pInData[len-(len%8)+i];
		Dataend[4] = 0x80;
		for(i=5;i<8;i++)
			Dataend[i] = 0x00;
		break;
	case 5:
		for(i=0;i<5;i++)
		    Dataend[i] = pInData[len-(len%8)+i];
		Dataend[5] = 0x80;
		for(i=6;i<8;i++)
			Dataend[i] = 0x00;
		break;
	case 6:
		for(i=0;i<6;i++)
		    Dataend[i] = pInData[len-(len%8)+i];
		Dataend[6] = 0x80;
		Dataend[7] = 0x00;
		break;
	case 7:
		for(i=0;i<7;i++)
		    Dataend[i] = pInData[len-(len%8)+i];
		Dataend[7] = 0x80;
		break;
	}


	for(i=0;i<8;i++)
		Bufout[i] = initVector[i];

    if(0<len/8)
	{
		for(j=0;j<(len/8);j++)
		{
		    for(i=0;i<8;i++)
			{	
		        Databuf[i] = pInData[8*j + i];
			    Databuf[i] = Databuf[i] ^ Bufout[i];
			}
		    des(Databuf,Bufout,sMacKey,1);		
		}


	    for(i=0;i<8;i++)
		{	
		    Databuf[i] = Dataend[i];
			Databuf[i] = Databuf[i] ^ Bufout[i];
		}
		des(Databuf,Bufout,sMacKey,1);
		
	}
	else 
	{
		for(i=0;i<8;i++)
		{	
		    Databuf[i] = Dataend[i];
			Databuf[i] = Databuf[i] ^ Bufout[i];
		}
		des(Databuf,Bufout,sMacKey,1);	    
	}
	
	

	for(i=0;i<4;i++)
		mac[i]=Bufout[i];
	
} */
void CheckMac(unsigned char *Keya,unsigned char *Keyb,unsigned char *pInData,int datalen,unsigned char *initVector,unsigned char *mac)
{
	unsigned char Bufout[8],Databuf[8],Dataend[8];
	int len,i,j;
	len = datalen;

	switch(len%8)
	{
	case 0:
		Dataend[0] = 0x80;
		for(i=1;i<8;i++)
			Dataend[i] = 0x00;
		break;
	case 1:
		Dataend[0] = pInData[len-(len%8)+0];
		Dataend[1] = 0x80;
		for(i=2;i<8;i++)
			Dataend[i] = 0x00;
		break;
	case 2:
		for(i=0;i<2;i++)
		    Dataend[i] = pInData[len-(len%8)+i];
		Dataend[2] = 0x80;
		for(i=3;i<8;i++)
			Dataend[i] = 0x00;
		break;
	case 3:
		for(i=0;i<3;i++)
		    Dataend[i] = pInData[len-(len%8)+i];
		Dataend[3] = 0x80;
		for(i=4;i<8;i++)
			Dataend[i] = 0x00;
		break;
	case 4:
		for(i=0;i<4;i++)
		    Dataend[i] = pInData[len-(len%8)+i];
		Dataend[4] = 0x80;
		for(i=5;i<8;i++)
			Dataend[i] = 0x00;
		break;
	case 5:
		for(i=0;i<5;i++)
		    Dataend[i] = pInData[len-(len%8)+i];
		Dataend[5] = 0x80;
		for(i=6;i<8;i++)
			Dataend[i] = 0x00;
		break;
	case 6:
		for(i=0;i<6;i++)
		    Dataend[i] = pInData[len-(len%8)+i];
		Dataend[6] = 0x80;
		Dataend[7] = 0x00;
		break;
	case 7:
		for(i=0;i<7;i++)
		    Dataend[i] = pInData[len-(len%8)+i];
		Dataend[7] = 0x80;
		break;
	}


	for(i=0;i<8;i++)
		Bufout[i] = initVector[i];

    if(0<len/8)
	{
		for(j=0;j<(len/8);j++)
		{
		    for(i=0;i<8;i++)
			{	
		        Databuf[i] = pInData[8*j + i];
			    Databuf[i] = Databuf[i] ^ Bufout[i];
			}
		    des(Databuf,Bufout,Keya,1);		
		}


	    for(i=0;i<8;i++)
		{	
		    Databuf[i] = Dataend[i];
			Databuf[i] = Databuf[i] ^ Bufout[i];
		}
		des(Databuf,Bufout,Keya,1);
		
	}
	else 
	{
		for(i=0;i<8;i++)
		{	
		    Databuf[i] = Dataend[i];
			Databuf[i] = Databuf[i] ^ Bufout[i];
		}
		des(Databuf,Bufout,Keya,1);	    
	}
	
	//KMB解密
	for(i = 0;i < 8;i++)
	{
		Databuf[i] = Bufout[i];		
	}
	des(Databuf,Bufout,Keyb,0);

	//KMA加密
	for(i = 0;i < 8;i++)
	{
		Databuf[i] = Bufout[i];		
	}
	des(Databuf,Bufout,Keya,1);

	for(i=0;i<4;i++)
		mac[i]=Bufout[i];
	
}


void Mac(unsigned char *Key,unsigned char *pInData2,int data2len,unsigned char *initData,unsigned char *mac)
{
//    unsigned char tempin[8]={0};
//	unsigned char tempout[8]={0};
	unsigned char keyma[8]={0};
	unsigned char keymb[8]={0};

	int i;
/*	for(i=0;i<8;i++)
		tempin[i] = pInData1[i];
	des(tempin,tempout,Key,1);

	for(i=0;i<8;i++)
		tempin[i] = tempout[i];
	des(tempin,tempout,&Key[8],0);

	for(i=0;i<8;i++)
		tempin[i] = tempout[i];	
	des(tempin,tempout,Key,1);

	for(i = 0;i < 8;i++)
	{
		keya[i] = tempout[i];	//keya
	}

	for(i=0;i<8;i++)
		tempin[i] = pInData1[i];
	des(tempin,tempout,&Key[8],1);

	for(i=0;i<8;i++)
		tempin[i] = tempout[i];
	des(tempin,tempout,Key,0);

	for(i=0;i<8;i++)
		tempin[i] = tempout[i];	
	des(tempin,tempout,&Key[8],1);

	for(i = 0;i < 8;i++)
	{
		keyb[i] = tempout[i];	//keyb
	}
*/
	for(i=0;i<8;i++)
		keyma[i] = Key[i];
	
	for(i=0;i<8;i++)
		keymb[i] = Key[8+i];


	CheckMac(keyma,keymb,pInData2,data2len,initData,mac);

}
/* ================================================================
CalcMasterKey()
Description: 计算非接主密钥
Input:  None
Output: None
================================================================ */
void CalcMasterKey(void)
{
//   uint8_t i,j,even_parity;
//   uint8_t even_parity_value;
//   uint8_t K0[16]= {0};
//   
//   uint8_t kuochong[112];

//   for(i=0;i<14;i++)	//拆分IDm
//   {
//   	  for(j=0;j<8;j++)
//	  {
//	      kuochong[i*8+j] = (IDm[i] >> (7-j)) & 0x01;
//      }
//   }

//   for(i=0;i<16;i++)  //每7个字节算一次偶校验
//   {
//   	   even_parity = 0;

//	   for(j=0;j<7;j++)
//	   {
//	       if(kuochong[i*7+j] == 1)
//		   {
//		   	  even_parity ++;
//		   }
//		   K0[i]  = K0[i] +( kuochong[i*7+j]<<(7-j));
//	   }

//	   if(even_parity%2 ==0)
//	   {
//	   	  even_parity_value = 0;
//	   }
//	   else
//	   {
//	   	  even_parity_value = 1;
//	   }

//	   K0[i]  = K0[i] + even_parity_value;
//   }
//   for(i=0;i<16;i++)
//   {
//  	   RFMasterKey[i]  = K0[i];
//   }
}

/* ================================================================
CalcSessionKey()
Description: 计算非接会话密钥
Input:  buf: 8字节随机数 
        key: 非接主密钥
Output: keyout:
================================================================ */
void CalcSessionKey(uint8_t * buf,uint8_t * key,uint8_t * keyout)
{
    uint8_t	i,keya[8],keyb[8],buffer[8];
	uint8_t	Bufout[8],Databuf[8];

	for(i=0;i<8;i++)
		buffer[i] = buf[i];

	for(i=0;i<8;i++)	 //a Key
		keya[i] = key[i];
	
	for(i=0;i<8;i++)	   // b Key
		keyb[i] = key[8+i];

	des(buffer,Bufout,keya,1);

	for(i = 0;i < 8;i++)
	{
		Databuf[i] = Bufout[i];		
	}
	des(Databuf,Bufout,keyb,0);

	for(i = 0;i < 8;i++)
	{
		Databuf[i] = Bufout[i];		
	}
	des(Databuf,Bufout,keya,1);

	for(i = 0;i < 8;i++)
	{
		keyout[i] = Bufout[i];		
	}

	for(i = 0;i < 8;i++)
	{
		buffer[i] = ~buffer[i];		
	}

	des(buffer,Bufout,keya,1);

	for(i = 0;i < 8;i++)
	{
		Databuf[i] = Bufout[i];		
	}
	des(Databuf,Bufout,keyb,0);

	//KMA加密
	for(i = 0;i < 8;i++)
	{
		Databuf[i] = Bufout[i];		
	}
	des(Databuf,Bufout,keya,1);

	for(i = 0;i < 8;i++)
	{
		keyout[8+i] = Bufout[i];		
	}
}
/* ================================================================
ThreeDes()
Description: 3dea加/解密
Input:  src: 待加/密源数据
        dest:加/密目标数据
		key: 加密密钥
		flag: 加/解密标识 1：加密 
		                  0：解密
Output: None
================================================================ */
void ThreeDes(uint8_t * src, uint8_t * dest, uint8_t * key, int flag)
{
	uint8_t	i,keya[8],keyb[8];
	uint8_t	Bufout[8],Databuf[8];

	for(i=0;i<8;i++)	 //a Key
		keya[i] = key[i];
	
	for(i=0;i<8;i++)	   // b Key
		keyb[i] = key[8+i];
	des(src,Bufout,keya,flag);

	for(i = 0;i < 8;i++)
	{
		Databuf[i] = Bufout[i];		
	}
	des(Databuf,Bufout,keyb,(1-flag));


	for(i = 0;i < 8;i++)
	{
		Databuf[i] = Bufout[i];		
	}
	des(Databuf,Bufout,keya,flag);

	for(i = 0;i < 8;i++)
	{
		dest[i] = Bufout[i];		
	}
}
/* ================================================================
ThreeDes()
Description: 3dea加/解密
Input:  src: 待加/密源数据
        dest:加/密目标数据
		key: 加密密钥
		flag: 加/解密标识 1：加密 
		                  0：解密
Output: None
================================================================ */

//void ThreeDes_ECB(uint8_t * src,uint8_t len, uint8_t * dest, int flag)
//{
//    uint8_t i;
//    uint8_t tmp1[8];
////	CalcMasterKey();
//	 if(flag == 1)
//	 {
//	 	 CalcSessionKey(SDRand,RFMasterKey,Ks);
//		 tmp1[0] = 0;
//		 tmp1[1] = 5;	
//		 for(i=0;i<len;i++)
//		 {
//		 	 tmp1[2+i] =  *(src+i);
//		 }
//		 if(len<6)
//		 {
//		     for(i=len;i<6;i++)
//			 {
//			 	 tmp1[2+i] = 0x80;
//			 }
//		 }
//		 ThreeDes(tmp1, dest, Ks, flag);
//	 }
//	 else
//	 {
//	 	 ThreeDes(src, dest, Ks, flag);
//	 }

//}
