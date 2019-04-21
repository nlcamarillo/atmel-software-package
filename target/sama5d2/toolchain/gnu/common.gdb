# Connect to the J-Link gdb server
define jlink_connect
  target remote localhost:3333
end

define reset_peripherals
  # Disable all interrupts and go to supervisor mode
  mon reg cpsr = 0xd3

  # Reset the chip to get to a known state
  #monitor reset

  # Reset peripherals (using RSTC_CR)
  set *0xF8048000 = 0xA5000004

  # Reset L2 Cache controller
  set *0x00A00100 = 0x0

  # Disable Watchdog (using WDT_MR)
  set *0xF8048044 = 0x00008000

  # Disable D-Cache, I-Cache and MMU
  mon cp15 1 0 0 0 = 0x00C50078
end

# Disable DDR clock and MPDDRC controller
# to avoid corrupted RAM data on soft reset.
define disable_ddr
  set *0xF0014004 = 0x4
  set *0xF0014014 = (1 << 13)  
end

define reset_registers
  # Zero registers (cannot reset core because it will disable JTAG)
  mon reg r8_fiq = 0
  mon reg r9_fiq = 0
  mon reg r10_fiq = 0
  mon reg r11_fiq = 0
  mon reg r12_fiq = 0
  mon reg sp_fiq = 0
  mon reg lr_fiq = 0
  mon reg spsr_fiq = 0
  mon reg sp_irq = 0
  mon reg lr_irq = 0
  mon reg spsr_irq = 0
  mon reg sp_abt = 0
  mon reg lr_abt = 0
  mon reg spsr_abt = 0
  mon reg sp_und = 0
  mon reg lr_und = 0
  mon reg spsr_und = 0
  mon reg sp_svc = 0
  mon reg lr_svc = 0
  mon reg spsr_svc = 0
  mon reg r0 = 0
  mon reg r1 = 0
  mon reg r2 = 0
  mon reg r3 = 0
  mon reg r4 = 0
  mon reg r5 = 0
  mon reg r6 = 0
  mon reg r7 = 0
  mon reg r8_usr = 0
  mon reg r9_usr = 0
  mon reg r10_usr = 0
  mon reg r11_usr = 0
  mon reg r12_usr = 0
  mon reg sp_usr = 0
  mon reg lr_usr = 0  
end

define init_ddr  

  reset_registers
  
  load target/bootstrap.elf
  
  # Initialize PC
  mon reg pc = 0x00200000
  mon reg pc = 0x00200000

  continue
end

define set_bureg_qspi1
  reset_registers

  # set Boot Sequence Controller Configuration Register to 100 to boot from bureg 0 (section 16.5.2)
  set *0xF8048054 = 0x66830004
  
  # set Backup Registers (BUREG), number 0 (see section 16.5.3 and 16.5.4)
  # 0 : secure mode off (default)
  # 0 : launch sam-ba if boot not found (default)
  # 0 : read from BUREG and qspi in qspi mode (default) 2 for qspi in xip mode
  # 4 : external memory enabled, use jtag ioset 0 (0100)
  # F : UART off (1111) (5 for uart2 ioset 3)
  # F : sdmmc off, nand flash off (1111) (8 for sdmmc 1)
  # F : spi 0 and 1 0ff (1111)
  # 7 : qspi1 ioset 2, qspi 0 off
  # set *0xF8045400 = 0x0024FFF7
  set *0xF8045400 = 0x00045FF7
end

define load_in_qspi1
  
  #reset_registers

  load
  
  # Initialize PC to QSPI1 mem
  # mon reg pc = 0xD8000000 # xip on
  mon reg pc = 0x00200000   # xip off
end

define load_in_ddr

  reset_registers

  load
  
  # Initialize PC
  mon reg pc = 0x20000000
end

define load_in_sram
  reset_registers

  load

  # Initialize PC
  mon reg pc = 0x00200000
end
