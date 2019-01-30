CC=g++
CFLAGS=-g -std=c++11 

all: test

test: lrcn_host.o fpga_version.o
	$(CC) $(CFLAGS) lrcn_host.o fpga_version.o -I include -o test 

lrcn_host.o: lrcn_host.cpp fpga_version.h
	$(CC) $(CFLAGS) -c lrcn_host.cpp -I include

fpga_version.o: fpga_version.cpp fpga_version.h
	$(CC) $(CFLAGS) -c fpga_version.cpp -I include
clean:
	rm -f lrcn_host.o fpga_version.o test
