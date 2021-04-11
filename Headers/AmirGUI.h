#pragma once
#include <windows.h>
#include <thread>
#include <cstring>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>

class InternalMessage;
enum MessageType{
    CreateWondow, AddChild
};
enum WindowType{
    BUTTON, TEXTBOX
};
//=============================================================================
class AmirChildWindow{
public:
    AmirChildWindow(WindowType windowtype, std::string text);
    void setText(std::string text);
    virtual void actionListener(){}
    unsigned short xpos,ypos,height,width;
    std::string &getText();
    WindowType getType();
    const HWND hwnd = nullptr;
private:
    WindowType windowtype;
    std::string text;
};
//=============================================================================
class AmirWindow{
public:
    AmirWindow();
    AmirWindow(std::string title, HINSTANCE instance = nullptr);
    void add(AmirChildWindow &childwindow);
    void createWindow();
    void setVisibility(bool type);
    virtual void actionListener(){}
    void waitUntilClose();
    ~AmirWindow();
    unsigned short xpos,ypos,height,width;
private:
    static bool terminateOSmsgdispacherthread;
    static std::queue<InternalMessage> internalmessages;
    static std::thread *OSmsgdispacherthread;
    static int windowscounter;
    static CALLBACK LRESULT getOSMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static void messageLoop();
    static void startLoopThread();
    static DWORD threadID;
    //--------------------------------------------
    std::string title;
    LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void closeWindow();
    void doIMessage(MessageType msgtype, void *msgcontent);
    HWND hwnd = nullptr;
    bool iswindowalive = false;
    void CreateWondowMSG();
    void addMSG(AmirChildWindow *childwindow);
    std::vector <AmirChildWindow*> childs;
    WNDCLASSEX window = {0};
    HINSTANCE instance;
    std::mutex mtx;
    std::condition_variable cv;
    //string title;
};
//=============================================================================
class InternalMessage{
public:
    MessageType messagetype;
    void *msgcontent;
    AmirWindow *amirwindowptr;
private:
};
//=============================================================================
/*class AmirFrame{
public:
    virtual void actionListener(){}
private:
};
class AmirFileChooser{
public:
    virtual void actionListener(){}
private:
};
class AmirLabel{
public:
    virtual void actionListener(){}
private:
};
class AmirTextField{
public:
    virtual void actionListener(){}
private:
};
class AmirPanel{
public:
    virtual void actionListener(){}
private:
};*/



