#define DATA_ADDR 0x0000000000000000
#define GPIO_IN_ADDR 0x40001000
#define GPIO_OUT_ADDR 0x40000000

#define MAGIC_CLR 0x00
#define MAGIC_START 0xAA
#define MAGIC_DONE 0x03
#include "fpga_version.h"


/*class Predictor {
public:
  int from_fpga_fd_;
  int to_fpga_fd_;
  Predictor(int from_fpga_fd, int to_fpga_fd){
	int from_fpga_fd_ = from_fpga_fd;
	int to_fpga_fd_ = to_fpga_fd;
  };
  ~Predictor() {
  }
};*/

Predictor* Newfpga(const char input_file[]) {
  
    ap_int <512> *reorder_data = new ap_int <512>[301050+125]; //total offset 2229123
    // Read data from binary file
  std::ifstream ifs;
  ifs.open(input_file, std::ios::binary | std::ios::in);
  ifs.read((char*) reorder_data, sizeof(ap_int<512>)*301050);
  ifs.close();

    //initialize LRCN
    printf("initialize LRCN\n");

    printf("TB pass pointer from addr. \n");

    int from_fpga_fd = open("/dev/xdma0_c2h_0", O_RDWR | O_NONBLOCK);
    int to_fpga_fd= open("/dev/xdma0_h2c_0", O_RDWR);
    if(from_fpga_fd<0 || to_fpga_fd<0) {
        puts("device not found");
        exit(0);
        return nullptr;
    }
    printf("write data\n");
    lseek(to_fpga_fd, DATA_ADDR, SEEK_SET);
    write(to_fpga_fd, reorder_data, (301050+125) * sizeof(ap_int <512>));
    printf("finished writing\n");
    Predictor* pred = new Predictor(from_fpga_fd,to_fpga_fd);
    //delete (char*)reorder_data;
    return (Predictor*) pred;

}

void Deletefpga(Predictor* pred) {
  auto predictor = (Predictor *)pred;
  if (predictor == nullptr) {
    return;
  }
  delete predictor;
}

const int *Predictfpga(Predictor* pred) {

  auto predictor = (Predictor *)pred;

  if (predictor == nullptr) {
    std::cerr << "fpga prediction error on " << __LINE__
              << " :: null predictor\n";
    return nullptr;
  }
    char ch = MAGIC_START;
    char ch_0 = MAGIC_CLR;
    lseek(predictor->to_fpga_fd_, GPIO_OUT_ADDR, SEEK_SET); 
    write(predictor->to_fpga_fd_, &ch, sizeof(unsigned char));

    lseek(predictor->to_fpga_fd_, GPIO_OUT_ADDR, SEEK_SET);
    write(predictor->to_fpga_fd_, &ch_0, sizeof(unsigned char));
    while( (ch&0x03) !=0x03)
    {

      // printf("CH: %d\n", ch);
        lseek(predictor->from_fpga_fd_, GPIO_IN_ADDR, SEEK_SET);    
        read(predictor->from_fpga_fd_, &ch, sizeof(unsigned char));
    
    } 
    puts("end!");


    ap_int <512> data_out[1];
    puts("test after hls");
    lseek(predictor->from_fpga_fd_,  DATA_ADDR+sizeof(ap_int<512>)*301050, SEEK_SET);
    read(predictor->from_fpga_fd_, &data_out,  512/32* sizeof(int));
    
    ap_int<32> num;
    int idx;
    int* output = new int[16];
    for(int j=0;j<15;j++)
    {
       num.range(31,0)=data_out[0].range(j*32+31,j*32);
       output[j]= num;
       
    }
    

    return output;
  
}
