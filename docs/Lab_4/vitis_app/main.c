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

#include "data.h"
#include "dma_interface.h"
#include <stdio.h>

// #define USE_DATA_H

// #define A_LEN 512
#define A_LEN 512
#define B_LEN 8
#define RES_LEN A_LEN / B_LEN
u32 A[A_LEN];
u32 B[B_LEN];
u32 RES[RES_LEN];
void readcsv();
void matmul_core();
void matmul();

const int db = 0;

int main()
{
  if(db) xil_printf("[main] EE4218 LAB 3 <G-FriAM-1> DMA DEMO BEGINS.\r\n");
	// initData(data_A, 512, data_B, 8);
	readcsv();
	dma(db);
	matmul();
  if(db) xil_printf("[main] EE4218 LAB 3 <G-FriAM-1> DMA DEMO BEGINS.\r\n");
	return XST_SUCCESS;
}


void readcsv() {

  memset(A, 0, sizeof(A));
  memset(B, 0, sizeof(B));
  memset(RES, 0, sizeof(RES));

#ifdef USE_DATA_H
  memcpy(A, data_A, sizeof(A));
  memcpy(B, data_B, sizeof(B));
  if(db) xil_printf("[readcsv] USING A & B FROM data.h\r\n");
  return;
#endif

  if(db) xil_printf("[readcsv] PLEASE INPUT MATRIX A (512 INTEGERS)\r\n");
  int ch;
  u32 cur = 0;
  int in_number = 0;
  int count = 0; // target: A_LEN + B_LEN

  while ((ch = getchar()) != EOF) {
    if (ch >= '0' && ch <= '9') {
      // currently reading number
      in_number = 1;
      cur = cur * 10UL + (u32)(ch - '0');
    } else {
      // encounter a delimiter
      if (in_number) {
        if (count < A_LEN) {
          if(db) xil_printf("[readcsv] -> A[%d/511]=%d\r\n", count, (u32)cur);
          A[count] = (u32)cur;
          if (count == A_LEN - 1)
            if(db) xil_printf("[readcsv] PLEASE INPUT MATRIX B (8 INTEGERS)\r\n");
        } else if (count < A_LEN + B_LEN) {
          if(db) xil_printf("[readcsv] -> B[%d/7]=%d\r\n", count - A_LEN, (u32)cur);
          B[count - A_LEN] = (u32)cur;
        }
        count++;
        if (count >= A_LEN + B_LEN)
          break;

        // reset for another number
        cur = 0;
        in_number = 0;
      }
      // otherwise skip delimiters
    }
  }

  // EOF when reading numbers - write in the current number
  if (in_number && count < A_LEN + B_LEN) {
    if (count < A_LEN) {
      A[count] = (u32)cur;
    } else {
      B[count - A_LEN] = (u32)cur;
    }
    count++;
  }
  if(db) xil_printf("[readcsv] Finish reading A & B with a total of %d numbers\r\n",
             count);
  initData(A, A_LEN, B, B_LEN);
}


void matmul_core(){
  for (int i = 0; i < (int)RES_LEN; ++i) {
    u64 acc = 0;
    int base = i * B_LEN;
    for (int j = 0; j < (int)B_LEN; ++j) {
      acc += (u64)A[base + j] * (u64)B[j];
    }
    RES[i] = (u32)(acc >> 8);
  }
}

void matmul() {
//   initTimer();
//   u32 t1 = startTimer(), t2, diff;

//   for(int i=0;i<MATMUL_CYCLE_TIME;i++)
    matmul_core();

//   diff = endTimer(t1, &t2);
//   double t_ns, t_us;
//   cycle2time(diff, &t_ns, &t_us, NULL, NULL);
//   if(db) printf("[matmul] Time: %d cycles = %.2fns = %.2fus\r\n", diff, t_ns, t_us);
//   // xil_printf("[matmul] Time: t1=%d t2=%d diff=%d\r\n",t1,t2,diff);
  for (int i = 0; i < (int)RES_LEN; i++)
    xil_printf("[software] [%d]=%d\r\n",i, RES[i]);
  // if(db) xil_printf("\r\n");
}