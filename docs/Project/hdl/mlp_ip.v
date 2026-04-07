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
  localparam X_depth_bits = 9;
  localparam W_depth_bits = 3;
  localparam M_depth_bits = 8;
  localparam RES_depth_bits = 6; // 64
  localparam width = 8;

  // wires (or regs) to connect to RAMs and matrix_multiply_0 for assignment 1
  // those which are assigned in an always block of myip_v1_0 shoud be changes to reg.
  reg X_write_en;  // myip_v1_0 -> X_RAM. To be assigned within myip_v1_0. Possibly reg.
  reg    [X_depth_bits-1:0] X_write_address;    // myip_v1_0 -> X_RAM. To be assigned within myip_v1_0. Possibly reg. 
  reg    [width-1:0] X_write_data_in;      // myip_v1_0 -> X_RAM. To be assigned within myip_v1_0. Possibly reg.
  wire X_read_en;  // matrix_multiply_0 -> X_RAM.
  wire [X_depth_bits-1:0] X_read_address;  // matrix_multiply_0 -> X_RAM.
  wire [width-1:0] X_read_data_out;  // X_RAM -> matrix_multiply_0.

  reg W1_write_en;  // myip_v1_0 -> W1_RAM. To be assigned within myip_v1_0. Possibly reg.
  reg    [W1_depth_bits-1:0] W1_write_address;    // myip_v1_0 -> W1_RAM. To be assigned within myip_v1_0. Possibly reg.
  reg    [width-1:0] W1_write_data_in;      // myip_v1_0 -> W1_RAM. To be assigned within myip_v1_0. Possibly reg.
  wire W1_read_en;  // matrix_multiply_0 -> W1_RAM.
  wire [W1_depth_bits-1:0] W1_read_address;  // matrix_multiply_0 -> W1_RAM.
  wire [width-1:0] W1_read_data_out;  // W1_RAM -> matrix_multiply_0.

  reg W2_write_en;  // myip_v1_0 -> W2_RAM. To be assigned within myip_v1_0. Possibly reg.
  reg    [W2_depth_bits-1:0] W2_write_address;    // myip_v1_0 -> W2_RAM. To be assigned within myip_v1_0. Possibly reg.
  reg    [width-1:0] W2_write_data_in;      // myip_v1_0 -> W2_RAM. To be assigned within myip_v1_0. Possibly reg.
  wire W2_read_en;  // matrix_multiply_0 -> W2_RAM.
  wire [W2_depth_bits-1:0] W2_read_address;  // matrix_multiply_0 -> W2_RAM.
  wire [width-1:0] W2_read_data_out;  // W2_RAM -> matrix_multiply_0.

  reg W3_write_en;  // myip_v1_0 -> W3_RAM. To be assigned within myip_v1_0. Possibly reg.
  reg    [W3_depth_bits-1:0] W3_write_address;    // myip_v1_0 -> W3_RAM. To be assigned within myip_v1_0. Possibly reg.
  reg    [width-1:0] W3_write_data_in;      // myip_v1_0 -> W3_RAM. To be assigned within myip_v1_0. Possibly reg.
  wire W3_read_en;  // matrix_multiply_0 -> W3_RAM.
  wire [W3_depth_bits-1:0] W3_read_address;  // matrix_multiply_0 -> W3_RAM.
  wire [width-1:0] W3_read_data_out;  // W3_RAM -> matrix_multiply_0.

  reg M_write_en;  // myip_v1_0 -> M_RAM. To be assigned within myip_v1_0. Possibly reg.
  reg    [M_depth_bits-1:0] M_write_address1;    // myip_v1_0 -> M_RAM. To be assigned within myip_v1_0. Possibly reg. 
  reg    [width-1:0] M_write_data_in1;      // myip_v1_0 -> M_RAM. To be assigned within myip_v1_0. Possibly reg.
  reg    [M_depth_bits-1:0] M_write_address2;    // myip_v1_0 -> M_RAM. To be assigned within myip_v1_0. Possibly reg. 
  reg    [width-1:0] M_write_data_in2;      // myip_v1_0 -> M_RAM. To be assigned within myip_v1_0. Possibly reg.
  wire M_read_en;  // matrix_multiply_0 -> M_RAM.
  wire [M_depth_bits-1:0] M_read_address;  // matrix_multiply_0 -> M_RAM.
  wire [width-1:0] M_read_data_out;  // M_RAM -> matrix_multiply_0.

  wire RES_write_en;  // matrix_multiply_0 -> RES_RAM.
  wire [RES_depth_bits-1:0] RES_write_address;  // matrix_multiply_0 -> RES_RAM.
  wire [width-1:0] RES_write_data_in;  // matrix_multiply_0 -> RES_RAM.
  reg RES_read_en;  // myip_v1_0 -> RES_RAM. To be assigned within myip_v1_0. Possibly reg.
  reg    [RES_depth_bits-1:0] RES_read_address;  // myip_v1_0 -> RES_RAM. To be assigned within myip_v1_0. Possibly reg.
  wire [width-1:0] RES_read_data_out;  // RES_RAM -> myip_v1_0

  // wires (or regs) to connect to matrix_multiply for assignment 1
  reg Start;  // myip_v1_0 -> matrix_multiply_0. To be assigned within myip_v1_0. Possibly reg.
  wire Done;  // matrix_multiply_0 -> myip_v1_0. 

  localparam NUMBER_OF_X_WORDS = 512;  // 2**X_depth_bits
  // Total number of input data.
  localparam NUMBER_OF_INPUT_WORDS  = 520; // 2**X_depth_bits + 2**W_depth_bits = 12 for assignment 1

  // Total number of output data
  localparam NUMBER_OF_OUTPUT_WORDS = 64;  // 2**RES_depth_bits = 2 for assignment 1

  // Define the states of state machine (one hot encoding)
  localparam Idle = 4'b1000;
  localparam Read_Inputs = 4'b0100;
  localparam Compute = 4'b0010;
  localparam Write_Outputs = 4'b0001;

  reg [3:0] state;

  // Accumulator to hold sum of inputs read at any point in time
  // reg [31:0] sum;

  // Counters to store the number inputs read & outputs written.
  // Could be done using the same counter if reads and writes are not overlapped (i.e., no dataflow optimization)
  // Left as separate for ease of debugging
  reg [$clog2(NUMBER_OF_INPUT_WORDS) - 1:0] read_counter;
  reg [$clog2(NUMBER_OF_OUTPUT_WORDS) - 1:0] write_counter;
  reg [RES_depth_bits-1:0] RES_nxt_address;
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
      X_write_en <= 0;
      W_write_en <= 0;
      case (state)

        Idle: begin
          read_counter <= 0;
          write_counter <= 0;
          // sum            <= 0;
          S_AXIS_TREADY <= 0;
          M_AXIS_TVALID <= 0;
          M_AXIS_TLAST <= 0;
          X_write_en <= 0;
          X_write_address <= 0;
          W_write_en <= 0;
          W_write_address <= 0;
          if (S_AXIS_TVALID == 1) begin
            state         <= Read_Inputs;
            S_AXIS_TREADY <= 1;
            // start receiving data once you go into Read_Inputs
          end
        end

        Read_Inputs: begin
          S_AXIS_TREADY <= 1;
          if (S_AXIS_TVALID == 1) begin
            // STORE INTO RAM_A
            if (read_counter < NUMBER_OF_X_WORDS) begin
              // READING MATRIX A
              X_write_en <= 1;
              X_write_address <= read_counter;
              X_write_data_in <= S_AXIS_TDATA[width-1:0];
            end else begin
              // READING MATRIX B
              W_write_en <= 1;
              W_write_address <= read_counter - NUMBER_OF_X_WORDS;  // TODO: improve?
              W_write_data_in <= S_AXIS_TDATA[width-1:0];
            end

            // Coprocessor function (adding the numbers together) happens here (partly)
            // sum <=  sum + S_AXIS_TDATA;
            // If we are expecting a variable number of words, we should make use of S_AXIS_TLAST.
            // Since the number of words we are expecting is fixed, we simply count and receive 
            // the expected number (NUMBER_OF_INPUT_WORDS) instead.
            if (read_counter == NUMBER_OF_INPUT_WORDS - 1) begin
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
            state    <= Write_Outputs;
            // START THE FIRST READING OF THE RES RAM
            RES_read_en   <= 1;
            RES_read_address  <= 0;
            RES_nxt_address  <= 1;
            write_counter   <= 0;
            Write_waiting  <= 1;
          end
          // Possible to save a cycle by asserting M_AXIS_TVALID and presenting M_AXIS_TDATA just before going into 
          // Write_Outputs state. However, need to adjust write_counter limits accordingly
          // Alternatively, M_AXIS_TVALID and M_AXIS_TDATA can be asserted combinationally to save a cycle.
        end

        Write_Outputs: begin
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
            if (write_counter == NUMBER_OF_OUTPUT_WORDS - 1) begin
              // LAST WORD, complete writing
              M_AXIS_TLAST  <= 1;
              RES_read_en   <= 0;
              state    <= Idle;
            end
            // READ THE NEXT WORD (third one onwards)
            if (RES_nxt_address > 0) begin // check if reading is complete (overflow means completion)
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
      .width(width),
      .depth_bits(X_depth_bits)
  ) X_RAM (
      .clk(ACLK),
      .write_en(X_write_en),
      .write_address(X_write_address),
      .write_data_in(X_write_data_in),
      .read_en(X_read_en),
      .read_address(X_read_address),
      .read_data_out(X_read_data_out)
  );

  memory_RAM #(
      .width(width),
      .depth_bits(W1_depth_bits)
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
      .width(width),
      .depth_bits(W2_depth_bits)
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
      .width(width),
      .depth_bits(W3_depth_bits)
  ) W3_RAM (
      .clk(ACLK),
      .write_en(W3_write_en),
      .write_address(W3_write_address),
      .write_data_in(W3_write_data_in),
      .read_en(W3_read_en),
      .read_address(W3_read_address),
      .read_data_out(W3_read_data_out)
  );

  hid_res_ram #(
      .width(width),
      .depth_bits(M_depth_bits)
  ) M_RAM (
      .clk(ACLK),
      .write_en(M_write_en),
      .write_address1(M_write_address1),
      .write_data_in1(M_write_data_in1),
      .write_address2(M_write_address2),
      .write_data_in2(M_write_data_in2),
      .read_en(M_read_en),
      .read_address(M_read_address),
      .read_data_out(M_read_data_out)
  );


  memory_RAM #(
      .width(width),
      .depth_bits(RES_depth_bits)
  ) RES_RAM (
      .clk(ACLK),
      .write_en(RES_write_en),
      .write_address(RES_write_address),
      .write_data_in(RES_write_data_in),
      .read_en(RES_read_en),
      .read_address(RES_read_address),
      .read_data_out(RES_read_data_out)
  );

  matrix_multiply #(
      .width(width),
      .X_depth_bits(X_depth_bits),
      .W_depth_bits(W_depth_bits),
      .RES_depth_bits(RES_depth_bits)
  ) matrix_multiply_0 (
      .clk  (ACLK),
      .Start(Start),
      .Done (Done),

      .X_read_en(X_read_en),
      .X_read_address(X_read_address),
      .X_read_data_out(X_read_data_out),

      .W_read_en(W_read_en),
      .W_read_address(W_read_address),
      .W_read_data_out(W_read_data_out),

      .RES_write_en(RES_write_en),
      .RES_write_address(RES_write_address),
      .RES_write_data_in(RES_write_data_in)
  );

endmodule

