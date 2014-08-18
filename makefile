include sources.mk
# try inclue suprproject config file
# '-' if do not exist don't give error
-include ..\build\config.mk

TARGET = libfatfs

ifneq ($(HAS_CONFIG),true)
# if we don't have config file we try to run standalone
CC         := arm-none-eabi-gcc
CC-OBJDUMP := arm-none-eabi-objdump
CC-SIZE    := arm-none-eabi-size
CC-AR      := arm-none-eabi-ar
OBJDIR 	   := ..\objects
LIBDIR	   := ..\libs
else
OBJDIR 	   := ..\$(OBJDIR)
LIBDIR	   := ..\$(LIBDIR)
endif

# add .. to source and obj directory (are outside build directory)
SRCDIR     := .

# optimize string elaboration
ifeq ($(OPTIMIZE),none)
OPTIMIZE := 0
endif
ifeq ($(OPTIMIZE),size)
OPTIMIZE := s
endif 

OPTIMIZE := $(addprefix -O,$(OPTIMIZE))
 

CFLAGS := $(OPTIMIZE) $(DEFINE) -Wall -std=gnu99 -mcpu=cortex-m3 -ffunction-sections -mthumb -g3
# include path
#IPATH  := -I$(SRCDIR) -I$(SRCDIR)\include
IPATH   := $(SRCDIR)

#include for ST driver
IPATH   += $(SRCDIR)\FWLIB\cmsis\interface
IPATH   += $(SRCDIR)\FWLIB\driver\interface
IPATH   += $(SRCDIR)\USER\interface

#generate objects with objdir preponed 
OBJECTS := $(addprefix $(OBJDIR)\,$(SRCS:.c=.o))
IPATH   := $(addprefix -I,$(IPATH))
OUTPUT  := $(LIBDIR)\$(TARGET)

.PHONY: all
all: $(OUTPUT).a 

.PHONY: clean
clean:
	del $(OBJECTS) 2>nul
	del $(OBJECTS:.o=.o.lst) 2>nul
	del $(OBJECTS:.o=.d) 2>nul
	del $(OUTPUT).elf 2>nul
	del $(OUTPUT).map 2>nul

# library comand
$(OUTPUT).a: $(OBJECTS)
	@if not exist $(dir $@) mkdir $(dir $@)
	$(CC-AR) rcs $(OUTPUT).a $(OBJECTS) 

# compile comand	
# With '-MD' we generate dependancies. Dependancies file are the same *.o file (same name, same path) but with *.d extension
$(OBJECTS): $(OBJDIR)\\%.o: $(SRCDIR)\\%.c
	@if not exist $(dir $@) mkdir $(dir $@)
	$(CC) $(CFLAGS) $(IPATH) -MD -c $< -Wa,-ahls=$@.lst -o $@ 
	

# Include depenencies. Include directive with '-' so don't generate error if some file is missing	
-include $(OBJECTS:.o=.d)

	
