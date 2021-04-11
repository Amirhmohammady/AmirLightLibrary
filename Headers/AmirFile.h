#pragma once
#include <string>
#include <vector>

class test{
};
class AmirFile{
public:
    AmirFile();
    void getAllFiles(std::string foldername, std::vector<std::string> &files, bool listsubfolder = false);
    std::string nextFile(std::string filename, bool re_name = false);
    std::string getFolder(std::string filename);
    void createFolder(std::string filename);
private:
};
