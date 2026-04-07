module mlp_hid #(
    parameter integer Width          = 8,  // Width is the number of bits per location
    parameter integer XDepthBits     = 9,  // depth is the number of locations (2^number of address bits)
    parameter integer WDepthBits     = 3,
    parameter integer MbDepthBits     = 3
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
    output wire [MbDepthBits:0]            RES_write_address1,
    output wire [MbDepthBits:0]            RES_write_address2,
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

  reg [XDepthBits-WDepthBits-1:0] RES_write_address;

  assign RES_write_address1 = RES_write_address * 3 + 1;
  assign RES_write_address2 = RES_write_address * 3 + 2;

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
          RES_write_data_in1 <= sig(nxt_tmp_res1[2*Width-1:Width]);
          RES_write_data_in2 <= sig(nxt_tmp_res2[2*Width-1:Width]);
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

  function [7:0] sig;
    input [7:0] a;
    begin
      case (a)
        8'd0: sig = 8'd12;
        8'd1: sig = 8'd12;
        8'd2: sig = 8'd12;
        8'd3: sig = 8'd12;
        8'd4: sig = 8'd13;
        8'd5: sig = 8'd13;
        8'd6: sig = 8'd13;
        8'd7: sig = 8'd14;
        8'd8: sig = 8'd14;
        8'd9: sig = 8'd14;
        8'd10: sig = 8'd15;
        8'd11: sig = 8'd15;
        8'd12: sig = 8'd15;
        8'd13: sig = 8'd16;
        8'd14: sig = 8'd16;
        8'd15: sig = 8'd16;
        8'd16: sig = 8'd17;
        8'd17: sig = 8'd17;
        8'd18: sig = 8'd18;
        8'd19: sig = 8'd18;
        8'd20: sig = 8'd18;
        8'd21: sig = 8'd19;
        8'd22: sig = 8'd19;
        8'd23: sig = 8'd20;
        8'd24: sig = 8'd20;
        8'd25: sig = 8'd21;
        8'd26: sig = 8'd21;
        8'd27: sig = 8'd21;
        8'd28: sig = 8'd22;
        8'd29: sig = 8'd22;
        8'd30: sig = 8'd23;
        8'd31: sig = 8'd23;
        8'd32: sig = 8'd24;
        8'd33: sig = 8'd24;
        8'd34: sig = 8'd25;
        8'd35: sig = 8'd26;
        8'd36: sig = 8'd26;
        8'd37: sig = 8'd27;
        8'd38: sig = 8'd27;
        8'd39: sig = 8'd28;
        8'd40: sig = 8'd28;
        8'd41: sig = 8'd29;
        8'd42: sig = 8'd30;
        8'd43: sig = 8'd30;
        8'd44: sig = 8'd31;
        8'd45: sig = 8'd32;
        8'd46: sig = 8'd32;
        8'd47: sig = 8'd33;
        8'd48: sig = 8'd34;
        8'd49: sig = 8'd34;
        8'd50: sig = 8'd35;
        8'd51: sig = 8'd36;
        8'd52: sig = 8'd36;
        8'd53: sig = 8'd37;
        8'd54: sig = 8'd38;
        8'd55: sig = 8'd39;
        8'd56: sig = 8'd39;
        8'd57: sig = 8'd40;
        8'd58: sig = 8'd41;
        8'd59: sig = 8'd42;
        8'd60: sig = 8'd43;
        8'd61: sig = 8'd44;
        8'd62: sig = 8'd44;
        8'd63: sig = 8'd45;
        8'd64: sig = 8'd46;
        8'd65: sig = 8'd47;
        8'd66: sig = 8'd48;
        8'd67: sig = 8'd49;
        8'd68: sig = 8'd50;
        8'd69: sig = 8'd51;
        8'd70: sig = 8'd52;
        8'd71: sig = 8'd53;
        8'd72: sig = 8'd54;
        8'd73: sig = 8'd55;
        8'd74: sig = 8'd56;
        8'd75: sig = 8'd57;
        8'd76: sig = 8'd58;
        8'd77: sig = 8'd59;
        8'd78: sig = 8'd60;
        8'd79: sig = 8'd61;
        8'd80: sig = 8'd62;
        8'd81: sig = 8'd63;
        8'd82: sig = 8'd64;
        8'd83: sig = 8'd66;
        8'd84: sig = 8'd67;
        8'd85: sig = 8'd68;
        8'd86: sig = 8'd69;
        8'd87: sig = 8'd70;
        8'd88: sig = 8'd72;
        8'd89: sig = 8'd73;
        8'd90: sig = 8'd74;
        8'd91: sig = 8'd75;
        8'd92: sig = 8'd76;
        8'd93: sig = 8'd78;
        8'd94: sig = 8'd79;
        8'd95: sig = 8'd80;
        8'd96: sig = 8'd82;
        8'd97: sig = 8'd83;
        8'd98: sig = 8'd84;
        8'd99: sig = 8'd86;
        8'd100: sig = 8'd87;
        8'd101: sig = 8'd88;
        8'd102: sig = 8'd90;
        8'd103: sig = 8'd91;
        8'd104: sig = 8'd92;
        8'd105: sig = 8'd94;
        8'd106: sig = 8'd95;
        8'd107: sig = 8'd97;
        8'd108: sig = 8'd98;
        8'd109: sig = 8'd99;
        8'd110: sig = 8'd101;
        8'd111: sig = 8'd102;
        8'd112: sig = 8'd104;
        8'd113: sig = 8'd105;
        8'd114: sig = 8'd107;
        8'd115: sig = 8'd108;
        8'd116: sig = 8'd110;
        8'd117: sig = 8'd111;
        8'd118: sig = 8'd113;
        8'd119: sig = 8'd114;
        8'd120: sig = 8'd116;
        8'd121: sig = 8'd117;
        8'd122: sig = 8'd119;
        8'd123: sig = 8'd120;
        8'd124: sig = 8'd122;
        8'd125: sig = 8'd123;
        8'd126: sig = 8'd125;
        8'd127: sig = 8'd126;
        8'd128: sig = 8'd128;
        8'd129: sig = 8'd129;
        8'd130: sig = 8'd130;
        8'd131: sig = 8'd132;
        8'd132: sig = 8'd133;
        8'd133: sig = 8'd135;
        8'd134: sig = 8'd136;
        8'd135: sig = 8'd138;
        8'd136: sig = 8'd139;
        8'd137: sig = 8'd141;
        8'd138: sig = 8'd142;
        8'd139: sig = 8'd144;
        8'd140: sig = 8'd145;
        8'd141: sig = 8'd147;
        8'd142: sig = 8'd148;
        8'd143: sig = 8'd150;
        8'd144: sig = 8'd151;
        8'd145: sig = 8'd153;
        8'd146: sig = 8'd154;
        8'd147: sig = 8'd156;
        8'd148: sig = 8'd157;
        8'd149: sig = 8'd158;
        8'd150: sig = 8'd160;
        8'd151: sig = 8'd161;
        8'd152: sig = 8'd163;
        8'd153: sig = 8'd164;
        8'd154: sig = 8'd165;
        8'd155: sig = 8'd167;
        8'd156: sig = 8'd168;
        8'd157: sig = 8'd169;
        8'd158: sig = 8'd171;
        8'd159: sig = 8'd172;
        8'd160: sig = 8'd173;
        8'd161: sig = 8'd175;
        8'd162: sig = 8'd176;
        8'd163: sig = 8'd177;
        8'd164: sig = 8'd179;
        8'd165: sig = 8'd180;
        8'd166: sig = 8'd181;
        8'd167: sig = 8'd182;
        8'd168: sig = 8'd183;
        8'd169: sig = 8'd185;
        8'd170: sig = 8'd186;
        8'd171: sig = 8'd187;
        8'd172: sig = 8'd188;
        8'd173: sig = 8'd189;
        8'd174: sig = 8'd191;
        8'd175: sig = 8'd192;
        8'd176: sig = 8'd193;
        8'd177: sig = 8'd194;
        8'd178: sig = 8'd195;
        8'd179: sig = 8'd196;
        8'd180: sig = 8'd197;
        8'd181: sig = 8'd198;
        8'd182: sig = 8'd199;
        8'd183: sig = 8'd200;
        8'd184: sig = 8'd201;
        8'd185: sig = 8'd202;
        8'd186: sig = 8'd203;
        8'd187: sig = 8'd204;
        8'd188: sig = 8'd205;
        8'd189: sig = 8'd206;
        8'd190: sig = 8'd207;
        8'd191: sig = 8'd208;
        8'd192: sig = 8'd209;
        8'd193: sig = 8'd210;
        8'd194: sig = 8'd211;
        8'd195: sig = 8'd211;
        8'd196: sig = 8'd212;
        8'd197: sig = 8'd213;
        8'd198: sig = 8'd214;
        8'd199: sig = 8'd215;
        8'd200: sig = 8'd216;
        8'd201: sig = 8'd216;
        8'd202: sig = 8'd217;
        8'd203: sig = 8'd218;
        8'd204: sig = 8'd219;
        8'd205: sig = 8'd219;
        8'd206: sig = 8'd220;
        8'd207: sig = 8'd221;
        8'd208: sig = 8'd221;
        8'd209: sig = 8'd222;
        8'd210: sig = 8'd223;
        8'd211: sig = 8'd223;
        8'd212: sig = 8'd224;
        8'd213: sig = 8'd225;
        8'd214: sig = 8'd225;
        8'd215: sig = 8'd226;
        8'd216: sig = 8'd227;
        8'd217: sig = 8'd227;
        8'd218: sig = 8'd228;
        8'd219: sig = 8'd228;
        8'd220: sig = 8'd229;
        8'd221: sig = 8'd229;
        8'd222: sig = 8'd230;
        8'd223: sig = 8'd231;
        8'd224: sig = 8'd231;
        8'd225: sig = 8'd232;
        8'd226: sig = 8'd232;
        8'd227: sig = 8'd233;
        8'd228: sig = 8'd233;
        8'd229: sig = 8'd234;
        8'd230: sig = 8'd234;
        8'd231: sig = 8'd234;
        8'd232: sig = 8'd235;
        8'd233: sig = 8'd235;
        8'd234: sig = 8'd236;
        8'd235: sig = 8'd236;
        8'd236: sig = 8'd237;
        8'd237: sig = 8'd237;
        8'd238: sig = 8'd237;
        8'd239: sig = 8'd238;
        8'd240: sig = 8'd238;
        8'd241: sig = 8'd239;
        8'd242: sig = 8'd239;
        8'd243: sig = 8'd239;
        8'd244: sig = 8'd240;
        8'd245: sig = 8'd240;
        8'd246: sig = 8'd240;
        8'd247: sig = 8'd241;
        8'd248: sig = 8'd241;
        8'd249: sig = 8'd241;
        8'd250: sig = 8'd242;
        8'd251: sig = 8'd242;
        8'd252: sig = 8'd242;
        8'd253: sig = 8'd243;
        8'd254: sig = 8'd243;
        8'd255: sig = 8'd243;
      endcase
    end
  endfunction

endmodule