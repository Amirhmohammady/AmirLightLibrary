#pragma once
#include <winsock2.h>
#include <vector>
#include <mutex>
#include <condition_variable>

class ClientEvent{
public:
    virtual void onCloseServerSocket(){}
    virtual void onReciveData(char *buff, unsigned int bufflen){}
    virtual ~ClientEvent(){}
private:
};
//=====================================================================================
class ServerEvent{
public:
    virtual void onCloseClientSocket(unsigned int conectionID){}
    virtual void onCreateClientSocket(unsigned int conectionID){}
    virtual void onCloseServerSocket(){}
    virtual void onReciveData(unsigned int conectionID, char *buff, unsigned int bufflen){}
    virtual ~ServerEvent(){}
private:
};
//=====================================================================================
class AmirServerSocket{
public:
    AmirServerSocket();
    AmirServerSocket(unsigned int listenport, ServerEvent &srvrevnt);
    void setPortandEventClass(unsigned int listenport, ServerEvent &srvrevnt);
    void start();
    bool isStarted();
    void closeClientConnection(unsigned int connectionID);
    void closeAllClientConnection();
    void closeServerConnection();
    void sendToClient(unsigned int connectionID, char* buff, unsigned int bufflen);
    void waitUntilServerSocketClosed();
    ~AmirServerSocket();
private:
    int getUnActiveClientID();
    void startServerThread();
    void clientThread(int connectionID);
    bool isPortOpen(unsigned int port);
    SOCKET server_socket = 0;
    unsigned int listenport;
    char threadmessage;
    struct ClientConnection{
        SOCKET skt = 0;
        bool issocketactive = false;
        bool isthreadactive = false;
        std::string clientaddr;
        int clientport;
    };
    std::vector<ClientConnection> client_connections;
    ServerEvent *srvrevnt;
    static std::mutex mtx;
    static std::condition_variable cv;
    unsigned int aliveclientthreads = 0;
    bool is_threads_in_use = false;
    bool isserversocketactive = false;
    bool isinitialized = false;
};
//=====================================================================================
class AmirClientSocket{
public:
    AmirClientSocket();
    AmirClientSocket(std::string host, unsigned short port, ClientEvent &clientevnt);
    void setPortandEventClass(std::string host, unsigned short port, ClientEvent &clientevnt);
    void start();
    bool isStarted();
    void closeConnection();
    void waitUntilClientSocketClosed();
    void sendToServer(char* buff, unsigned int bufflen);
    ~AmirClientSocket();
private:
    void ServerThread();
    SOCKET ssocket = 0;
    std::string host;
    unsigned short port;
    ClientEvent *clientevnt;
    static std::mutex mtx;
    static std::condition_variable cv;
    bool is_thread_in_use = false;
    bool issocketactive = false;
    bool isinitialized = false;
};

