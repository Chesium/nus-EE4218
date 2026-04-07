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

#define A_LEN 512
#define B_LEN 8
#define RES_LEN A_LEN / B_LEN

#define NUMBER_OF_INPUT_WORDS A_LEN + B_LEN  // length of an input vector
#define NUMBER_OF_OUTPUT_WORDS RES_LEN  // length of an input vector

// ACLK, ARESETN, TREADY, TDATA, TVALID are essential signals for AXIS. New version of AXI DMA seems to expect TSTRB and/or TKEEP as well.

typedef ap_axis<32,0,0,0> AXIS;  //data, user, id, dest

void myip_v1_0_HLS(hls::stream<AXIS>& S_AXIS, hls::stream<AXIS>& M_AXIS){
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE axis port=S_AXIS
#pragma HLS INTERFACE axis port=M_AXIS

	int word_cnt;
	ap_uint<8> sum = 0; // using arbitrary precision
	// int sum = 0;		 // using 32 bit precision

	ap_uint<19> acc;
	ap_uint<8> A[A_LEN];
	ap_uint<8> B[B_LEN];
	ap_uint<8> res[RES_LEN];
	#pragma HLS ARRAY_PARTITION variable=B complete dim=1
	#pragma HLS ARRAY_PARTITION variable=A cyclic factor=8 dim=1

	AXIS read_input, write_output;

		myip_v1_0_HLS_for_input:for(word_cnt = 0; word_cnt < NUMBER_OF_INPUT_WORDS; word_cnt++){
#pragma HLS UNROLL
			read_input = S_AXIS.read();
			// read_input is the element (data + other signals) received by our ip through S_AXIS in one clock cycle (which contains one word).
			// read() extracts it from the stream. Overloaded operator >> can also be used.
			if (word_cnt < A_LEN)
				A[word_cnt] = read_input.data;
			else 
				B[word_cnt - A_LEN] = read_input.data;
			// sum += read_input.data; //extracting that word
			// We are not making using of S_AXIS_TLAST in this example.
			// S_AXIS_TLAST is required only when we are receiving an unknown number of words.
		}
		
  		myip_v1_0_HLS_for_calc:
		for (int i = 0; i < RES_LEN; ++i) {
			#pragma HLS PIPELINE II=1
    		acc = 0;
    		int base = i * B_LEN;
    		for (int j = 0; j < B_LEN; ++j) {
			#pragma HLS UNROLL
      			acc += A[base + j] * B[j];
    		}
    		res[i] = acc.range(15,8);
  		}

		myip_v1_0_HLS_output:for(word_cnt = 0; word_cnt < NUMBER_OF_OUTPUT_WORDS; word_cnt++){
			#pragma HLS pipeline off
			//write_output.data = sum.to_int() + word_cnt;	// using arbitrary precision internally but int for interfacing
			write_output.data = res[word_cnt];	// using 32 bit precision or arbitrary precision all the way
			// write_output is the element sent by our ip through M_AXIS in one clock cycle.
			write_output.last = 0;
			write_output.keep = 0xFU;
			write_output.strb = 0xFU;
			if(word_cnt==NUMBER_OF_OUTPUT_WORDS-1)
			{
				write_output.last = 1;
				// M_AXIS_TLAST is required to be asserted for the last word.
				// Else, the AXI Stream FIFO / AXI DMA will not know if all the words have been received from the co-processor.
			}
			M_AXIS.write(write_output);
			// write() inserts it into the stream. Overloaded operator << can also be used.
		}
}

void myip_v1_0_HLS_unoptimized(hls::stream<AXIS>& S_AXIS, hls::stream<AXIS>& M_AXIS){
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE axis port=S_AXIS
#pragma HLS INTERFACE axis port=M_AXIS

	int word_cnt;
	ap_uint<8> sum = 0; // using arbitrary precision
	// int sum = 0;		 // using 32 bit precision

	ap_uint<19> acc;
	ap_uint<8> A[A_LEN];
	ap_uint<8> B[B_LEN];
	ap_uint<8> res[RES_LEN];
	// #pragma HLS ARRAY_PARTITION variable=B complete dim=1
	// #pragma HLS ARRAY_PARTITION variable=A cyclic factor=8 dim=1

	AXIS read_input, write_output;

		myip_v1_0_HLS_for_input:for(word_cnt = 0; word_cnt < NUMBER_OF_INPUT_WORDS; word_cnt++){
			#pragma HLS pipeline off
// #pragma HLS UNROLL
			read_input = S_AXIS.read();
			// read_input is the element (data + other signals) received by our ip through S_AXIS in one clock cycle (which contains one word).
			// read() extracts it from the stream. Overloaded operator >> can also be used.
			if (word_cnt < A_LEN)
				A[word_cnt] = read_input.data;
			else 
				B[word_cnt - A_LEN] = read_input.data;
			// sum += read_input.data; //extracting that word
			// We are not making using of S_AXIS_TLAST in this example.
			// S_AXIS_TLAST is required only when we are receiving an unknown number of words.
		}
		
  		myip_v1_0_HLS_for_calc:
		for (int i = 0; i < RES_LEN; ++i) {
			#pragma HLS pipeline off
			// #pragma HLS PIPELINE II=1
    		acc = 0;
    		int base = i * B_LEN;
    		for (int j = 0; j < B_LEN; ++j) {
			#pragma HLS pipeline off
			// #pragma HLS UNROLL
      			acc += A[base + j] * B[j];
    		}
    		res[i] = acc.range(15,8);
  		}

		myip_v1_0_HLS_output:for(word_cnt = 0; word_cnt < NUMBER_OF_OUTPUT_WORDS; word_cnt++){
			#pragma HLS pipeline off
			//write_output.data = sum.to_int() + word_cnt;	// using arbitrary precision internally but int for interfacing
			write_output.data = res[word_cnt];	// using 32 bit precision or arbitrary precision all the way
			// write_output is the element sent by our ip through M_AXIS in one clock cycle.
			write_output.last = 0;
			write_output.keep = 0xFU;
			write_output.strb = 0xFU;
			if(word_cnt==NUMBER_OF_OUTPUT_WORDS-1)
			{
				write_output.last = 1;
				// M_AXIS_TLAST is required to be asserted for the last word.
				// Else, the AXI Stream FIFO / AXI DMA will not know if all the words have been received from the co-processor.
			}
			M_AXIS.write(write_output);
			// write() inserts it into the stream. Overloaded operator << can also be used.
		}
}
