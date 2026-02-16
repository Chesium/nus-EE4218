/******************************************************************************
 * Copyright (C) 2013 - 2022 Xilinx, Inc.  All rights reserved.
 * Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file XLlFifo_polling_example.c
 * This file demonstrates how to use the Streaming fifo driver on the xilinx AXI
 * Streaming FIFO IP.The AXI4-Stream FIFO core allows memory mapped access to a
 * AXI-Stream interface. The core can be used to interface to AXI Streaming IPs
 * similar to the LogiCORE IP AXI Ethernet core, without having to use full DMA
 * solution.
 *
 * This is the polling example for the FIFO it assumes that at the
 * h/w level FIFO is connected in loopback.In these we write known amount of
 * data to the FIFO and Receive the data and compare with the data transmitted.
 *
 * Note: The TDEST Must be enabled in the H/W design inorder to
 * get correct RDR value.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 3.00a adk 08/10/2013 initial release CR:727787
 * 5.1   ms  01/23/17   Modified xil_printf statement in main function to
 *                      ensure that "Successfully ran" and "Failed" strings
 *                      are available in all examples. This is a fix for
 *                      CR-965028.
 *       ms  04/05/17   Added tabspace for return statements in functions for
 *                      proper documentation and Modified Comment lines
 *                      to consider it as a documentation block while
 *                      generating doxygen.
 * 5.3  rsp 11/08/18    Modified TxSend to fill SourceBuffer with non-zero
 *                      data otherwise the test can return a false positive
 *                      because DestinationBuffer is initialized with zeros.
 *                      In fact, fixing this exposed a bug in RxReceive and
 *                      caused the test to start failing. According to the
 *                      product guide (pg080) for the AXI4-Stream FIFO, the
 *                      RDFO should be read before reading RLR. Reading RLR
 *                      first will result in the RDFO being reset to zero and
 *                      no data being received.
 * </pre>
 *
 * ***************************************************************************
 */

#include "fifo_interface.h"
#include "stdio.h"
#include "timer.h"

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XLlFifo FifoInstance;

u32 SourceBuffer[MAX_DATA_BUFFER_SIZE * WORD_SIZE];
u32 DestinationBuffer[MAX_DATA_BUFFER_SIZE * WORD_SIZE];

void initData(const u32 *A, size_t A_size, const u32 *B, size_t B_size) {
  memset(SourceBuffer, 0, sizeof(SourceBuffer));
  memset(DestinationBuffer, 0, sizeof(DestinationBuffer));
  for (size_t i = 0; i < A_size; i++)
    SourceBuffer[i] = A[i];
  for (size_t i = 0; i < B_size; i++)
    SourceBuffer[A_size + i] = B[i];
  // memcpy(SourceBuffer,A,A_size*sizeof(u32));
  // memcpy(SourceBuffer+A_size,B,B_size*sizeof(u32));
}

/*****************************************************************************/
/**
 *
 * Main function
 *
 * This function is the main entry of the Axi FIFO Polling test.
 *
 * @param	None
 *
 * @return
 *		- XST_SUCCESS if tests pass
 * 		- XST_FAILURE if fails.
 *
 * @note		None
 *
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * This function demonstrates the usage AXI FIFO
 * It does the following:
 *       - Set up the output terminal if UART16550 is in the hardware build
 *       - Initialize the Axi FIFO Device.
 *	- Transmit the data
 *	- Receive the data from fifo
 *	- Compare the data
 *	- Return the result
 *
 * @param	InstancePtr is a pointer to the instance of the
 *		XLlFifo component.
 * @param	DeviceId is Device ID of the Axi Fifo Device instance,
 *		typically XPAR_<AXI_FIFO_instance>_DEVICE_ID value from
 *		xparameters.h.
 *
 * @return
 *		-XST_SUCCESS to indicate success
 *		-XST_FAILURE to indicate failure
 *
 ******************************************************************************/
int fifo_loopback_core(const int db) {
  XLlFifo *InstancePtr = &FifoInstance;
  UINTPTR BaseAddress = XPAR_XLLFIFO_0_BASEADDR;
  XLlFifo_Config *Config;
  int Status;
  Status = XST_SUCCESS;

  /* Initial setup for Uart16550 */
#ifdef XPAR_UARTNS550_0_BASEADDR

  Uart550_Setup();

#endif

  /* Initialize the Device Configuration Interface driver */
#ifndef SDT
  Config = XLlFfio_LookupConfig(DeviceId);
#else
  Config = XLlFfio_LookupConfig(BaseAddress);
#endif
  if (!Config) {
#ifndef SDT
    xil_printf("No config found for %d\r\n", DeviceId);
#endif
    return XST_FAILURE;
  }

  // memset(SourceBuffer,0,sizeof(SourceBuffer));
  // memcpy(SourceBuffer,data_A,sizeof(data_A));
  // memcpy(SourceBuffer+(sizeof(data_A)/sizeof(uint32_t)),data_B,sizeof(data_B));

  /*
   * This is where the virtual address would be used, this example
   * uses physical address.
   */
  Status = XLlFifo_CfgInitialize(InstancePtr, Config, Config->BaseAddress);
  if (Status != XST_SUCCESS) {
    xil_printf("Initialization failed\r\n");
    return Status;
  }

  /* Check for the Reset value */
  Status = XLlFifo_Status(InstancePtr);
  XLlFifo_IntClear(InstancePtr, 0xffffffff);
  Status = XLlFifo_Status(InstancePtr);
  if (Status != 0x0) {
    xil_printf("\n ERROR : Reset value of ISR0 : 0x%x\t"
               "Expected : 0x0\r\n",
               XLlFifo_Status(InstancePtr));
    return XST_FAILURE;
  }

  /* Transmit the Data Stream */
#ifdef LOOPBACK_PRINT_LOG
  u32 t1, t2, diff;
  if(db) xil_printf("[fifo_loopback] Transmitting Data ...\r\n");
  initTimer();
  t1 = startTimer();
#endif
  Status = TxSend(InstancePtr, SourceBuffer);
  if (Status != XST_SUCCESS) {
    xil_printf("[fifo_loopback] Transmission of Data failed\r\n");
    return XST_FAILURE;
  }

  /* Receive the Data Stream */
  Status = RxReceive(InstancePtr, DestinationBuffer);
#ifdef LOOPBACK_PRINT_LOG
  diff = endTimer(t1, &t2);
  if (Status != XST_SUCCESS) {
    xil_printf("[fifo_loopback] Receiving data failed");
    return XST_FAILURE;
  }

  double t_ns, t_us;
  cycle2time(diff, &t_ns, &t_us, NULL, NULL);
  if(db) printf("[fifo_loopback] AXIS Loopback Time: %d cycles = %.2fns = %.2fus\r\n",
         diff, t_ns, t_us);
#endif
}

int fifo_loopback(const int db) {
  int i;
  int Error;

  // xil_printf("[fifo_loopback] AXIS Loopback Time: t1=%d t2=%d
  // diff=%d\r\n",t1,t2,diff);

  Error = 0;

  for(i=0;i<LOOPBACK_CYCLE_TIME;i++)
    fifo_loopback_core(db);

  /* Compare the data send with the data received */
  if(db) xil_printf("[fifo_loopback] Comparing data ...\r\n");
  for (i = 0; i < MAX_DATA_BUFFER_SIZE; i++) {
    // xil_printf("[%d]%d~%d ",i,*(SourceBuffer + i),*(DestinationBuffer + i));
    if (*(SourceBuffer + i) != *(DestinationBuffer + i)) {
      Error = 1;
      break;
    }
  }

  if (Error != 0) {
    return XST_FAILURE;
  }

  // if (Status != XST_SUCCESS) {
  //   xil_printf("Axi Streaming FIFO Polling Example Test Failed\r\n");
  //   xil_printf("--- Exiting main() ---\r\n");
  //   return XST_FAILURE;
  // }

  // xil_printf("[fifo_loopback] Successfully tested AXIS FIFO Loopback\r\n");

  // return Status;
  return 0;
}

/*****************************************************************************/
/**
 *
 * TxSend routine, It will send the requested amount of data at the
 * specified addr.
 *
 * @param	InstancePtr is a pointer to the instance of the
 *		XLlFifo component.
 *
 * @param	SourceAddr is the address where the FIFO stars writing
 *
 * @return
 *		-XST_SUCCESS to indicate success
 *		-XST_FAILURE to indicate failure
 *
 * @note		None
 *
 ******************************************************************************/
int TxSend(XLlFifo *InstancePtr, u32 *SourceAddr) {

  // int i;
  int j;

  /* Fill the transmit buffer with incremental pattern */
  // for (i=0;i<MAX_DATA_BUFFER_SIZE;i++)
  // 	*(SourceAddr + i) = i;

  // for(i=0 ; i < NO_OF_PACKETS ; i++){

  // 	/* Writing into the FIFO Transmit Port Buffer */
  // 	for (j=0 ; j < MAX_PACKET_LEN ; j++){
  // 		if( XLlFifo_iTxVacancy(InstancePtr) ){
  // 			XLlFifo_TxPutWord(InstancePtr,
  // 				*(SourceAddr+(i*MAX_PACKET_LEN)+j));
  // 		}
  // 	}

  // }

  /* Writing into the FIFO Transmit Port Buffer */
  for (j = 0; j < MAX_DATA_BUFFER_SIZE; j++) {
    if (XLlFifo_iTxVacancy(InstancePtr)) {
      XLlFifo_TxPutWord(InstancePtr, *(SourceAddr + j));
    }
  }

  /* Start Transmission by writing transmission length into the TLR */
  XLlFifo_iTxSetLen(InstancePtr, (MAX_DATA_BUFFER_SIZE * WORD_SIZE));

  /* Check for Transmission completion */
  while (!(XLlFifo_IsTxDone(InstancePtr))) {
  }

  /* Transmission Complete */
  return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * RxReceive routine.It will receive the data from the FIFO.
 *
 * @param	InstancePtr is a pointer to the instance of the
 *		XLlFifo instance.
 *
 * @param	DestinationAddr is the address where to copy the received data.
 *
 * @return
 *		-XST_SUCCESS to indicate success
 *		-XST_FAILURE to indicate failure
 *
 * @note		None
 *
 ******************************************************************************/
int RxReceive(XLlFifo *InstancePtr, u32 *DestinationAddr) {

  u32 i;
  int Status;
  u32 RxWord;
  static u32 ReceiveLength;

  // xil_printf(" Receiving data ....\r\n");

  while (XLlFifo_iRxOccupancy(InstancePtr)) {
    /* Read Receive Length */
    ReceiveLength = (XLlFifo_iRxGetLen(InstancePtr)) / WORD_SIZE;
    for (i = 0; i < ReceiveLength; i++) {
      RxWord = XLlFifo_RxGetWord(InstancePtr);
      *(DestinationAddr + i) = RxWord;
    }
  }

  Status = XLlFifo_IsRxDone(InstancePtr);
  if (Status != TRUE) {
    xil_printf("Failing in receive complete ... \r\n");
    return XST_FAILURE;
  }

  return XST_SUCCESS;
}

#ifdef XPAR_UARTNS550_0_BASEADDR
/*****************************************************************************/
/*
 *
 * Uart16550 setup routine, need to set baudrate to 9600 and data bits to 8
 *
 * @param	None
 *
 * @return	None
 *
 * @note		None
 *
 ******************************************************************************/
static void Uart550_Setup(void) {

  XUartNs550_SetBaud(XPAR_UARTNS550_0_BASEADDR, XPAR_XUARTNS550_CLOCK_HZ, 9600);

  XUartNs550_SetLineControlReg(XPAR_UARTNS550_0_BASEADDR, XUN_LCR_8_DATA_BITS);
}
#endif