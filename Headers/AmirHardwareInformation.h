#pragma once
#include "AmirLog.h"//AmirString
#include <string>
#include <mutex>
#include <condition_variable>

class OutPutEvent{
public:
    virtual void onOupPut();
};
class AmirGetSystemInfo{
public:
    std::string getCommandOutput(const char* cmd);
    std::string getHDDSerial();
    std::string getBiosSerial();
    std::string getMacAddress();
    std::string getCpuID();
private:
};
//=======================================================================
