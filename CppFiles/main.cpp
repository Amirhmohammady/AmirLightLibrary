#include <iostream>
#include "../Headers/AmirGUI.h"
#include "../Headers/AmirNetwork.h"
#include "../Headers/AmirLib1Globals.h"
#include "../Headers/AmirLib2Globals.h"
#include "../Headers/AmirMath.h"
#include "../Headers/AmirException.h"
#include "../Headers/AmirProxyManager.h"
#include "../Headers/AmirFile.h"
#include "../Headers/AmirHardwareInformation.h"

using namespace std;

class svevnt:public ServerEvent
{
public:
    void onReciveData(unsigned int conectionID, char *buff, unsigned int bufflen)
    {
        char inp[2048];
        memcpy(inp,buff,bufflen);
        //inp[bufflen] = 0;
        cout<<inp<<"received from socket ID: "<<conectionID<<endl;
    }
    void onCloseClientSocket(unsigned int conectionID){
        if (conectionID == 0) isactive = false;
    }
    bool isactive = true;
} srvrevnt;
class clevnt:public ClientEvent
{
public:
    void onReciveData(char *buff, unsigned int bufflen)
    {
        char inp[2048];
        memcpy(inp,buff,bufflen);
        inp[bufflen] = 0;
        cout<<"received: "<<inp<<endl;
    }
    void onCloseServerSocket(){
        isactive = false;
    }
    bool isactive = true;
} clntevnt;
//=========================================================================================
/*class child02:public AmirChildWindow
{
public:
    child02(WindowType windowtype, std::string text):AmirChildWindow(windowtype, text){}
    void actionListener(){
        cout<<"tttttttttttttttttttttttttttttttttttt"<<endl;
        ch->setText("zvxcvxcbbvcvvc");
    }
    AmirChildWindow *ch;
};
class child03:public AmirChildWindow
{
public:
    child03(WindowType windowtype, std::string text):AmirChildWindow(windowtype, text){}
    void actionListener(){
        cout<<"ssssssssssssssssssssssssssssssssssss"<<endl;
    }
};
class AmirWindow02:public AmirWindow{
public:
    AmirWindow02(std::string title, HINSTANCE instance):AmirWindow(title, instance){}
    void actionListener(){
        cout<<"llllllllllllllllllllllllllllllllll"<<endl;
    }
};*/
//=========================================================================================
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nCmdShow )
{
    /*TEST AMIRLOG*/
    //tempLOG.logtype = LOG_TO_CONSOLE;
    //tempLOG.log(_INFO, "test");
    //tempLOG.log(_FATAL, "test2");

    /*AmirLib1LOG.loglevel = _TEST;
    AmirLib1LOG.logtype = LOG_TO_CONSOLE;
    AmirLib1LOG.logDateTimeLine = false;
    AmirString as("https");
    string host;
    int port = 0;
    as>>host>>port;
    cout<<host<<':'<<port<<endl;*/

    tempLOG.loglevel = _TEST;
    AmirLib1LOG.loglevel = _TEST;
    AmirLib2LOG.loglevel = _DEBUG;
    AmirLib2LOG.logtype = LOG_TO_CONSOLE;
    AmirLib1LOG.logtype = LOG_TO_CONSOLE;
    AmirLib2LOG.logDateTimeLine = false;
    AmirLib1LOG.logDateTimeLine = false;
    //tempLOG.logDateTimeLine = false;
    //tempLOG.filesizeKB = 1;
    /*AmirServerSocket as;
    as.setPortandEventClass(8125, srvrevnt);
    as.start();
    as.waitUntilServerSocketClosed();*/
    AmirProxyManager().start();
    Sleep(1000);
    //cout<<ap.getHDDSerial();
    return 0;
}












