#pragma once
#include <mutex>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>

#define _AMIRLOG(obj, msgtype, msg) {if (msgtype != _OFF && obj.loglevel >= msgtype) obj._log(msgtype, msg, __func__, __LINE__, __FILE__);}
//#define amirLog(msgtype, msg) _log(msgtype, msg, __func__, __LINE__, __FILE__)

enum LogType
{
    LOG_TO_FILE,LOG_TO_CONSOLE
};
//===========================================================
enum LogLevel
{
    _OFF, _TEST, _FATAL, _ERROR, _IMPORTANT, _DEBUG, _INFO
};
//===========================================================
class AmirString;
//===========================================================
class AmirLog
{
public:
    AmirLog(AmirLog const&) = delete;
    void operator=(AmirLog const&)  = delete;
    //WARNING!!! Don't Use This Function Directly.
    //Use _AMIRLOG macro as defined top of the header.
    void _log(LogLevel msglevel, const AmirString &msg, const char *functionname,
                     int linenumber, const char *filename);
    void setLogFileSize(unsigned int filesizeKB);
    void setLogFileName(const std::string &logfilename);
    bool logDateTimeLine = true;
    LogLevel loglevel = _ERROR;
    LogType logtype = LOG_TO_FILE;;
    unsigned int filesizeKB = 50;
    static AmirLog& getInstance(const std::string log_group);
private:
    //---------------------------single ton----------------------------
    AmirLog(){}
    AmirLog(AmirLog &);
    void operator  = (AmirLog&);
    //-------------------------------------------------------
    static bool isFileExist(const std::string &name);
    void createFolder();
    bool isFileSizeLarger();
    bool openFile();
    void closeFile();
    static const std::string toString(LogLevel msglevel);
    void logToFile(LogLevel msglevel, const std::string &msg, const char *functionname,
                          int linenumber, const char *filename);
    void logToConsole(LogLevel msglevel, const std::string &msg, const char *functionname,
                             int linenumber, const char *filename);
    static std::vector<std::pair <std::string,AmirLog*> > group_instance;
    std::string logfilename;
    unsigned int fileindex = 0;
    unsigned int instanceID;
    static std::mutex mtx;
    std::ofstream logfile;
};
//===========================================================
//===========================================================
class AmirString:public std::string
{
public:
    AmirString() {}
    template<typename tmpl>AmirString (tmpl s1);
    template<typename tmpl>void operator = (tmpl &s1);
    template<typename tmpl>AmirString& operator << (tmpl s1);
    template<typename tmpl>AmirString operator + (tmpl s1);
    template<typename tmpl>AmirString& operator >> (tmpl &s1);
    template<typename tmpl>void operator |= (tmpl &s1);
    void toUpperCase();
    void toLowerCase();
private:
    //std::string value;
};
//===========================================================
template<typename tmpl> AmirString::AmirString(tmpl s1):std::string{}
{
    std::stringstream ss;
    ss<<s1;
    std::string::operator = (ss.str());
}
//===========================================================
template<typename tmpl> void AmirString::operator=(tmpl &s1)
{
    std::stringstream ss;
    ss<<s1;
    std::string::operator = (ss.str());
}
//===========================================================
template<typename tmpl> AmirString& AmirString::operator << (tmpl s1){
    std::stringstream ss;
    ss<<s1;
    std::string::operator += (ss.str());
    return *this;
}
//===========================================================
template<typename tmpl> AmirString AmirString::operator + (tmpl s1){
    AmirString as;
    as = *this;
    as<<s1;
    return as;
}
//===========================================================
template<typename tmpl>AmirString& AmirString::operator >> (tmpl &s1){
    std::stringstream ss;//WARNING DON'T USE ss(*this) BEACUASE THE POSITION WILL NOT SET AND tellp RETURN ZERO!
    ss<<*this;
    ss>>s1;
    if (static_cast<size_type>(ss.tellg()) != std::string::npos && static_cast<size_type>(ss.tellp()) != std::string::npos)
        std::string::operator = (ss.str().substr(ss.tellg(),ss.tellp()-ss.tellg()));
    else std::string::operator = ("");
    return *this;
}
//===========================================================
template<typename tmpl>void AmirString::operator |= (tmpl &s1){
    std::stringstream ss;//WARNING DON'T USE ss(*this) BEACUASE THE POSITION WILL NOT SET AND tellp RETURN ZERO!
    ss<<*this;
    ss>>s1;
}
//===========================================================







