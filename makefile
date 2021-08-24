LDFLAGS=-lcrypto
all: xts.o
	g++ -o xts $< $(LDFLAGS)
