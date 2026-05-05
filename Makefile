CC = gcc

CFLAGS = -std=gnu99
#CFLAGS += -D_DEBUG

TARGET = I2cMasterController

SRCS = I2cMasterController.c
SRCS += I2CTrasnsporter.c

OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -c $(SRCS)

clean:
	rm $(OBJS) $(TARGET) 
