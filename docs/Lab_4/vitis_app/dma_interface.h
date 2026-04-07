#ifndef DMA_INTERFACE_H
#define DMA_INTERFACE_H

/***************************** Include Files *********************************/
#include "xaxidma.h"
#include "xdebug.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xstatus.h"
#include "data.h"

/***************** Transmit and receive buffers *********************/
#ifndef DDR_BASE_ADDR
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
DEFAULT SET TO 0x01000000
#define MEM_BASE_ADDR		0x01000000
#else
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x1000000)
#endif

// Transmit and receive buffer allocated sufficiently away from the start of DDR, hopefully not overlapping with the program's other memory segments.
// It is better to hard code transmit and receive buffers to avoid it being in the same cache line as other variables, and for better alignment.
#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000) 
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)

/***************** Macros *********************/
#define NUMBER_OF_INPUT_WORDS 520  // length of an input vector
#define NUMBER_OF_OUTPUT_WORDS 64  // length of an input vector
#define NUMBER_OF_TEST_VECTORS 1  // number of such test vectors (cases)

#define DMA_DEV_ID        XPAR_XAXIDMA_0_BASEADDR

/************************** Variable Definitions *****************************/

void initData(const u32 *A, size_t A_size, const u32 *B, size_t B_size);
int dma_core(const int db);
int dma(const int db);

#endif