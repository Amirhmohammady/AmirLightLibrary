#include "../Headers/AmirLog.h"
#include <iostream>
#include <unistd.h>
#include <windows.h>//CreateDirectory

using namespace std;

std::mutex AmirLog::mtx;
vector<pair<string,AmirLog*>> AmirLog::group_instance;
//===========================================================
void AmirLog::_log(LogLevel msglevel, const AmirString &msg, const char *functionname,
                   int linenumber, const char *filename)
{
    mtx.lock();
    //this if check in _AMIRLOG macro. this if is necessary for amirLog macro.
    //if (msglevel <= loglevel && msglevel>_OFF)
    {
        switch(logtype)
        {
        case LOG_TO_CONSOLE:
            logToConsole(msglevel, msg, functionname, linenumber, filename);
            break;
        case LOG_TO_FILE:
            logToFile(msglevel, msg, functionname, linenumber, filename);
            break;
        }
    }
    mtx.unlock();
}
//===========================================================
bool AmirLog::openFile()
{
    logfile.open(logfilename,ios::app);
    if (logfile.is_open()) return true;
    else return false;
}
//===========================================================
void AmirLog::closeFile()
{
    if (logfile.is_open()) logfile.close();
}
//===========================================================
void AmirLog::setLogFileName(const string &logfilename){
    this->logfilename = logfilename;
    createFolder();
}
//===========================================================
void AmirLog::createFolder(){
    unsigned int tmp;
    tmp = logfilename.find_last_of('\\');
    if (tmp!=string::npos){
        string foldername = logfilename.substr(0,tmp);
        CreateDirectory(foldername.c_str(), NULL);
    }
}
//===========================================================
void AmirLog::logToFile(LogLevel msglevel, const string &msg, const char *functionname,
                        int linenumber, const char *filename)
{
    if (isFileSizeLarger())
    {
        AmirString indexfile;
        do
        {
            indexfile = logfilename;
            indexfile.erase(indexfile.begin()+indexfile.find_last_of('.'),indexfile.end());
            indexfile<<fileindex<<".log";
            fileindex++;
        }
        while(isFileExist(indexfile));
        rename(logfilename.c_str(), indexfile.c_str());
    }
    openFile();
    if (logDateTimeLine)
    {
        logfile<<"("<<filename<<") \t("<<functionname<<") \t("<<linenumber<<")"<<endl;
        logfile<<'('<<__DATE__<<") \t("<<__TIME__<<") \t("<<toString(msglevel)<<')'<<endl;
    }
    logfile<<msg<<endl;
    //logfile<<"--------------------------------------------------------------------"<<endl;
    closeFile();
}
//===========================================================
void AmirLog::logToConsole(LogLevel msglevel, const string &msg, const char *functionname,
                           int linenumber, const char *filename)
{
    if (logDateTimeLine)
    {
        cout<<"file: \t"<<filename<<" function: \t"<<functionname<<" line: \t"<<linenumber<<endl;
        cout<<__DATE__<<" \t"<<__TIME__<<" \t"<<toString(msglevel)<<endl;
    }
    cout<<msg<<endl;
    //cout<<"--------------------------------------------------------------------"<<endl;
}
//===========================================================
AmirLog& AmirLog::getInstance(const std::string log_group){
    for(unsigned int i=0; i<group_instance.size(); i++)
        if(group_instance[i].first == log_group) return *group_instance[i].second;
    AmirLog *t1 = new AmirLog;
    group_instance.push_back(make_pair(log_group,t1));
    AmirString tempfilename("log\\");
    tempfilename<<log_group<<".log";
    t1->setLogFileName(tempfilename);
    t1->instanceID = group_instance.size() - 1;
    return *t1;
}
//===========================================================
bool AmirLog::isFileSizeLarger()
{
    ifstream in(logfilename, std::ios::binary | std::ios::ate);
    if (in.is_open())
        if (in.tellg()>filesizeKB*1024)
        {
            in.close();
            return true;
        }
    in.close();
    return false;
}
//===========================================================
bool AmirLog::isFileExist(const std::string& name)
{
    return ( access( name.c_str(), F_OK ) != -1 );
}
//===========================================================
const string AmirLog::toString(LogLevel msglevel)
{
    switch(msglevel)
    {
    case _TEST:
        return string("TEST");
    case _FATAL:
        return string("FATAL");
    case _ERROR:
        return string("ERROR");
    case _IMPORTANT:
        return string("IMPORTANT");
    case _DEBUG:
        return string("DEBUG");
    case _INFO:
        return string("INFO");
    default:
        return string("");
    }
}
//===========================================================
//===========================================================
void AmirString::toUpperCase()
{
    for (auto & c: *this) c = toupper(c);
    /*unsigned int ch = std::string::size();
    while (ch--)
        if (std::string::at(ch)>='a' && std::string::at(ch)<='z') std::string::at(ch) -= 32;//A65a97)*/
}
//===========================================================
void AmirString::toLowerCase()
{
    for (auto & c: *this) c = tolower(c);
    /*unsigned int ch = std::string::size();
    while (ch--)
        if (std::string::at(ch)>='A' && std::string::at(ch)<='Z') std::string::at(ch) += 32;//A65a97)*/
}
//===========================================================






