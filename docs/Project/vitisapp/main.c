/*
----------------------------------------------------------------------------------
--	(c) Rajesh C Panicker, NUS,
--	Modified from XLlFifo_polling_example.c, (c) Xilinx Inc
--  Description : Self-checking sample program for AXI Stream Coprocessor interfaced using AXI Stream FIFO.
--	License terms :
--	You are free to use this code as long as you
--		(i) DO NOT post a modified version of this on any public repository;
--		(ii) use it only for educational purposes;
--		(iii) accept the responsibility to ensure that your implementation does not violate any intellectual property of any entity.
--		(iv) accept that the program is provided "as is" without warranty of any kind or assurance regarding its suitability for any particular purpose;
--		(v) send an email to rajesh.panicker@ieee.org briefly mentioning its use (except when used for the course EE4218 at the National University of Singapore);
--		(vi) retain this notice in this file or any files derived from this.
----------------------------------------------------------------------------------
*/

/***************************** Include Files *********************************/
#include "xaxidma.h"
#include "xdebug.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xstatus.h"

#include "mlp_data.h"
#include "dma_interface.h"
#include <stdio.h>

// #define USE_DATA_H

// #define A_LEN 512
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

#define NUMBER_OF_INPUT_WORDS W1_LEN + W2_LEN + W3_LEN + X_LEN
#define NUMBER_OF_OUTPUT_WORDS RES_LEN

u32 W1[W1_LEN];
u32 W2[W2_LEN];
u32 W3[W3_LEN];
u32 X[X_LEN];
u32 Xb[Xb_LEN];
u32 Mb[Mb_LEN];
u32 RES[RES_LEN];

void initData();
void mlp_core();
void software_mlp();

const int db = 0;

int main()
{
  if(db) xil_printf("[main] EE4218 LAB 3 <G-FriAM-1> DMA DEMO BEGINS.\r\n");
  initData();
  dma(db);
  software_mlp();
  if(db) xil_printf("[main] EE4218 LAB 3 <G-FriAM-1> DMA DEMO BEGINS.\r\n");
	return XST_SUCCESS;
}

void initData()
{
  memset((void *)TX_BUFFER_BASE, 0, sizeof(u32) * NUMBER_OF_INPUT_WORDS);
  memset((void *)RX_BUFFER_BASE, 0, sizeof(u32) * NUMBER_OF_OUTPUT_WORDS);
  u32 *tx_buffer = (u32 *)TX_BUFFER_BASE;

  size_t word_cnt,i;

  for (word_cnt = 0, i = 0; i < W1_LEN; i++, word_cnt++)
    tx_buffer[word_cnt] = data_W1[i];
  for (i = 0; i < W2_LEN; i++, word_cnt++)
    tx_buffer[word_cnt] = data_W2[i];
  for (i = 0; i < W3_LEN; i++, word_cnt++)
    tx_buffer[word_cnt] = data_W3[i];
  for (i = 0; i < X_LEN; i++, word_cnt++)
    tx_buffer[word_cnt] = data_X[i];

  memcpy(W1, data_W1, sizeof(W1));
  memcpy(W2, data_W2, sizeof(W2));
  memcpy(W3, data_W3, sizeof(W3));
  memcpy(X, data_X, sizeof(X));
}

void mlp_core(){
  memset(Xb, -1, sizeof(Xb));
  memset(Mb, -1, sizeof(Mb));
  for (int i = 0; i < X_LEN; i++)
  {
    Xb[i / FeatureN * (FeatureN + 1) + i % FeatureN + 1] = X[i];
    // printf("W1[%d] = %u\n",W1[word_cnt]);
  }
  u32 acc1,acc2;
  for (int i = 0; i < SampleN; ++i)
  {
    acc1 = 0;
    acc2 = 0;
    int base = i * W1_LEN;
    for (int j = 0; j < W1_LEN; ++j)
    {
      acc1 += Xb[base + j] * W1[j];
      acc2 += Xb[base + j] * W2[j];
      // printf("i=%d j=%d acc2 = %u\n", i,j, acc2);
    }
    Mb[i * 3 + 1] = sig_value[acc1/256];
    Mb[i * 3 + 2] = sig_value[acc2/256];

    // printf("M[%d] = %u %u %u\n", i, Mb[i * 3], Mb[i * 3 + 1], Mb[i * 3 + 2]);
  }

  // res = Mb @ W3
  for (int i = 0; i < SampleN; ++i)
  {
    acc1 = 0;
    int base = i * W3_LEN;
    for (int j = 0; j < W3_LEN; ++j)
    {
      acc1 += Mb[base + j] * W3[j];
      // printf("i=%d j=%d acc1 = %u\n", i,j, acc1);
    }
    RES[i] = acc1/256;
  }
}

void software_mlp() {
//   initTimer();
//   u32 t1 = startTimer(), t2, diff;

//   for(int i=0;i<MATMUL_CYCLE_TIME;i++)
    mlp_core();

//   diff = endTimer(t1, &t2);
//   double t_ns, t_us;
//   cycle2time(diff, &t_ns, &t_us, NULL, NULL);
//   if(db) printf("[matmul] Time: %d cycles = %.2fns = %.2fus\r\n", diff, t_ns, t_us);
//   // xil_printf("[matmul] Time: t1=%d t2=%d diff=%d\r\n",t1,t2,diff);
  for (int i = 0; i < (int)RES_LEN; i++)
    xil_printf("[software] [%d]=%d\r\n",i, RES[i]);
  // if(db) xil_printf("\r\n");
}