/*
----------------------------------------------------------------------------------
--	(c) Rajesh C Panicker, NUS,
--  Description : Self-checking testbench for AXI Stream Coprocessor (HLS)
implementing the sum of 4 numbers
--	License terms :
--	You are free to use this code as long as you
--		(i) DO NOT post a modified version of this on any public
repository;
--		(ii) use it only for educational purposes;
--		(iii) accept the responsibility to ensure that your
implementation does not violate any intellectual property of any entity.
--		(iv) accept that the program is provided "as is" without
warranty of any kind or assurance regarding its suitability for any particular
purpose;
--		(v) send an email to rajesh.panicker@ieee.org briefly mentioning
its use (except when used for the course EE4218/CEG5203 at the National
University of Singapore);
--		(vi) retain this notice in this file or any files derived from
this.
----------------------------------------------------------------------------------
*/

#include "ap_axi_sdata.h"
#include "mlp_data.hpp"
#include "hls_stream.h"
#include <stdio.h>

typedef ap_axis<32, 0, 0, 0> AXIS;

/***************** Coprocessor function declaration *********************/

void mlp_hls(hls::stream<AXIS> &S_AXIS, hls::stream<AXIS> &M_AXIS);

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

#define NUMBER_OF_INPUT_WORDS W1_LEN + W2_LEN + W3_LEN + X_LEN
#define NUMBER_OF_OUTPUT_WORDS RES_LEN

#define NUMBER_OF_TEST_VECTORS 1 // number of such test vectors (cases)

/************************** Variable Definitions *****************************/
int test_input_memory[NUMBER_OF_TEST_VECTORS *
                      NUMBER_OF_INPUT_WORDS]; // 4 inputs * 2
int test_result_expected_memory[NUMBER_OF_TEST_VECTORS *
                                NUMBER_OF_OUTPUT_WORDS]; // 4 outputs *2
int result_memory[NUMBER_OF_TEST_VECTORS *
                  NUMBER_OF_OUTPUT_WORDS]; // same size as
                                           // test_result_expected_memory

/*****************************************************************************
 * Main function
 ******************************************************************************/
int main()
{
  int word_cnt, i;
  int success;
  AXIS read_output, write_input;
  hls::stream<AXIS> S_AXIS;
  hls::stream<AXIS> M_AXIS;

  for (word_cnt = 0, i = 0; i < W1_LEN; i++, word_cnt++)
    test_input_memory[word_cnt] = data_W1[i];
  for (i = 0; i < W2_LEN; i++, word_cnt++)
    test_input_memory[word_cnt] = data_W2[i];
  for (i = 0; i < W3_LEN; i++, word_cnt++)
    test_input_memory[word_cnt] = data_W3[i];
  for (i = 0; i < X_LEN; i++, word_cnt++)
    test_input_memory[word_cnt] = data_X[i];
  for (i = 0; i < RES_LEN; i++)
    test_result_expected_memory[i] = data_ref[i];

  /******************** Input to Coprocessor : Transmit the Data Stream
   * ***********************/

  // printf(" Transmitting Data for test case %d ... \r\n", test_case_cnt);

  for (word_cnt = 0; word_cnt < NUMBER_OF_INPUT_WORDS; word_cnt++)
  {
    if (word_cnt < 8)
      printf("input %d : %d\r\n", word_cnt, test_input_memory[word_cnt]);
    write_input.data = test_input_memory[word_cnt];
    write_input.last = 0;
    if (word_cnt == NUMBER_OF_INPUT_WORDS - 1)
    {
      write_input.last = 1;
      // S_AXIS_TLAST is asserted for the last word.
      // Actually, doesn't matter since we are not making using of
      // S_AXIS_TLAST.
    }
    S_AXIS.write(write_input); // insert one word into the stream
  }

  /* Transmission Complete */

  /********************* Call the hardware function (invoke the co-processor /
   * ip) ***************/

  mlp_hls(S_AXIS, M_AXIS);

  /******************** Output from Coprocessor : Receive the Data Stream
   * ***********************/

  // printf(" Receiving data for test case %d ... \r\n", test_case_cnt);

  for (word_cnt = 0; word_cnt < NUMBER_OF_OUTPUT_WORDS; word_cnt++)
  {

    read_output = M_AXIS.read(); // extract one word from the stream
    result_memory[word_cnt] = read_output.data;
  }

  /* Reception Complete */
  //   }

  /************************** Checking correctness of results
   * *****************************/

  success = 1;

  /* Compare the data send with the data received */
  printf(" Comparing data ...\r\n");
  for (word_cnt = 0; word_cnt < NUMBER_OF_TEST_VECTORS * NUMBER_OF_OUTPUT_WORDS;
       word_cnt++)
  {
    printf("[%d] %d ~ %d\n", word_cnt, result_memory[word_cnt],
           test_result_expected_memory[word_cnt]);
    success = success & (result_memory[word_cnt] ==
                         test_result_expected_memory[word_cnt]);
  }

  if (success != 1)
  {
    printf("Test Failed\r\n");
    return 1;
  }

  printf("Test Success\r\n");

  return 0;
}
