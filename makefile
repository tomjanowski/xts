LDFLAGS=-lcrypto -ldl
all: xts xts_static
xts: xts.o
	g++ -o $@ $< $(LDFLAGS)
xts_static: xts.o
	g++ -O0 -static -g -o $@ $< $(LDFLAGS) -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
