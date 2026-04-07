#ifndef DMA_INTERFACE_H
#define DMA_INTERFACE_H

/***************************** Include Files *********************************/
#include "xaxidma.h"
#include "xdebug.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xstatus.h"

/***************** Transmit and receive buffers *********************/
#ifndef DDR_BASE_ADDR
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

#define Width 8
#define FeatureN 7
#define SampleN 64
#define NodeN 2

#define W1_LEN (FeatureN + 1)
#define W2_LEN (FeatureN + 1)
#define W3_LEN (NodeN + 1)

#define X_LEN (SampleN * FeatureN)
#define Xb_LEN (SampleN * (FeatureN + 1))
// #define M_LEN (SampleN * NodeN)
#define Mb_LEN (SampleN * (NodeN + 1))
#define RES_LEN SampleN

#define NUMBER_OF_INPUT_WORDS (W1_LEN + W2_LEN + W3_LEN + X_LEN)
#define NUMBER_OF_OUTPUT_WORDS RES_LEN
#define NUMBER_OF_TEST_VECTORS 1  // number of such test vectors (cases)

#define DMA_DEV_ID        XPAR_XAXIDMA_0_BASEADDR

/************************** Variable Definitions *****************************/

int dma_core(const int db);
int dma(const int db);

#endif