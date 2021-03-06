#ifndef _TXUNIT_H_
#define _TXUNIT_H_

#include <systemc.h>

SC_MODULE(txUnit){
    sc_in<bool>         sysclk;
    sc_in<bool>         reset;
    sc_in<bool>         enable;
    sc_in<bool>         load;
    sc_in<sc_uint<8>>   data_in;
    sc_out<bool>        txd_send;
    sc_out<bool>        reg_empty;
    sc_out<bool>        buf_empty;

    sc_uint<8> TReg;
    sc_uint<8> TBuf;

    bool temp_reg_empty;
    bool temp_buf_empty;
    bool txd;

    int  bitCount;

    SC_CTOR(txUnit) : sysclk("sysclk"),
                      reset("reset"),
                      enable("enable"),
                      load("load"),
                      data_in("data_in"),
                      txd_send("txd"),
                      reg_empty("reg_empty"),
                      buf_empty("buf_empty"){
        SC_THREAD(tx_Unit_Thread);
    }
    
    void tx_Unit_Thread(){
        std::cout << "Starting TxUnit Thread" << std::endl;

        while(true){
            wait(sysclk -> posedge_event());
            if(reset -> read() == true){
                bitCount = 0;
                temp_reg_empty = true;
                temp_buf_empty = true;
                reg_empty -> write(temp_reg_empty);
                buf_empty -> write(temp_buf_empty);
                txd = true;
                txd_send -> write(txd);
            } else {
                if(load -> read() == true){
                    TBuf = data_in -> read();
                    temp_buf_empty = false;
                    buf_empty -> write(temp_buf_empty);
                    // std::cout << sc_time_stamp() << "temp Reg Empty = " << temp_reg_empty <<std::endl;
                    // std::cout << sc_time_stamp() << "temp Buf Empty = " << temp_buf_empty <<std::endl;
                }
                else if(enable -> read() == true){
                    if((temp_reg_empty == true) && (temp_buf_empty == false)){
                        TReg = TBuf;
                        temp_reg_empty = false;
                        temp_buf_empty = true;
                        reg_empty -> write(temp_reg_empty);
                        buf_empty -> write(temp_buf_empty);
                        // std::cout << sc_time_stamp() << "TReg = " << TReg <<std::endl;
                    }

                    if(temp_reg_empty == false){
                        switch(bitCount){
                            case 0 :
                                txd = false;
                                // std::cout << sc_time_stamp() << "bitCount = "<< bitCount <<", TXD = " << txd << std::endl;
                                bitCount++;
                            break;

                            case  1 ... 8:
                                txd = TReg & 0x01 ;
                                // std::cout << sc_time_stamp() << " : bitCount " << bitCount << ", TXD = " << txd << std::endl;
                                TReg = TReg >> 1;
                                bitCount++;
                            break;

                            case 9 :
                                txd = true;
                                TReg = TReg >> 1;
                                // std::cout << sc_time_stamp() << " : bitCount = "<< bitCount <<", TXD = " << txd << std::endl;
                                bitCount = 0;
                                temp_reg_empty = true;
                                reg_empty -> write(temp_reg_empty);
                            break;

                            default : 
                            break;
                        }
                        txd_send -> write(txd);
                    }
                }
            }
        }
    }
};

#endif