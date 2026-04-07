`timescale 1ns / 1ps

module mlp_tb ();
  reg           ACLK = 0;  // Synchronous clock
  reg           ARESETN;  // System reset, active low
  // slave in interface
  wire          S_AXIS_TREADY;  // Ready to accept data in
  reg  [31 : 0] S_AXIS_TDATA;  // Data in
  reg           S_AXIS_TLAST;  // Optional data in qualifier
  reg           S_AXIS_TVALID;  // Data in is valid
  // master out interface
  wire          M_AXIS_TVALID;  // Data out is valid
  wire [31 : 0] M_AXIS_TDATA;  // Data out
  wire          M_AXIS_TLAST;  // Optional data out qualifier
  reg           M_AXIS_TREADY;  // Connected slave device is ready to accept data out

  mlp_ip dut (
      .ACLK(ACLK),
      .ARESETN(ARESETN),
      .S_AXIS_TREADY(S_AXIS_TREADY),
      .S_AXIS_TDATA(S_AXIS_TDATA),
      .S_AXIS_TLAST(S_AXIS_TLAST),
      .S_AXIS_TVALID(S_AXIS_TVALID),
      .M_AXIS_TVALID(M_AXIS_TVALID),
      .M_AXIS_TDATA(M_AXIS_TDATA),
      .M_AXIS_TLAST(M_AXIS_TLAST),
      .M_AXIS_TREADY(M_AXIS_TREADY)
  );

  localparam integer NumberInputWords  = 467;
  // length of an input vector (for lab 1: 12) (for lab 3: 520)
  localparam integer NumberOutputWords  = 64;
  // length of an output vector (for lab 1:  2) (for lab 3: 64)
  localparam integer NumberTestVectors = 1;
  // number of such test vectors (cases) (for lab 1: 5)
  localparam integer Width = 8;  // Width of an input vector

  // 4 inputs * 2
  reg [Width-1:0] test_input_memory[0:NumberTestVectors*NumberInputWords-1];
  // 4 outputs *2
  reg [Width-1:0] test_result_expected_memory[0:NumberTestVectors*NumberOutputWords-1];
  // same size as test_result_expected_memory
  reg [Width-1:0] result_memory[0:NumberTestVectors*NumberOutputWords-1];

  integer word_cnt, test_case_cnt;
  reg success = 1'b1;
  reg M_AXIS_TLAST_prev = 1'b0;

  // Clock Generation T = 100ns
  always #50 ACLK = ~ACLK;

  // stimulations & updates following the CLK signal
  always @(posedge ACLK) M_AXIS_TLAST_prev <= M_AXIS_TLAST;

  initial begin
    $display("Loading Memory.");

    // add the .mem file to the project or specify the complete path
    $readmemh("mlp_test_input.mem", test_input_memory);  // for lab 3
    // $readmemh("test_input.mem", test_input_memory); // for lab 1

    // add the .mem file to the project or specify the complete path
    $readmemh("mlp_test_expected.mem", test_result_expected_memory);  // for lab 3
    // $readmemh("test_result_expected.mem", test_result_expected_memory);  // for lab 1

    #25  // to make inputs and capture from testbench not aligned with clock edges
    ARESETN = 1'b0;  // apply reset (active low)
    S_AXIS_TVALID = 1'b0;  // no valid data placed on the S_AXIS_TDATA yet
    S_AXIS_TLAST = 1'b0;
    // not required unless we are dealing with an unknown number of inputs. Ignored by the coprocessor. We will be asserting it correctly anyway
    M_AXIS_TREADY = 1'b0;  // not ready to receive data from the co-processor yet.

    #100  // hold reset for 100 ns.
    ARESETN = 1'b1;  // release reset


    for (
        test_case_cnt = 0; test_case_cnt < NumberTestVectors; test_case_cnt = test_case_cnt + 1
    ) begin
      word_cnt = 0;
      S_AXIS_TVALID = 1'b1;  // data is ready at the input of the coprocessor.
      while (word_cnt < NumberInputWords) begin
        if(S_AXIS_TREADY)
        // S_AXIS_TREADY is asserted by the coprocessor in response to S_AXIS_TVALID
        begin
          S_AXIS_TDATA = test_input_memory[word_cnt+test_case_cnt*NumberInputWords];
          // set the next data ready
          if (word_cnt == NumberInputWords - 1) S_AXIS_TLAST = 1'b1;
          else S_AXIS_TLAST = 1'b0;
          word_cnt = word_cnt + 1;
        end
        #100;
        // wait for one clock cycle before for co-processor to capture data (if S_AXIS_TREADY was set)
        // or before checking S_AXIS_TREADY again (if S_AXIS_TREADY was not set)
      end
      S_AXIS_TVALID = 1'b0;  // we no longer give any data to the co-processor
      S_AXIS_TLAST = 1'b0;

      /// Output
      // Note: result_memory is not written at a clock edge, which is fine as it is just a testbench construct and not actual hardware
      word_cnt = 0;
      M_AXIS_TREADY = 1'b1;  // we are now ready to receive data
      // receive data until the falling edge of M_AXIS_TLAST
      while (M_AXIS_TLAST | ~M_AXIS_TLAST_prev) begin
        if (M_AXIS_TVALID) begin
          result_memory[word_cnt+test_case_cnt*NumberOutputWords] = M_AXIS_TDATA;
          word_cnt = word_cnt + 1;
        end
        #100;
      end  // receive loop
      M_AXIS_TREADY = 1'b0;  // not ready to receive data from the co-processor anymore.
    end  // next test vector

    // checking correctness of results
    for (
        word_cnt = 0;
        word_cnt < NumberTestVectors * NumberOutputWords;
        word_cnt = word_cnt + 1
    )
    success = success & (result_memory[word_cnt] == test_result_expected_memory[word_cnt]);
    if (success) $display("Test Passed.");
    else $display("Test Failed.");

    $finish;
  end

endmodule
