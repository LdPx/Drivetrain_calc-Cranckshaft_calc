CC = $(CROSS_COMPILE)cc

CFLAGS  = -g -Wall
LDFLAGS = -lrt

TARGET = drive

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET)
