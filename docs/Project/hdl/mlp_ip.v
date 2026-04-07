module mlp_ip (
    // DO NOT EDIT BELOW THIS LINE ////////////////////
    ACLK,
    ARESETN,
    S_AXIS_TREADY,
    S_AXIS_TDATA,
    S_AXIS_TLAST,
    S_AXIS_TVALID,
    M_AXIS_TVALID,
    M_AXIS_TDATA,
    M_AXIS_TLAST,
    M_AXIS_TREADY
    // DO NOT EDIT ABOVE THIS LINE ////////////////////
);

  input ACLK;  // Synchronous clock
  input ARESETN;  // System reset, active low
  // slave in interface
  output reg S_AXIS_TREADY;  // Ready to accept data in
  input [31 : 0] S_AXIS_TDATA;  // Data in
  input S_AXIS_TLAST;  // Optional data in qualifier
  input S_AXIS_TVALID;  // Data in is valid
  // master out interface
  output reg M_AXIS_TVALID;  // Data out is valid
  output  reg [31 : 0] M_AXIS_TDATA;  // Data Out
  output reg M_AXIS_TLAST;  // Optional data out qualifier
  input M_AXIS_TREADY;  // Connected slave device is ready to accept data out

  //----------------------------------------
  // Implementation Section
  //----------------------------------------
  // In this section, we povide an example implementation of MODULE myip_v1_0
  // that does the following:
  //
  // 1. Read all inputs
  // 2. Add each input to the contents of register 'sum' which acts as an accumulator
  // 3. After all the inputs have been read, write out the content of 'sum', 'sum+1', 'sum+2', 'sum+3'
  //
  // You will need to modify this example for
  // MODULE myip_v1_0 to implement your coprocessor


  // RAM parameters for assignment 1
  localparam integer XbDepthBits = 9;
  localparam integer WDepthBits = 3;
  localparam integer MDepthBits = 6;
  localparam integer MbDepthBits = 8;
  localparam integer RESDepthBits = 6;  // 64
  localparam integer Width = 8;

  // wires (or regs) to connect to RAMs and matrix_multiply_0 for assignment 1
  // those which are assigned in an always block of myip_v1_0 shoud be changes to reg.
  reg Xb_write_en;
  reg [XbDepthBits-1:0] Xb_write_address;
  reg [Width-1:0] Xb_write_data_in;
  wire Xb_read_en;
  wire [XbDepthBits-1:0] Xb_read_address;
  wire [Width-1:0] Xb_read_data_out;

  reg W1_write_en;
  reg [WDepthBits-1:0] W1_write_address;
  reg [Width-1:0] W1_write_data_in;
  wire W1_read_en;
  wire [WDepthBits-1:0] W1_read_address;
  wire [Width-1:0] W1_read_data_out;

  reg W2_write_en;
  reg [WDepthBits-1:0] W2_write_address;
  reg [Width-1:0] W2_write_data_in;
  wire W2_read_en;
  wire [WDepthBits-1:0] W2_read_address;
  wire [Width-1:0] W2_read_data_out;

  // for mlp_hid (sync read for both W1 and W2)
  wire W_read_en;
  assign W1_read_en = W_read_en;
  assign W2_read_en = W_read_en;
  wire [WDepthBits-1:0] W_read_address;
  assign W1_read_address = W_read_address;
  assign W2_read_address = W_read_address;

  reg W3_write_en;
  reg [WDepthBits-1:0] W3_write_address;
  reg [Width-1:0] W3_write_data_in;
  wire W3_read_en;
  wire [WDepthBits-1:0] W3_read_address;
  wire [Width-1:0] W3_read_data_out;

  wire M1_write_en;
  wire [MDepthBits-1:0] M1_write_address;
  wire [Width-1:0] M1_write_data_in;
  wire M1_read_en;
  wire [MDepthBits-1:0] M1_read_address;
  wire [Width-1:0] M1_read_data_out;

  wire M2_write_en;
  wire [MDepthBits-1:0] M2_write_address;
  wire [Width-1:0] M2_write_data_in;
  wire M2_read_en;
  wire [MDepthBits-1:0] M2_read_address;
  wire [Width-1:0] M2_read_data_out;

  // for mlp_hid (sync write for both M1 and M2)
  wire M_write_en;
  assign M1_write_en = M_write_en;
  assign M2_write_en = M_write_en;
  wire [MDepthBits-1:0] M_write_address;
  assign M1_write_address = M_write_address;
  assign M2_write_address = M_write_address;

  reg Mb_write_en;
  reg [MbDepthBits-1:0] Mb_write_address;
  reg [Width-1:0] Mb_write_data_in;
  wire Mb_read_en;
  wire [MbDepthBits-1:0] Mb_read_address;
  wire [Width-1:0] Mb_read_data_out;

  wire RES_write_en;
  wire [RESDepthBits-1:0] RES_write_address;
  wire [Width-1:0] RES_write_data_in;
  reg RES_read_en;
  reg [RESDepthBits-1:0] RES_read_address;
  wire [Width-1:0] RES_read_data_out;

  // wires (or regs) to connect to matrix_multiply for assignment 1
  reg Start;  // myip_v1_0 -> matrix_multiply_0. To be assigned within myip_v1_0. Possibly reg.
  wire Done;  // matrix_multiply_0 -> myip_v1_0.

  localparam integer W1Length = 8;
  localparam integer W2Length = 8;
  localparam integer W3Length = 3;
  localparam integer XLength = 448;

  localparam integer NumberOfInputWords  = W1Length + W2Length + W3Length + XLength;
  localparam integer NumberOfOutputWords = 64;

  // Define the states of state machine (one hot encoding)
  localparam integer Idle = 4'b1000;
  localparam integer ReadInputs = 4'b0100;
  localparam integer Compute = 4'b0010;
  localparam integer WriteOutputs = 4'b0001;

  reg [3:0] state;

  // Accumulator to hold sum of inputs read at any point in time
  // reg [31:0] sum;

  // Counters to store the number inputs read & outputs written.
  // Could be done using the same counter if reads and writes are not overlapped (i.e., no dataflow optimization)
  // Left as separate for ease of debugging
  reg [$clog2(NumberOfInputWords) - 1:0] read_counter;
  reg [$clog2(NumberOfOutputWords) - 1:0] write_counter;
  reg [RESDepthBits-1:0] RES_nxt_address;
  reg Write_waiting;  // Wait for 2 cycles

  // CAUTION:
  // The sequence in which data are read in and written out should be
  // consistent with the sequence they are written and read in the driver's hw_acc.c file

  always @(posedge ACLK) begin
    // implemented as a single-always Moore machine
    // a Mealy machine that asserts S_AXIS_TREADY and captures S_AXIS_TDATA etc can save a clock cycle

    /****** Synchronous reset (active low) ******/
    if (!ARESETN) begin
      // CAUTION: make sure your reset polarity is consistent with the system reset polarity
      state <= Idle;
    end else begin
      Xb_write_en  <= 0;
      W1_write_en  <= 0;
      W2_write_en  <= 0;
      W3_write_en  <= 0;
      case (state)

        Idle: begin
          read_counter <= 0;
          write_counter <= 0;
          S_AXIS_TREADY <= 0;
          M_AXIS_TVALID <= 0;
          M_AXIS_TLAST <= 0;

          W1_write_en <= 0;
          W1_write_address <= 0;
          W2_write_en <= 0;
          W2_write_address <= 0;
          W3_write_en <= 0;
          W3_write_address <= 0;
          Xb_write_en <= 0;
          Xb_write_address <= 0;
          if (S_AXIS_TVALID == 1) begin
            state         <= ReadInputs;
            S_AXIS_TREADY <= 1;
            // start receiving data once you go into ReadInputs
          end
        end

        ReadInputs: begin
          S_AXIS_TREADY <= 1;
          if (S_AXIS_TVALID == 1) begin
            // STORE INTO RAM_A
            if (read_counter < W1Length) begin
              // READING W1
              W1_write_en <= 1;
              W1_write_address <= read_counter;
              W1_write_data_in <= S_AXIS_TDATA[Width-1:0];
            end else if (read_counter < W1Length + W2Length) begin
              // READING W2
              W2_write_en <= 1;
              W2_write_address <= read_counter - W1Length;
              W2_write_data_in <= S_AXIS_TDATA[Width-1:0];
            end else if (read_counter < W1Length + W2Length + W3Length) begin
              // READING W1
              W3_write_en <= 1;
              W3_write_address <= read_counter - W1Length - W2Length;
              W3_write_data_in <= S_AXIS_TDATA[Width-1:0];
            end else begin
              // READING X
              Xb_write_en <= 1;
              Xb_write_address <=
              (read_counter - W1Length - W2Length - W3Length) / 7 * 8 +
              (read_counter - W1Length - W2Length - W3Length) % 7 + 1;
              Xb_write_data_in <= S_AXIS_TDATA[Width-1:0];
            end

            // Coprocessor function (adding the numbers together) happens here (partly)
            // sum <=  sum + S_AXIS_TDATA;
            // If we are expecting a variable number of words, we should make use of S_AXIS_TLAST.
            // Since the number of words we are expecting is fixed, we simply count and receive
            // the expected number (NumberOfInputWords) instead.
            if (read_counter == NumberOfInputWords - 1) begin
              state         <= Compute;
              S_AXIS_TREADY <= 0;
            end else begin
              read_counter <= read_counter + 1;
            end
          end
        end

        Compute: begin
          Start <= 1;
          if (Done) begin
            Start    <= 0;
            state    <= WriteOutputs;
            // START THE FIRST READING OF THE RES RAM
            RES_read_en   <= 1;
            RES_read_address  <= 0;
            RES_nxt_address  <= 1;
            write_counter   <= 0;
            Write_waiting  <= 1;
          end
          // Possible to save a cycle by asserting M_AXIS_TVALID and presenting M_AXIS_TDATA just before going into
          // WriteOutputs state. However, need to adjust write_counter limits accordingly
          // Alternatively, M_AXIS_TVALID and M_AXIS_TDATA can be asserted combinationally to save a cycle.
        end

        WriteOutputs: begin
          if (Write_waiting) begin
            // still waiting for the first word, read the second word
            RES_read_en <= 1;
            RES_read_address <= RES_nxt_address;
            RES_nxt_address <= RES_nxt_address + 1;
            Write_waiting <= 0;
          end else if (M_AXIS_TREADY) begin  // DATA IS READY AND THE RECEIVER IS READY
            // TRANSMIT THE FIRST WORD AND READ THE third word
            M_AXIS_TVALID <= 1;
            M_AXIS_TLAST  <= 0;
            M_AXIS_TDATA  <= RES_read_data_out;
            write_counter <= write_counter + 1;
            if (write_counter == NumberOfOutputWords - 1) begin
              // LAST WORD, complete writing
              M_AXIS_TLAST  <= 1;
              RES_read_en   <= 0;
              state    <= Idle;
            end
            // READ THE NEXT WORD (third one onwards)
            if (RES_nxt_address > 0) begin
              // check if reading is complete (overflow means completion)
              RES_read_en <= 1;
              RES_read_address <= RES_nxt_address;
              RES_nxt_address <= RES_nxt_address + 1;
            end
          end
        end
      endcase
    end
  end

  // Connection to sub-modules / components for assignment 1

  initialized_ram #(
      .Width(Width),
      .DepthBits(XbDepthBits)
  ) Xb_RAM (
      .clk(ACLK),
      .write_en(Xb_write_en),
      .write_address(Xb_write_address),
      .write_data_in(Xb_write_data_in),
      .read_en(Xb_read_en),
      .read_address(Xb_read_address),
      .read_data_out(Xb_read_data_out)
  );

  memory_RAM #(
      .width(Width),
      .depth_bits(WDepthBits)
  ) W1_RAM (
      .clk(ACLK),
      .write_en(W1_write_en),
      .write_address(W1_write_address),
      .write_data_in(W1_write_data_in),
      .read_en(W1_read_en),
      .read_address(W1_read_address),
      .read_data_out(W1_read_data_out)
  );

  memory_RAM #(
      .width(Width),
      .depth_bits(WDepthBits)
  ) W2_RAM (
      .clk(ACLK),
      .write_en(W2_write_en),
      .write_address(W2_write_address),
      .write_data_in(W2_write_data_in),
      .read_en(W2_read_en),
      .read_address(W2_read_address),
      .read_data_out(W2_read_data_out)
  );

  memory_RAM #(
      .width(Width),
      .depth_bits(WDepthBits)
  ) W3_RAM (
      .clk(ACLK),
      .write_en(W3_write_en),
      .write_address(W3_write_address),
      .write_data_in(W3_write_data_in),
      .read_en(W3_read_en),
      .read_address(W3_read_address),
      .read_data_out(W3_read_data_out)
  );

  memory_RAM #(
      .width(Width),
      .depth_bits(MDepthBits)
  ) M1_RAM (
      .clk(ACLK),
      .write_en(M1_write_en),
      .write_address(M1_write_address),
      .write_data_in(M1_write_data_in),
      .read_en(M1_read_en),
      .read_address(M1_read_address),
      .read_data_out(M1_read_data_out)
  );

  memory_RAM #(
      .width(Width),
      .depth_bits(MDepthBits)
  ) M2_RAM (
      .clk(ACLK),
      .write_en(M2_write_en),
      .write_address(M2_write_address),
      .write_data_in(M2_write_data_in),
      .read_en(M2_read_en),
      .read_address(M2_read_address),
      .read_data_out(M2_read_data_out)
  );

  initialized_ram #(
      .Width(Width),
      .DepthBits(MbDepthBits)
  ) Mb_RAM (
      .clk(ACLK),
      .write_en(Mb_write_en),
      .write_address(Mb_write_address),
      .write_data_in(Mb_write_data_in),
      .read_en(Mb_read_en),
      .read_address(Mb_read_address),
      .read_data_out(Mb_read_data_out)
  );


  memory_RAM #(
      .width(Width),
      .depth_bits(RESDepthBits)
  ) RES_RAM (
      .clk(ACLK),
      .write_en(RES_write_en),
      .write_address(RES_write_address),
      .write_data_in(RES_write_data_in),
      .read_en(RES_read_en),
      .read_address(RES_read_address),
      .read_data_out(RES_read_data_out)
  );

  mlp_hid #(
      .Width     (Width),
      .XDepthBits(XbDepthBits),
      .WDepthBits(WDepthBits)
  ) mlp_hid_inst (
      .clk  (ACLK),
      .Start(Start),
      .Done (Done),

      .X_read_en(Xb_read_en),
      .X_read_address(Xb_read_address),
      .X_read_data_out(Xb_read_data_out),

      .W_read_en(W_read_en),
      .W_read_address(W_read_address),
      .W1_read_data_out(W1_read_data_out),
      .W2_read_data_out(W2_read_data_out),

      .RES_write_en(M_write_en),
      .RES_write_address(M_write_address),
      .RES_write_data_in1(M1_write_data_in),
      .RES_write_data_in2(M2_write_data_in)
  );

endmodule

