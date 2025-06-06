PLUGIN_NAME = rtpssrcptdemux
TARGET = libgst$(PLUGIN_NAME).so

SRCS = src/RTPssrcptDemux.c src/utils.c
OBJS = $(SRCS:.c=.o)

CC = gcc
CFLAGS = -Wall -fPIC $(shell pkg-config --cflags gstreamer-1.0 gstreamer-base-1.0 gstreamer-rtp-1.0)
LDFLAGS = -shared $(shell pkg-config --libs gstreamer-1.0 gstreamer-base-1.0 gstreamer-rtp-1.0)

PREFIX ?= /usr
PLUGINDIR = $(PREFIX)/lib/x86_64-linux-gnu/gstreamer-1.0

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

install: $(TARGET)
	rm $(PLUGINDIR)/$(TARGET)
	mkdir -p $(PLUGINDIR)
	cp $(TARGET) $(PLUGINDIR)/

uninstall:
	rm -f $(PLUGINDIR)/$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
