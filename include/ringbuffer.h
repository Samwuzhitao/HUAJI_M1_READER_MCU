/**
  ******************************************************************************
  * @file   	ringbuffer.h
  * @author  	sam.wu
  * @version 	V1.0.0.0
  * @date   	2016.8.26
  * @brief   	ringbuffer
  ******************************************************************************
  */
	
#ifndef __RINGBUFFER_H_
#define __RINGBUFFER_H_

#include "stm32f10x.h"
#include "app_serial_cmd_process.h"

/* Private define ------------------------------------------------------------*/
/* Set parameter of budffer */
#define REVICE_RINGBUFFER    0
#define SEND_RINGBUFFER      1
#define SPI_REVICE_BUFFER    2
#define PRINT_BUFFER         3
#define SPI_IRQ_BUFFER       4

#define PACKETSIZE           (UART_NBUF+9)
#define REVICEBUFFERSIZE     (1024)
#define SENDBUFFERSIZE       (1024*2)
#define SPIBUFFERSIZE        (1)
#define SPIIRQBUFFERSIZE     (1)
#define PRINTBUFFERSIZE      (1)
#define RINGBUFFERSUM        (5)

/* buffer status  */
#define BUFFEREMPTY          1
#define BUFFERUSEING         2
#define BUFFERFULL           3

/* the threshold of buffer */
#define USAGE_TATE_FULL      (100-PACKETSIZE*100/REVICEBUFFERSIZE)
#define USAGE_TATE_EREMPTY   0

/* buffer operation */
#define OPERATIONWRITE       1
#define OPERATIONREAD        2

#define CLOSEIRQ()           __set_PRIMASK(1)        
#define OPENIRQ()            __set_PRIMASK(0)

#define SPI_DATA_IRQ_BUFFER_BLOCK_COUNT          6

/* Private functions ---------------------------------------------------------*/
uint8_t buffer_get_buffer_status( uint8_t sel );
void serial_ringbuffer_write_data( uint8_t sel, uart_cmd_typedef *data );
void serial_ringbuffer_read_data( uint8_t sel, uart_cmd_typedef *data );
uint8_t serial_ringbuffer_get_usage_rate(uint8_t sel);
void print_write_data_to_buffer( char *str, uint8_t len );
void print_read_data_to_buffer( uint8_t *str ,uint8_t size);
#endif
