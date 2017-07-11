CC = $(CROSS_COMPILE)cc

CFLAGS  = -g -Wall

TARGET = sensor_test

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET)
