#pragma once
#include <vector>
#include <cstring>
#include "AmirNetwork.h"
#include "AmirLog.h"
#include <mutex>
#include <ctime>
#include <random>
#include <map>

//============================================================================
class AmirProxyManager;
//============================================================================
class Proxy
{
public:
    std::string host;
    unsigned int port, speed, availability, totalreceive, totalsend, duration;//totalbytes
    unsigned long firstactivitytime, lastactivitytime;
    //AmirProxyManager *owner;
private:
};
//============================================================================
class ProxyRepository
{
public:
    ProxyRepository();
    void reportConnectonFailed(Proxy *p);
    void reportConnectonSucceed(Proxy *p);
    void reportStableConnecton(Proxy *p);
    void reportByteRecieved(Proxy *p, unsigned int bytes);
    void reportByteSent(Proxy *p, unsigned int bytes, int poolId);
    void logSendRecieve(Proxy *p, int poolId);
    bool isFrequentlyClosed(int poolId);
    bool isSpeedLowOnStart(int poolId);
    int getActiveProxyNu(Proxy* proxy);
    void addProxy(Proxy *pr);
    void loadFromFile();
    void addExtraProxyFromFile();
    void saveProxiesToFile();
    void setRandomProxy(unsigned int active_id);
    void changePid(unsigned int active_id, unsigned int pid);
    void sortProxies();
    static bool isBetter(Proxy *a, Proxy *b);
    ~ProxyRepository();
    struct active_proxy
    {
        Proxy *pptr;//activeproxy
        unsigned int pid;//activeproxyid
        short renewcnt = 4;
        unsigned int firstrequesttime, totalreceive, sendcnt, startcnt;
        bool is_speed_checked;
    };
    std::vector<active_proxy> active_proxies;
    std::vector<Proxy*> proxies;
    //---------------------Setting-----------------------------
    std::string extra_folder;
    std::string proxy_file_name;
    unsigned short renew_max = 4, proxy_pool_size = 2, speed_price = 100;
    unsigned int set_new_activity_time = 1000, new_cost = 409, updete_per_time = 120000
                    , updete_per_bytes = 1<<20, received_check_time = 9000;
private:
    static unsigned short getLength(unsigned int i);
    //---------------------Random-----------------------------
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution;
    //----------------Private Variables-----------------------
    std::mutex updatemtx, sortmtx, savemtx;
    unsigned short sortcnt = 0;
};
//============================================================================
class ProxyLayer
{
public:
    struct Header
    {

    };

    std::vector<std::vector<Proxy*>> layers;
    //---------------------Setting-----------------------------

private:

};
//============================================================================
class AmirProxyManager
{
public:
    //add proxy from file
    AmirProxyManager();
    void start();
    ~AmirProxyManager();
private:
    void loadSetting();
    void saveSetting();
    void closeClientSocket(unsigned int i);
    //void closeClientSocketThread(AmirClientSocket *acs);
    //maybe we have thousands proxies but only use some of them in clientsockets
    class SEvent : public ServerEvent
    {
    public:
        AmirProxyManager *owner;
        void onReciveData(unsigned int conectionID, char *buff, unsigned int bufflen);
        void onCreateClientSocket(unsigned int conectionID);
        void onCloseClientSocket(unsigned int conectionID);
    private:
    };
    class CEvent : public ClientEvent
    {
    public:
        //void removeFirstFiveHeader();
        bool isclosed = true;//111, isserverclosed, removefirstline;
        AmirProxyManager *owner;
        void onReciveData(char *buff, unsigned int bufflen);
        void onCloseServerSocket();
        unsigned short ceventsID, sendcounter = 0;
        Proxy *proxiesID;
    };
    SEvent sevent;
    AmirServerSocket serversocket;
    std::vector<AmirClientSocket*> clientsockets;
    std::vector<CEvent*> cevents;
    //---------------------Setting-----------------------------
    unsigned int listen_port = 8125;
    LogType log_type = LOG_TO_CONSOLE;
    bool check_received_on_start = true;
    bool check_frequently_close = true;
    //bool removeFirstFiveHeader = false;
    //---------------------active proxy information-----------------------------
    unsigned short activecnt=0;
    ProxyRepository pRepository;
    ProxyLayer pLayer;
};
//============================================================================

