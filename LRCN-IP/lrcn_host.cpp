#include "fpga_version.h"


int main()
{

    auto t_start = std::chrono::high_resolution_clock::now();    

    
    Predictor* pred = Newfpga("reorderdata_for_maxDSP_small_diffQ.bin");

    auto t_2 = std::chrono::high_resolution_clock::now();
    
    const int * idx = Predictfpga(pred);


    auto t_3 = std::chrono::high_resolution_clock::now();
    Deletefpga(pred);
   
   for(int j=0;j<15;j++)
   {
       printf("%d\n",idx[j]);
   }
   delete idx;
    
    std::chrono::duration<double> data_prepare = t_2 - t_start; 
    std::chrono::duration<double> kernel_exec = t_3 - t_2; 
    std::cout << "Preprocessing: " << data_prepare.count() << "s" << std::endl;
    std::cout << "Kernel exec time: " << kernel_exec.count() << "s" << std::endl;

    return 0;
}
