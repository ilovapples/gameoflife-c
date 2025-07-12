BUILD		:= build
OBJS		:= obj
SRCS		:= src

LIFE_TARGET	:= life
CST_TARGET	:= cst

LIFE_SRCS	:= $(SRCS)/$(LIFE_TARGET)
CST_SRCS	:= $(SRCS)/$(CST_TARGET)

INCLUDEDIR	:= $(SRCS)/include

OBJ_EXT	:= .o
TARGET_EXT	:= 

INCLUDE 	:= -I$(INCLUDEDIR)
CFLAGS	:= -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -pedantic -Werror $(INCLUDE)
ifeq ($(OS),Windows_NT)
	OBJ_EXT = .obj
	TARGET_EXT = .exe
endif

all: $(BUILD)/$(LIFE_TARGET)$(TARGET_EXT) $(BUILD)/$(CST_TARGET)$(TARGET_EXT)


$(BUILD)/$(LIFE_TARGET)$(TARGET_EXT): $(OBJS)/life$(OBJ_EXT) $(OBJS)/border$(OBJ_EXT)
	gcc -o $(BUILD)/$(LIFE_TARGET)$(TARGET_EXT) $(OBJS)/life$(OBJ_EXT) $(OBJS)/border$(OBJ_EXT)


$(OBJS)/life$(OBJ_EXT): $(LIFE_SRCS)/life.c $(INCLUDEDIR)/border.h $(INCLUDEDIR)/border_chars.h
	gcc -o $(OBJS)/life$(OBJ_EXT) -c $(LIFE_SRCS)/life.c $(CFLAGS)

$(OBJS)/border$(OBJ_EXT): $(LIFE_SRCS)/border.c $(INCLUDEDIR)/border.h
	gcc -o $(OBJS)/border$(OBJ_EXT) -c $(LIFE_SRCS)/border.c $(CFLAGS)


$(BUILD)/$(CST_TARGET)$(TARGET_EXT): $(OBJS)/cellstate$(OBJ_EXT)
	gcc -o $(BUILD)/$(CST_TARGET)$(TARGET_EXT) $(OBJS)/cellstate$(OBJ_EXT)

$(OBJS)/cellstate$(OBJ_EXT): $(CST_SRCS)/cellstate.c
	gcc -o $(OBJS)/cellstate$(OBJ_EXT) -c $(CST_SRCS)/cellstate.c $(CFLAGS)

