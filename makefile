LDFLAGS=-lcrypto
xts: xts.o
	g++ -o $@ $< $(LDFLAGS)
