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
#define RES_LEN SampleN

#define NUMBER_OF_INPUT_WORDS (W1_LEN + W2_LEN + W3_LEN + X_LEN)
#define NUMBER_OF_OUTPUT_WORDS RES_LEN

typedef ap_axis<32, 0, 0, 0> AXIS;
typedef ap_uint<64> XROW_T;   // 8 bytes: bias + 7 features
typedef ap_uint<24> MBROW_T;  // 3 bytes: bias + 2 hidden nodes

static void read_weights(
    hls::stream<AXIS> &S_AXIS,
    ap_uint<Width> W1[W1_LEN],
    ap_uint<Width> W2[W2_LEN],
    ap_uint<Width> W3[W3_LEN])
{
#pragma HLS INLINE off

read_w1:
    for (int i = 0; i < W1_LEN; i++)
    {
#pragma HLS PIPELINE II=1
        W1[i] = S_AXIS.read().data;
    }

read_w2:
    for (int i = 0; i < W2_LEN; i++)
    {
#pragma HLS PIPELINE II=1
        W2[i] = S_AXIS.read().data;
    }

read_w3:
    for (int i = 0; i < W3_LEN; i++)
    {
#pragma HLS PIPELINE II=1
        W3[i] = S_AXIS.read().data;
    }
}

static void read_x_samples(
    hls::stream<AXIS> &S_AXIS,
    hls::stream<XROW_T> &x_stream)
{
#pragma HLS INLINE off

read_samples:
    for (int i = 0; i < SampleN; i++)
    {
        XROW_T row = 0;
        row.range(7, 0) = 0xFF; // bias

    read_features:
        for (int j = 0; j < FeatureN; j++)
        {
#pragma HLS PIPELINE II=1
            row.range((j + 1) * 8 + 7, (j + 1) * 8) = S_AXIS.read().data;
        }

        x_stream.write(row);
    }
}

static void hidden_layer(
    hls::stream<XROW_T> &x_stream,
    hls::stream<MBROW_T> &mb_stream,
    ap_uint<Width> W1[W1_LEN],
    ap_uint<Width> W2[W2_LEN])
{
#pragma HLS INLINE off

hidden_samples:
    for (int i = 0; i < SampleN; i++)
    {
#pragma HLS PIPELINE II=1
        XROW_T row = x_stream.read();
        ap_uint<19> acc1 = 0;
        ap_uint<19> acc2 = 0;

    hidden_dot:
        for (int j = 0; j < W1_LEN; j++)
        {
#pragma HLS UNROLL
            ap_uint<Width> x = row.range(j * 8 + 7, j * 8);
            acc1 += x * W1[j];
            acc2 += x * W2[j];
        }

        MBROW_T mb = 0;
        mb.range(7, 0) = 0xFF; // bias
        mb.range(15, 8) = sig_value[acc1.range(15, 8)];
        mb.range(23, 16) = sig_value[acc2.range(15, 8)];

        mb_stream.write(mb);
    }
}

static void output_layer(
    hls::stream<MBROW_T> &mb_stream,
    hls::stream<ap_uint<Width> > &res_stream,
    ap_uint<Width> W3[W3_LEN])
{
#pragma HLS INLINE off

output_samples:
    for (int i = 0; i < SampleN; i++)
    {
#pragma HLS PIPELINE II=1
        MBROW_T mb = mb_stream.read();
        ap_uint<19> acc = 0;

    output_dot:
        for (int j = 0; j < W3_LEN; j++)
        {
#pragma HLS UNROLL
            ap_uint<Width> m = mb.range(j * 8 + 7, j * 8);
            acc += m * W3[j];
        }

        res_stream.write(acc.range(15, 8));
    }
}

static void write_results(
    hls::stream<ap_uint<Width> > &res_stream,
    hls::stream<AXIS> &M_AXIS)
{
#pragma HLS INLINE off

write_out:
    for (int i = 0; i < NUMBER_OF_OUTPUT_WORDS; i++)
    {
#pragma HLS PIPELINE II=1
        AXIS out_word;
        out_word.data = res_stream.read();
        out_word.last = (i == NUMBER_OF_OUTPUT_WORDS - 1) ? 1 : 0;
        out_word.keep = 0xF;
        out_word.strb = 0xF;
        M_AXIS.write(out_word);
    }
}

void mlp_hls(hls::stream<AXIS> &S_AXIS, hls::stream<AXIS> &M_AXIS)
{
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE axis port=S_AXIS
#pragma HLS INTERFACE axis port=M_AXIS

    ap_uint<Width> W1[W1_LEN];
    ap_uint<Width> W2[W2_LEN];
    ap_uint<Width> W3[W3_LEN];

#pragma HLS ARRAY_PARTITION variable=W1 complete dim=1
#pragma HLS ARRAY_PARTITION variable=W2 complete dim=1
#pragma HLS ARRAY_PARTITION variable=W3 complete dim=1

    hls::stream<XROW_T> x_stream;
    hls::stream<MBROW_T> mb_stream;
    hls::stream<ap_uint<Width> > res_stream;

#pragma HLS STREAM variable=x_stream depth=8
#pragma HLS STREAM variable=mb_stream depth=8
#pragma HLS STREAM variable=res_stream depth=8

    read_weights(S_AXIS, W1, W2, W3);

#pragma HLS DATAFLOW
    read_x_samples(S_AXIS, x_stream);
    hidden_layer(x_stream, mb_stream, W1, W2);
    output_layer(mb_stream, res_stream, W3);
    write_results(res_stream, M_AXIS);
}