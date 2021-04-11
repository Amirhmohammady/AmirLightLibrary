#include "../Headers/AmirGUI.h"
#include "../Headers/AmirLog.h"
#include "../Headers/AmirException.h"
#include "../Headers/AmirLib1Globals.h"

using namespace std;

//============================================================================
int AmirWindow::windowscounter = 0;
thread *AmirWindow::OSmsgdispacherthread = nullptr;
queue<InternalMessage> AmirWindow::internalmessages;
DWORD AmirWindow::threadID;
bool AmirWindow::terminateOSmsgdispacherthread = false;
int tmp = 0;
//============================================================================
AmirWindow::AmirWindow():AmirWindow(""){}
//============================================================================
void AmirWindow::add(AmirChildWindow &childwindow)
{
    if(iswindowalive)
    {
        InternalMessage im;
        im.amirwindowptr = this;
        im.messagetype = AddChild;
        im.msgcontent = &childwindow;
        internalmessages.push(im);
    }
    else throw AmirException("create window before adding child window.");
}
//============================================================================
void AmirWindow::addMSG(AmirChildWindow *childwindow)
{
    if(iswindowalive)
    {
        string wtype;
        switch(childwindow->getType())
        {
        case BUTTON:
            wtype = "BUTTON";
            break;
        case TEXTBOX:
            wtype = "Edit";
            break;
        }
        HWND *hwnd01 = (HWND*)&(childwindow->hwnd);
        *hwnd01=
        CreateWindowExA(0,wtype.c_str(),
                        childwindow->getText().c_str(),
                        WS_VISIBLE|WS_CHILD,
                        childwindow->xpos,childwindow->ypos,
                        childwindow->width,childwindow->height,
                        hwnd,(HMENU)childs.size(),NULL,NULL);
        if (childwindow->hwnd) childs.push_back(childwindow);
        else _AMIRLOG(AmirLib1LOG, _ERROR, "can not create child window.");
        //throw AmirException("can not create child window.");
    }
}
//============================================================================
AmirWindow::AmirWindow(std::string title, HINSTANCE instance)
{
    this->title = title;
    window.cbSize        = sizeof(WNDCLASSEX);
    window.style         = CS_BYTEALIGNCLIENT;//2;
    window.lpfnWndProc   = getOSMessages;
    window.cbClsExtra    = 0;
    window.cbWndExtra    = 0;
    window.hInstance     = instance;
    //window.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    window.hCursor       = LoadCursor(NULL, IDC_ARROW);
    window.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    window.lpszMenuName  = this->title.c_str();
    window.lpszClassName = this->title.c_str();
    //window.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
    this->instance = instance;
    startLoopThread();
}
//============================================================================
void AmirWindow::createWindow()
{
    if(!iswindowalive)
    {
        windowscounter++;
        iswindowalive = true;
        InternalMessage im;
        im.amirwindowptr = this;
        im.messagetype = CreateWondow;
        internalmessages.push(im);
    }
}
//============================================================================
void AmirWindow::setVisibility(bool type)
{
}
//============================================================================
CALLBACK LRESULT AmirWindow::getOSMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Retrieve instance pointer
    AmirWindow* pWnd = reinterpret_cast<AmirWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if ( pWnd != NULL )  // See Note 1 below
        // Call member function if instance is available
        return pWnd->WindowProc(hwnd, msg, wParam, lParam);
    else
        // Otherwise perform default message handling
        return DefWindowProc(hwnd, msg, wParam, lParam);
}
//============================================================================
void AmirWindow::startLoopThread()
{
    if (!OSmsgdispacherthread)//&& !windowscounter
    {
        _AMIRLOG(AmirLib1LOG, _DEBUG, "creating main thread for dispatching OS messages....");
        OSmsgdispacherthread = new thread(messageLoop);
        OSmsgdispacherthread->detach();
    }
}
//============================================================================
void AmirWindow::messageLoop()
{
    _AMIRLOG(AmirLib1LOG, _DEBUG, "Main thread created.");
    threadID = GetCurrentThreadId();
    SetTimer(NULL, 0, 1000, nullptr);
    MSG message;// = { };
    while (!terminateOSmsgdispacherthread)//windowscounter > 0)
    {
        if (GetMessage(&message, NULL, 0, 0))//GetMessage
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        _AMIRLOG(AmirLib1LOG, _INFO, AmirString(windowscounter)<<'\t'<<tmp++<<'\t'<<terminateOSmsgdispacherthread);
        while(!internalmessages.empty())
        {
            _AMIRLOG(AmirLib1LOG, _INFO, AmirString("doIMessage"));
            InternalMessage im = internalmessages.front();
            internalmessages.pop();
            im.amirwindowptr->doIMessage(im.messagetype, im.msgcontent);
        }
    }
    KillTimer(NULL,0);
}
//============================================================================
AmirWindow::~AmirWindow()
{
    if (iswindowalive) closeWindow();
    _AMIRLOG(AmirLib1LOG, _INFO, AmirString("windowscounter: ")<<windowscounter);//
    //terminate OS message dispache rthread if it be last window
    /*if (!windowscounter)
    {
        _AMIRLOG(AmirLib1LOG, _INFO,AmirString("OSmsgdispacherthread: "<<OSmsgdispacherthread));//<<endl;
        if (OSmsgdispacherthread)
        {
            terminateOSmsgdispacherthread = true;
            //terminate(OSmsgdispacherthread);
            delete OSmsgdispacherthread;
            OSmsgdispacherthread = nullptr;
        }
    }*/
}
//============================================================================
void AmirWindow::CreateWondowMSG()
{
    _AMIRLOG(AmirLib1LOG, _DEBUG, "Registering main window...");
    if(!RegisterClassEx(&window))
    {
        /*MessageBox(NULL, "Window Registration Failed!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);*/
        _AMIRLOG(AmirLib1LOG, _ERROR, "Main window not registered.");
        windowscounter--;
        iswindowalive = false;
        return;
    }
    _AMIRLOG(AmirLib1LOG, _DEBUG, "Main window registered.");
    _AMIRLOG(AmirLib1LOG, _DEBUG, "Creating main window...");
    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
                          window.lpszClassName,
                          title.c_str(),
                          WS_OVERLAPPEDWINDOW,
                          200,
                          150,
                          640,
                          480,
                          NULL,
                          NULL,
                          instance,
                          NULL                /* No Window Creation data */
                         );
    if(hwnd == NULL)
    {
        /*MessageBox(NULL, "Window Creation Failed!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);*/
        _AMIRLOG(AmirLib1LOG, _ERROR, "Main window not created.");
        windowscounter--;
        iswindowalive = false;
        return;
    }
    _AMIRLOG(AmirLib1LOG, _DEBUG, "Main window created.");
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    UpdateWindow(hwnd);
    ShowWindow(hwnd,1);
}
//============================================================================
void AmirWindow::doIMessage(MessageType msgtype, void *msgcontent)
{
    mtx.lock();
    switch(msgtype)
    {
    case CreateWondow:
        CreateWondowMSG();
        break;
    case AddChild:
        addMSG((AmirChildWindow*)msgcontent);
        break;
    }
    mtx.unlock();
}
//============================================================================
void AmirWindow::waitUntilClose()
{
    if (iswindowalive)
    {
        unique_lock<mutex> lck(mtx);
        cv.wait(lck);
    }
}
//============================================================================
/*DWORD AmirWindow::getThreadID(){
    if (!OSmsgdispacherthread) throw "Error message dispatcher thread is not created yet!";
    typedef DWORD (WINAPI * GetThreadIdptr)(HANDLE);
    HMODULE hKernel32 = LoadLibrary("kernel32.dll");
    if (!hKernel32) cout<< "Error loading kernel32.dll!";
    GetThreadIdptr GetThreadId = (GetThreadIdptr)GetProcAddress(hKernel32, "GetThreadId");
    if (!GetThreadId) cout<< "Error loading GetThreadId in kernel32.dll!";
    return GetThreadId((HANDLE)OSmsgdispacherthread->native_handle());
    //(HANDLE)OSmsgdispacherthread.native_handle();
}*/
//============================================================================
LRESULT AmirWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    _AMIRLOG(AmirLib1LOG, _INFO, AmirString("HWND: ")<<hwnd<<" MSG: "<<msg<<" WPARAM: "<<wParam<<" LPARAM: "<<lParam);//<<endl;
    switch(msg)
    {
    case WM_CLOSE:
        _AMIRLOG(AmirLib1LOG, _INFO, AmirString("WM_CLOSE is called"));//<<endl;
        closeWindow();
        break;
    case WM_DESTROY:
        //PostQuitMessage(0);
        _AMIRLOG(AmirLib1LOG, _INFO, AmirString("WM_DESTROY is called"));//<<endl;
        closeWindow();
        break;
    case WM_LBUTTONDOWN:
        actionListener();
        break;
    case WM_COMMAND:
        //for (int i = 0; i < childs.size(); i++)
        _AMIRLOG(AmirLib1LOG, _INFO, AmirString("WM_COMMAND: ")<<wParam);//<<endl;
        if (wParam<childs.size()) childs[wParam]->actionListener();
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
//============================================================================
void AmirWindow::closeWindow()
{
    //cout<<"AmirWindow::closeWindow function"<<endl;
    mtx.lock();
    //cout<<"mutex locked"<<endl;
    //cout<<"hwnd: "<<hwnd<<endl;
    if (iswindowalive)
    {
        iswindowalive = false;
        mtx.unlock();
        //cout<<"destroying windows ..."<<endl;
        DestroyWindow(hwnd);
        //cout<<"windows destroyed"<<endl;
        hwnd = nullptr;
        windowscounter--;
        //cout<<"windowscounter: "<<windowscounter<<endl;
    }
    else mtx.unlock();
    unique_lock<mutex> lck(mtx);
    cv.notify_all();
}
//============================================================================
//============================================================================
AmirChildWindow::AmirChildWindow(WindowType windowtype, string text)
{
    this->text = text;
    this->windowtype = windowtype;
}
//============================================================================
std::string &AmirChildWindow::getText()
{
    return text;
}
//============================================================================
WindowType AmirChildWindow::getType()
{
    return windowtype;
}
//============================================================================
void AmirChildWindow::setText(string text){
    SetWindowTextA(hwnd,text.c_str());
    //this->text = text;
}




