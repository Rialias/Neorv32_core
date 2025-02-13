
# Include the nanopb provided Makefile rules
include /home/ralias/nanopb/extra/nanopb.mk

# Override the default CPU ISA
MARCH = rv32i_zicsr_zifencei

# Override the default RISC-V GCC prefix
#RISCV_PREFIX ?= riscv-none-elf-

# Override default optimization goal
EFFORT = -Os

# Add extended debug symbols
USER_FLAGS += -ggdb -gdwarf-3

# Adjust processor IMEM size
USER_FLAGS += -Wl,--defsym,__neorv32_rom_size=16k

# Adjust processor DMEM size
USER_FLAGS += -Wl,--defsym,__neorv32_ram_size=8k

# Adjust maximum heap size
#USER_FLAGS += -Wl,--defsym,__neorv32_heap_size=1k

# Additional sources
#APP_SRC += main.pb.h
APP_SRC += pb_common.c
#APP_SRC += pb_common.h
APP_SRC += pb_decode.c
#APP_SRC += pb_decode.h
APP_SRC += pb_encode.c
#APP_SRC += pb_encode.h
APP_SRC += server.c
APP_INC += -I .

# Set path to NEORV32 root directory
NEORV32_HOME ?= ../../..

# Include the main NEORV32 makefile
include $(NEORV32_HOME)/sw/common/common.mk