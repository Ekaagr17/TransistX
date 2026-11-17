CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -Iinclude
LIBS = -lws2_32 -lm

SRCS = src/cJSON.c \
       src/haversine.c \
       src/data_store.c \
       src/eta_engine.c \
       src/journey_planner.c \
       src/websocket.c \
       src/http_server.c \
       src/main.c

OBJS = $(SRCS:.c=.o)
TARGET = transitx_c.exe

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q /S src\*.o 2>nul
	del /Q $(TARGET) 2>nul
