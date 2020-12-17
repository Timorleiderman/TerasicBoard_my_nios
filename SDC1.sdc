create_clock -name CLOCK2_50 -period 10.000 [get_ports {CLOCK2_50}] 
derive_pll_clocks  
derive_clock_uncertainty 