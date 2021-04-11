#include "../Headers/AmirFile.h"
#include "../Headers/AmirLib1Globals.h"
#include <windows.h>//CreateDirectory
#include <dirent.h>

using namespace std;

//=================================================================================
AmirFile::AmirFile() {}
//=================================================================================
void AmirFile::getAllFiles(string foldername, vector<string> &files, bool listsubfolder)
{
    DIR *dir;
    struct dirent *direntry;
    if ((dir = opendir (foldername.c_str())) != NULL)
    {
        // print all the files and directories within directory
        while ((direntry = readdir (dir)) != NULL)
        {
            //cout<<direntry->d_name<<endl;
            //check if direntry->d_name is . or .. or not
            if (strcmp(direntry->d_name,".")&&strcmp(direntry->d_name,".."))
            {
                string subfolder(foldername);
                subfolder += "\\";
                subfolder += direntry->d_name;
                DIR *subdir;
                //check if folder or not
                if ((subdir = opendir (subfolder.c_str())) != NULL)
                {
                    closedir(subdir);
                    if (listsubfolder) getAllFiles(subfolder, files, listsubfolder);
                }
                else
                {
                    //best way to check if folder or not
                    /*if (direntry->d_type == DT_DIR)
                    {
                        string temp(direntry->d_name);
                        list_all_files_subfolder(temp);
                    }else*/
                    //if suffix be .txt then rename
                    AmirString as(foldername);
                    as<<'\\'<<direntry->d_name;
                    files.push_back(as);
                }
            }
        }
        closedir(dir);
    }
    else
    {
        // could not open directory
        _AMIRLOG(AmirLib1LOG, _ERROR, AmirString("could not open ")<<foldername<<" directory");
    }
}
//=================================================================================
void AmirFile::createFolder(string filename){
    unsigned int tmp;
    tmp = filename.find_last_of('\\');
    if (tmp!=string::npos){
        string foldername = filename.substr(0,tmp);
        CreateDirectory(foldername.c_str(), NULL);
    }
}
//=================================================================================
string AmirFile::getFolder(string filename){
    unsigned int tmp;
    tmp = filename.find_last_of('\\');
    if (tmp!=string::npos)
        return filename.substr(0,tmp);
    else return "";
}
//=================================================================================
string AmirFile::nextFile(string filename, bool re_name)
{
    AmirString indexname, indexpath;
    unsigned int fileindex = 0, dotlocation = filename.find_last_of('.');
    if (dotlocation != string::npos)
    {
        indexpath = filename.substr(dotlocation);
        //indexpath.erase(indexpath.begin(), indexpath.begin()+indexpath.find_last_of('.'));
        filename.erase(filename.begin()+dotlocation, filename.end());
    }
    do
    {
        indexname = filename;
        indexname<<fileindex;
        if (dotlocation != string::npos) indexname<<indexpath;
        fileindex++;
    }
    while(access(indexname.c_str(), F_OK) != -1);//check file is exist or not
    if (re_name) rename(filename.c_str(), indexname.c_str());
    return indexname;
}
//=================================================================================
