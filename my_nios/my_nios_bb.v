
module my_nios (
	clk_clk,
	pio_0_external_connection_export,
	pio_interrupt_external_connection_export,
	reset_reset_n);	

	input		clk_clk;
	output	[7:0]	pio_0_external_connection_export;
	input		pio_interrupt_external_connection_export;
	input		reset_reset_n;
endmodule
