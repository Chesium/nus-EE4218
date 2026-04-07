#ifndef TIMER_H
#define TIMER_H
#include "xil_printf.h"
#include "xparameters.h"
#include "xtmrctr.h"


/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define TMRCTR_DEVICE_ID XPAR_TMRCTR_0_DEVICE_ID
#else
#define XTMRCTR_BASEADDRESS XPAR_XTMRCTR_0_BASEADDR
#endif

/*
 * This example only uses the 1st of the 2 timer counters contained in a
 * single timer counter hardware device
 */
#define TIMER_COUNTER_0 0

#define CLK_FREQ 99999001

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int timer_main();
#ifndef SDT
int TmrCtrPolledExample(u16 DeviceId, u8 TmrCtrNumber);
#else
int TmrCtrPolledExample(UINTPTR BaseAddr, u8 TmrCtrNumber);
#endif

int initTimer();
u32 startTimer();
u32 endTimer(u32 Value1, u32 *value_ptr);
void cycle2time(u32 cycles, double *ns, double *us, double *ms, double *s);
#endif