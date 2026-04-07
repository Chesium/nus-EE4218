/*
----------------------------------------------------------------------------------
--	(c) Rajesh C Panicker, NUS,
--  Description : AXI Stream Coprocessor (HLS), implementing the sum of 4 numbers
--	License terms :
--	You are free to use this code as long as you
--		(i) DO NOT post a modified version of this on any public repository;
--		(ii) use it only for educational purposes;
--		(iii) accept the responsibility to ensure that your implementation does not violate any intellectual property of any entity.
--		(iv) accept that the program is provided "as is" without warranty of any kind or assurance regarding its suitability for any particular purpose;
--		(v) send an email to rajesh.panicker@ieee.org briefly mentioning its use (except when used for the course EE4218/CEG5203 at the National University of Singapore);
--		(vi) retain this notice in this file or any files derived from this.
----------------------------------------------------------------------------------
*/

#include "hls_stream.h"
#include "ap_int.h"
#include "ap_axi_sdata.h"
#include "sigmoid_lookup.hpp"
#include <cstdio>

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

// ACLK, ARESETN, TREADY, TDATA, TVALID are essential signals for AXIS. New version of AXI DMA seems to expect TSTRB and/or TKEEP as well.

typedef ap_axis<32, 0, 0, 0> AXIS; // data, user, id, dest

void mlp_hls(hls::stream<AXIS> &S_AXIS, hls::stream<AXIS> &M_AXIS)
{
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INTERFACE axis port = S_AXIS
#pragma HLS INTERFACE axis port = M_AXIS

  int word_cnt;
  ap_uint<Width> sum = 0; // using arbitrary precision
  // int sum = 0;		 // using 32 bit precision

  ap_uint<19> acc1;
  ap_uint<19> acc2;
  ap_uint<Width> W1[W1_LEN];
  ap_uint<Width> W2[W2_LEN];
  ap_uint<Width> W3[W3_LEN];
  ap_uint<Width> Xb[Xb_LEN];
  ap_uint<Width> Mb[Mb_LEN];
  ap_uint<Width> res[RES_LEN];

  // bias term
  for (int i = 0; i < SampleN; i++)
  {
    Xb[i * (FeatureN + 1)] = 0xFF;
    Mb[i * (NodeN + 1)] = 0xFF;
  }

  AXIS read_input, write_output;
  for (int i = 0; i < W1_LEN; i++)
  {
    W1[i] = S_AXIS.read().data;
    // printf("W1[%d] = %u\n", i, W1[i]);
  }
  for (int i = 0; i < W2_LEN; i++)
  {
    W2[i] = S_AXIS.read().data;
    // printf("W2[%d] = %u\n", i, W2[i]);
  }
  for (int i = 0; i < W3_LEN; i++)
  {
    W3[i] = S_AXIS.read().data;
    // printf("W3[%d] = %u\n", i, W3[i]);
  }
  for (int i = 0; i < X_LEN; i++)
  {
    Xb[i / FeatureN * (FeatureN + 1) + i % FeatureN + 1] = S_AXIS.read().data;
    // printf("W1[%d] = %u\n",W1[word_cnt]);
  }

  // Mb = [ones , sig( Xb @ [W1,W2] ) ]
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
    Mb[i * 3 + 1] = sig_value[acc1.range(15, 8)];
    Mb[i * 3 + 2] = sig_value[acc2.range(15, 8)];

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
    res[i] = acc1.range(15, 8);
  }

mlp_hls_output:
  for (word_cnt = 0; word_cnt < NUMBER_OF_OUTPUT_WORDS; word_cnt++)
  {
#pragma HLS pipeline off
    // write_output.data = sum.to_int() + word_cnt;	// using arbitrary precision internally but int for interfacing
    write_output.data = res[word_cnt]; // using 32 bit precision or arbitrary precision all the way
    // write_output is the element sent by our ip through M_AXIS in one clock cycle.
    write_output.last = 0;
    write_output.keep = 0xFU;
    write_output.strb = 0xFU;
    if (word_cnt == NUMBER_OF_OUTPUT_WORDS - 1)
    {
      write_output.last = 1;
      // M_AXIS_TLAST is required to be asserted for the last word.
      // Else, the AXI Stream FIFO / AXI DMA will not know if all the words have been received from the co-processor.
    }
    M_AXIS.write(write_output);
    // write() inserts it into the stream. Overloaded operator << can also be used.
  }
}
