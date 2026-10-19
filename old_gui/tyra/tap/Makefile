TARGET      := tap.elf
ENGINEDIR	:= ../engine

#The Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := src
INCDIR      := inc
BUILDDIR    := obj
TARGETDIR   := bin
RESDIR      := res
SRCEXT      := cpp
VSMEXT		:= vsm
VCLEXT		:= vcl
VCLPPEXT	:= vclpp
DEPEXT      := d
OBJEXT      := o

#Flags, Libraries and Includes
CFLAGS      :=
LIB         := -ltyra -lmc -lnetman -lps2ip
LIBDIRS     := -L$(ENGINEDIR)/bin
INC         := -I$(INCDIR) -I$(ENGINEDIR)/inc
# INCDEP      := -I$(INCDIR) -I$(ENGINEDIR)/inc
INCDEP 		:= -I$(INCDIR) -I$(ENGINEDIR)/inc -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include

include ../Makefile.base

clean-engine:
	cd $(ENGINEDIR) && $(MAKE) cleaner

build-engine:
	cd $(ENGINEDIR) && $(MAKE)

build-release-engine:
	cd $(ENGINEDIR) && $(MAKE) release

ps2dev9_irx.cpp: $(PS2SDK)/iop/irx/ps2dev9.irx
	cd src/ && bin2c $< $@ ps2dev9_irx
netman_irx.cpp: $(PS2SDK)/iop/irx/netman.irx
	cd src/ && bin2c $< $@ netman_irx
smap_irx.cpp: $(PS2SDK)/iop/irx/smap.irx
	cd src/ && bin2c $< $@ smap_irx
