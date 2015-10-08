CC = gcc
LD = gcc

CFLAGS = -O3 -Wall -fPIC
LDFLAGS = -shared -llua

TARGET = iso8583.so

OBJS = iso8583.o luaiso8583.o

$(TARGET):$(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

$(OBJS):%.o:%.c
	$(CC) -c $< $(CFLAGS)

.PHONY:clean

clean:
	-rm -rf $(TARGET) $(OBJS)

