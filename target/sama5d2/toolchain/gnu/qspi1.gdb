source target/common.gdb

# define 'reset' command
define reset

  # Connect to the J-Link gdb server
  jlink_connect

  mon halt
  
  reset_peripherals

  disable_ddr

  set_bureg_qspi1
  load_in_qspi1
end
