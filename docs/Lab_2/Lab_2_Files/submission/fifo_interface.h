#ifndef FIFO_INTERFACE_H
#define FIFO_INTERFACE_H

/***************************** Include Files *********************************/
#include "data.h"
#include "stdint.h"
#include "stdlib.h"
#include "timer.h"
#include "xil_cache.h"
#include "xil_exception.h"
#include "xllfifo.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xstreamer.h"


#ifdef XPAR_UARTNS550_0_BASEADDR
#include "xuartns550_l.h" /* to use uartns550 */
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#ifndef SDT
#define FIFO_DEV_ID XPAR_AXI_FIFO_0_DEVICE_ID
#endif

#define WORD_SIZE 4 /* Size of words in bytes */

// #define MAX_PACKET_LEN 4

// #define NO_OF_PACKETS 130

#define MAX_DATA_BUFFER_SIZE 520

#define LOOPBACK_CYCLE_TIME 1
#define LOOPBACK_PRINT_LOG

#undef DEBUG

/************************** Function Prototypes ******************************/
#ifdef XPAR_UARTNS550_0_BASEADDR
static void Uart550_Setup(void);
#endif
void initData(const u32 *A, size_t A_size, const u32 *B, size_t B_size);
int fifo_loopback_core(const int db);
int fifo_loopback(const int db);

int TxSend(XLlFifo *InstancePtr, u32 *SourceAddr);
int RxReceive(XLlFifo *InstancePtr, u32 *DestinationAddr);

#endif