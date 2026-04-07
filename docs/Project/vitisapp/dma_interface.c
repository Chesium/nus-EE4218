#include "dma_interface.h"
#include "timer.h"
#include "mlp_data.h"
#include <stdio.h>
#include <xstatus.h>

XAxiDma AxiDma;                 // Device instance
XAxiDma *InstancePtr = &AxiDma; // Device pointer

int dma_core(const int db)
{
  int Status;

  int *test_input_memory = (int *)TX_BUFFER_BASE;
  int *result_memory = (int *)RX_BUFFER_BASE;
  /************************** Initializations *****************************/

  XAxiDma_Config *CfgPtr;

  /* Initialize the XAxiDma device.*/
  CfgPtr = XAxiDma_LookupConfig(DMA_DEV_ID);
  if (!CfgPtr)
  {
    xil_printf("No config found for %d\r\n", DMA_DEV_ID);
    return XST_FAILURE;
  }

  Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
  if (Status != XST_SUCCESS)
  {
    xil_printf("Initialization failed %d\r\n", Status);
    return XST_FAILURE;
  }

  if (XAxiDma_HasSg(&AxiDma))
  {
    xil_printf("Device configured as SG mode \r\n");
    return XST_FAILURE;
  }

  /* Disable interrupts, we use polling mode */
  XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
  XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

  // Xil_DCacheDisable(); // uncomment this as a last resort, which will avoid
  // all cache related issues, but at the expense of performance.

  // if (db)
  //   xil_printf("[DMA_CORE] Transmitting Data ...\r\n");
  /* Flush the SrcBuffer and DestBuffer before the DMA transfer, in case the
   * Data Cache is enabled */
  Xil_DCacheFlushRange((UINTPTR)result_memory, 4 * NUMBER_OF_INPUT_WORDS);
  Xil_DCacheFlushRange((UINTPTR)test_input_memory, 4 * NUMBER_OF_INPUT_WORDS);

  Status =
      XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR)test_input_memory,
                             4 * NUMBER_OF_INPUT_WORDS, XAXIDMA_DMA_TO_DEVICE);

  if (Status != XST_SUCCESS)
  {
    return XST_FAILURE;
  }
  while (XAxiDma_Busy(&AxiDma, XAXIDMA_DMA_TO_DEVICE))
  {
    // wait for transfer to complete
  }
  /* Transmission Complete */
  /************************** Receive the Data Stream
   * *****************************/
  // if (db)
  //   xil_printf("[DMA_CORE] Receiving Data ...\r\n");

  Status =
      XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR)result_memory,
                             4 * NUMBER_OF_OUTPUT_WORDS, XAXIDMA_DEVICE_TO_DMA);

  if (Status != XST_SUCCESS)
    return XST_FAILURE;
  while (XAxiDma_Busy(&AxiDma, XAXIDMA_DEVICE_TO_DMA))
  {
    // wait for transfer to complete
  }
  /* Invalidate the DestBuffer before receiving the data, in case the Data Cache
   * is enabled */
  Xil_DCacheInvalidateRange((UINTPTR)result_memory, 4 * NUMBER_OF_OUTPUT_WORDS);
  /* Reception Complete */
  return XST_SUCCESS;
}

int dma(const int db)
{
  u32 t1, t2, diff;
  initTimer();
  t1 = startTimer();

  int ret = dma_core(db);

  diff = endTimer(t1, &t2);
  double t_ns, t_us;
  cycle2time(diff, &t_ns, &t_us, NULL, NULL);
  printf("[DMA] DMA Time: %d cycles = %.2fns = %.2fus\r\n", diff, t_ns, t_us);

  int *result_memory = (int *)RX_BUFFER_BASE;

  for (int i = 0; i < 64; i++)
    xil_printf("DMA Result[%d]=%d\r\n", i, result_memory[i]);

  printf("[DMA] accuracy: %.4f\r\n", accuracy((uint32_t *)result_memory, SampleN));

  return ret;
}