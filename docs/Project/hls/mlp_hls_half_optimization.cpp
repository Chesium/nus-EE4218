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
#define Mb_LEN (SampleN * (NodeN + 1))
#define RES_LEN SampleN

#define NUMBER_OF_INPUT_WORDS W1_LEN + W2_LEN + W3_LEN + X_LEN
#define NUMBER_OF_OUTPUT_WORDS RES_LEN

typedef ap_axis<32, 0, 0, 0> AXIS;

void mlp_hls(hls::stream<AXIS> &S_AXIS, hls::stream<AXIS> &M_AXIS)
{
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE axis port=S_AXIS
#pragma HLS INTERFACE axis port=M_AXIS

  int word_cnt;
  ap_uint<19> acc1;
  ap_uint<19> acc2;

  ap_uint<Width> W1[W1_LEN];
  ap_uint<Width> W2[W2_LEN];
  ap_uint<Width> W3[W3_LEN];
  ap_uint<Width> Xb[Xb_LEN];
  ap_uint<Width> Mb[Mb_LEN];
  ap_uint<Width> res[RES_LEN];

#pragma HLS ARRAY_PARTITION variable=W1 complete dim=1
#pragma HLS ARRAY_PARTITION variable=W2 complete dim=1
#pragma HLS ARRAY_PARTITION variable=W3 complete dim=1
#pragma HLS ARRAY_RESHAPE variable=Xb cyclic factor=W1_LEN dim=1
#pragma HLS ARRAY_RESHAPE variable=Mb cyclic factor=W3_LEN dim=1

  AXIS write_output;

init_bias:
  for (int i = 0; i < SampleN; i++)
  {
#pragma HLS PIPELINE II=1
    Xb[i * (FeatureN + 1)] = 0xFF;
    Mb[i * (NodeN + 1)] = 0xFF;
  }

read_W1:
  for (int i = 0; i < W1_LEN; i++)
  {
#pragma HLS PIPELINE II=1
    W1[i] = S_AXIS.read().data;
  }

read_W2:
  for (int i = 0; i < W2_LEN; i++)
  {
#pragma HLS PIPELINE II=1
    W2[i] = S_AXIS.read().data;
  }

read_W3:
  for (int i = 0; i < W3_LEN; i++)
  {
#pragma HLS PIPELINE II=1
    W3[i] = S_AXIS.read().data;
  }

read_X:
  for (int i = 0; i < X_LEN; i++)
  {
#pragma HLS PIPELINE II=1
    Xb[(i / FeatureN) * (FeatureN + 1) + (i % FeatureN) + 1] = S_AXIS.read().data;
  }

hidden_layer:
  for (int i = 0; i < SampleN; ++i)
  {
#pragma HLS PIPELINE II=1
    acc1 = 0;
    acc2 = 0;
    int base = i * W1_LEN;

hidden_dot:
    for (int j = 0; j < W1_LEN; ++j)
    {
#pragma HLS UNROLL
      acc1 += Xb[base + j] * W1[j];
      acc2 += Xb[base + j] * W2[j];
    }

    Mb[i * W3_LEN + 1] = sig_value[acc1.range(15, 8)];
    Mb[i * W3_LEN + 2] = sig_value[acc2.range(15, 8)];
  }

output_layer:
  for (int i = 0; i < SampleN; ++i)
  {
#pragma HLS PIPELINE II=1
    acc1 = 0;
    int base = i * W3_LEN;

output_dot:
    for (int j = 0; j < W3_LEN; ++j)
    {
#pragma HLS UNROLL
      acc1 += Mb[base + j] * W3[j];
    }

    res[i] = acc1.range(15, 8);
  }

mlp_hls_output:
  for (word_cnt = 0; word_cnt < NUMBER_OF_OUTPUT_WORDS; word_cnt++)
  {
#pragma HLS PIPELINE II=1
    write_output.data = res[word_cnt];
    write_output.last = (word_cnt == NUMBER_OF_OUTPUT_WORDS - 1) ? 1 : 0;
    write_output.keep = 0xF;
    write_output.strb = 0xF;
    M_AXIS.write(write_output);
  }
}