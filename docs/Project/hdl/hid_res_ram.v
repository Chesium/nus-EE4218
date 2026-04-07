`timescale 1ns / 1ps

// Width is the number of bits per location; DepthBits is the number of address bits. 2^DepthBits is the number of locations

module hid_res_ram #(
    // 256 * 256  2bit per pixel (1 address per pixel)
    parameter integer Width      = 8,  // Width is the number of bits per location
    parameter integer DepthBits = 14  // depth is the number of locations (2^number of address bits)
) (
    input clk,
    input write_en,
    input [DepthBits-1:0] write_address1,
    input [Width-1:0] write_data_in1,
    input [DepthBits-1:0] write_address2,
    input [Width-1:0] write_data_in2,
    input read_en,
    input [DepthBits-1:0] read_address,
    output reg [Width-1:0] read_data_out
);

  reg [Width-1:0] RAM[0:2**DepthBits-1];  // [0x00  0x00  0x00  0x00]  2**7 128
  wire enable;

  // to convert external signals to a form followed in the template given in Vivado synthesis manual. 
  // Not really necessary, but to follow the spirit of using templates
  assign enable  = read_en | write_en;

  // the following is from a template given in Vivado synthesis manual.
  // Read up more about write first, read first, no change modes.

  // initialized to be all one
  integer j;
  initial
    for(j = 0; j < 2**DepthBits; j = j+1)
      RAM[j] = {Width{1'b1}};

  always @(posedge clk) begin
    if (enable) begin
      if (write_en) begin
        RAM[write_address1] <= write_data_in1;
        RAM[write_address2] <= write_data_in2;
      end
      else read_data_out <= RAM[read_address];
    end
  end

endmodule
