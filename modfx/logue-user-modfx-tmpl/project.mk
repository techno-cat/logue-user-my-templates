# #############################################################################
# Project Customization
# #############################################################################

PROJECT = modfx_tmpl

UCSRC = $(wildcard ../user/lib/*.c)

UCXXSRC = ../user/modfx.cpp

# NOTE: Relative path from `Makefile` that refer this file.
UINCDIR = ../user/lib

UDEFS =

ULIB =

ULIBDIR =
