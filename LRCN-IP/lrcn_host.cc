#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <time.h>

#include <chrono> // for high_resolution_clock
#include <ap_fixed.h>
#define DATA_ADDR 0x0000000000000000
#define GPIO_IN_ADDR 0x40001000
#define GPIO_OUT_ADDR 0x40000000

#define MAGIC_CLR 0x00
#define MAGIC_START 0xAA
#define MAGIC_DONE 0x03


int main()
{

    auto t_start = std::chrono::high_resolution_clock::now();    

    ap_int <512> *reorder_data = new ap_int <512>[301050+125]; //total offset 2229123
    // Read data from binary file
	char* dest  = new char[sizeof(reorder_data)];
	std::ifstream ifs;
	ifs.open("reorderdata_for_maxDSP_small_diffQ.bin", std::ios::binary | std::ios::in);
	ifs.read((char*) reorder_data, sizeof(ap_int<512>)*301050);
	ifs.close();

    //initialize LRCN
    printf("initialize LRCN\n");

    printf("TB pass pointer from addr. \n");

    auto t_1 = std::chrono::high_resolution_clock::now();



    int from_fpga_fd = open("/dev/xdma0_c2h_0", O_RDWR | O_NONBLOCK);
    int to_fpga_fd= open("/dev/xdma0_h2c_0", O_RDWR);
    if(from_fpga_fd<0 || to_fpga_fd<0) {
        puts("device not found");
        exit(0);
    }
    printf("write data\n");
    lseek(to_fpga_fd, DATA_ADDR, SEEK_SET);
    write(to_fpga_fd, reorder_data, (301050+125) * sizeof(ap_int <512>));
    printf("finished writing\n");

    auto t_2 = std::chrono::high_resolution_clock::now();


    unsigned char ch=MAGIC_START;
    unsigned char ch_0 = 0;


    lseek(to_fpga_fd, GPIO_OUT_ADDR, SEEK_SET); 
    write(to_fpga_fd, &ch, sizeof(unsigned char));

    lseek(to_fpga_fd, GPIO_OUT_ADDR, SEEK_SET);
    write(to_fpga_fd, &ch_0, sizeof(unsigned char));
    while( (ch&0x03) !=0x03)
    {

      // printf("CH: %d\n", ch);
        lseek(from_fpga_fd, GPIO_IN_ADDR, SEEK_SET);    
        read(from_fpga_fd, &ch, sizeof(unsigned char));
    
    } 
    printf("%x\n", ch); 
    puts("end!");


	
    auto t_3 = std::chrono::high_resolution_clock::now();

    ap_int <512> data_out[1];
    puts("test after hls");
    lseek(from_fpga_fd,  DATA_ADDR+sizeof(ap_int<512>)*301050, SEEK_SET);
    read(from_fpga_fd, &data_out,  512/32* sizeof(int));
    
    ap_int<32> num;

    int c;
    FILE* tb_file;
    tb_file=fopen("tbtestout.txt","w");

           for(int j=0;j<15;j++)
           {
               num.range(31,0)=data_out[0].range(j*32+31,j*32);
               int temp=num;
               fprintf(tb_file,"%d\n",temp);
           }

        fclose(tb_file);
    

    std::chrono::duration<double> data_prepare = t_1 - t_start; 
    std::chrono::duration<double> pcie_write = t_2 - t_1; 
    std::chrono::duration<double> kernel_exec = t_3 - t_2; 
    std::cout << "Data Preparation: " << data_prepare.count() << "s" << std::endl;
    std::cout << "PCIE Write  time: " << pcie_write.count() << "s" << std::endl;
    std::cout << "Kernel exec time: " << kernel_exec.count() << "s" << std::endl;



    return 0;
}
