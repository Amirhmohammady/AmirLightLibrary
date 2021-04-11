#pragma once
#include <exception>
#include <cstring>

class AmirException:public std::exception{
public:
    AmirException(std::string msg, int errcode = 0){
        this->errcode = errcode;
        this->msg = msg;
    }
    const char* what() const noexcept{
        return msg.c_str();
    }
    int getErrCode(){
        return errcode;
    }
private:
    std::string msg;
    int errcode;
};
