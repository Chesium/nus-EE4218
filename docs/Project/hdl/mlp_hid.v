module mlp_hid #(
    parameter integer Width          = 8,  // Width is the number of bits per location
    parameter integer XDepthBits     = 9,  // depth is the number of locations (2^number of address bits)
    parameter integer WDepthBits     = 3
    // (n,m) * (m,1) = (n,1)
    // n*m = 2^A, m = 2^B => n = 2^(A-B)
) (
    input clk,
    input Start, // myip_v1_0 -> matrix_multiply_0.
    output reg Done,

    output reg                    X_read_en,       // matrix_multiply_0 -> X_RAM. Possibly reg.
    output reg [XDepthBits-1:0]   X_read_address,  // matrix_multiply_0 -> X_RAM. Possibly reg.
    input      [       Width-1:0] X_read_data_out, // X_RAM -> matrix_multiply_0.

    // two RAM, same address
    output reg                    W_read_en,       // matrix_multiply_0 -> W_RAM. Possibly reg.
    output reg [WDepthBits-1:0]   W_read_address,  // matrix_multiply_0 -> W_RAM. Possibly reg.
    input      [       Width-1:0] W1_read_data_out, // W_RAM -> matrix_multiply_0.
    input      [       Width-1:0] W2_read_data_out, // W_RAM -> matrix_multiply_0.

    // one RAM, two write ports
    output reg                             RES_write_en,
    output reg [XDepthBits-WDepthBits-1:0] RES_write_address,
    output reg [Width-1:0]                 RES_write_data_in1,
    output reg [Width-1:0]                 RES_write_data_in2
);

  // implement the logic to read X_RAM, read W_RAM, do the multiplication and write the results to RES_RAM
  // Note: X_RAM and W_RAM are to be read synchronously. Read the wiki for more details.

  reg init;
  reg [2*Width-1:0] tmp_res1;
  wire [2*Width-1:0] nxt_tmp_res1;
  reg [2*Width-1:0] tmp_res2;
  wire [2*Width-1:0] nxt_tmp_res2;
  reg begin_new_line;
  reg increase_RES_addr;
  reg read_end;

  wire [XDepthBits-1:0] X_nxt_address;
  wire [WDepthBits-1:0] W_nxt_address;
  wire [XDepthBits-WDepthBits-1:0] RES_nxt_address;

  assign X_nxt_address = X_read_address + 1;
  assign W_nxt_address = W_read_address + 1;
  assign nxt_tmp_res1   = tmp_res1 + X_read_data_out * W1_read_data_out;
  assign nxt_tmp_res2   = tmp_res2 + X_read_data_out * W2_read_data_out;

  // assign Done = Start; // dummy code. Change as appropriate.
  always @(posedge clk) begin
    if (Start) begin
      RES_write_en <= 0;  // default: do not write
      if (init) begin
        // initialization
        X_read_en         <= 1;
        W_read_en         <= 1;
        X_read_address    <= 0;
        W_read_address    <= 0;
        RES_write_address <= 0;
        tmp_res1          <= 0;
        tmp_res2          <= 0;
        begin_new_line    <= 0;
        increase_RES_addr <= 0;
        read_end          <= 0;
        Done              <= 0;
        init              <= 0;
      end else begin
        if (read_end) begin
          X_read_en <= 0;
          W_read_en <= 0;
        end else begin
          X_read_address <= X_nxt_address;
          W_read_address <= W_nxt_address;
          read_end       <= X_nxt_address == 0;
        end
        if (X_nxt_address >= 2 || X_nxt_address == 0) begin
          // when A B outputs are available
          tmp_res1 <= nxt_tmp_res1;
          tmp_res2 <= nxt_tmp_res2;
        end
        if (increase_RES_addr) begin
          RES_write_address <= RES_write_address + 1;
          increase_RES_addr <= 0;
          Done              <= read_end;
        end
        if (begin_new_line) begin
          // STORE THE TMP_RES
          RES_write_en       <= 1;
          RES_write_data_in1 <= nxt_tmp_res1[2*Width-1:Width];
          RES_write_data_in2 <= nxt_tmp_res2[2*Width-1:Width];
          increase_RES_addr  <= 1;
          tmp_res1           <= 0;
          tmp_res2           <= 0;
          begin_new_line     <= 0;
        end
        begin_new_line <= W_nxt_address == 0;  // when overflow
      end
    end else begin
      // Idle state. keep init high
      init <= 1;
      Done <= 0;
    end
  end

endmodule


