BUILD		:= build
OBJS		:= obj
SRCS		:= src

LIFE_TARGET	:= life
CST_TARGET	:= cst

LIFE_SRCS	:= $(SRCS)/$(LIFE_TARGET)
CST_SRCS	:= $(SRCS)/$(CST_TARGET)

INCLUDEDIR	:= include

OBJ_EXT	:= .o
TARGET_EXT	:=

INCLUDE 	:= -I$(INCLUDEDIR)
CFLAGS	:= -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -pedantic -Wshadow $(INCLUDE)
ifeq ($(OS),Windows_NT)
	OBJ_EXT = .obj
	TARGET_EXT = .exe
endif

all: $(OBJS) $(BUILD) $(BUILD)/$(LIFE_TARGET)$(TARGET_EXT) $(BUILD)/$(CST_TARGET)$(TARGET_EXT)

clean:
	rm build/* obj/*

# LIFE BUILD RULES

$(BUILD)/$(LIFE_TARGET)$(TARGET_EXT): $(OBJS)/life$(OBJ_EXT) $(OBJS)/border$(OBJ_EXT) $(OBJS)/viewing$(OBJ_EXT) $(OBJS)/arg_parse$(OBJ_EXT) $(OBJS)/border_chars$(OBJ_EXT)
	gcc -o $(BUILD)/$(LIFE_TARGET)$(TARGET_EXT) $(OBJS)/life$(OBJ_EXT) $(OBJS)/border$(OBJ_EXT) $(OBJS)/viewing$(OBJ_EXT) $(OBJS)/arg_parse$(OBJ_EXT) $(OBJS)/border_chars$(OBJ_EXT)

$(OBJS)/life$(OBJ_EXT): $(LIFE_SRCS)/main.c $(LIFE_SRCS)/border.h $(LIFE_SRCS)/viewing.h $(LIFE_SRCS)/arg_parse.h $(LIFE_SRCS)/big_header.h $(INCLUDEDIR)/runtime_flags.h 
	gcc -o $(OBJS)/life$(OBJ_EXT) -c $(LIFE_SRCS)/main.c $(CFLAGS)

$(OBJS)/border$(OBJ_EXT): $(LIFE_SRCS)/border.c $(LIFE_SRCS)/border.h
	gcc -o $(OBJS)/border$(OBJ_EXT) -c $(LIFE_SRCS)/border.c $(CFLAGS)

$(OBJS)/border_chars$(OBJ_EXT): $(LIFE_SRCS)/border_chars.c
	gcc -o $(OBJS)/border_chars$(OBJ_EXT) -c $(LIFE_SRCS)/border_chars.c $(CFLAGS)

$(OBJS)/viewing$(OBJ_EXT): $(LIFE_SRCS)/viewing.c $(LIFE_SRCS)/viewing.h $(LIFE_SRCS)/border.h $(LIFE_SRCS)/big_header.h $(INCLUDEDIR)/runtime_flags.h
	gcc -o $(OBJS)/viewing$(OBJ_EXT) -c $(LIFE_SRCS)/viewing.c $(CFLAGS)

$(OBJS)/arg_parse$(OBJ_EXT): $(LIFE_SRCS)/arg_parse.c $(LIFE_SRCS)/arg_parse.h $(LIFE_SRCS)/big_header.h
	gcc -o $(OBJS)/arg_parse$(OBJ_EXT) -c $(LIFE_SRCS)/arg_parse.c $(CFLAGS)


# CST BUILD RULES

$(BUILD)/$(CST_TARGET)$(TARGET_EXT): $(OBJS)/cellstate$(OBJ_EXT)
	gcc -o $(BUILD)/$(CST_TARGET)$(TARGET_EXT) $(OBJS)/cellstate$(OBJ_EXT)

$(OBJS)/cellstate$(OBJ_EXT): $(CST_SRCS)/cellstate.c
	gcc -o $(OBJS)/cellstate$(OBJ_EXT) -c $(CST_SRCS)/cellstate.c $(CFLAGS)


$(OBJS):
	mkdir -p $(OBJS)
$(BUILD):
	mkdir -p $(BUILD)
