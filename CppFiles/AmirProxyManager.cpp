#include "../Headers/AmirProxyManager.h"
#include "../Headers/AmirLib2Globals.h"
#include "../Headers/AmirException.h"
#include "../Headers/AmirFile.h"
#include <fstream>//ifstream ofstream
#include <windows.h>//GetTickCount CreateDirectory
#include <cmath>//log2
#include <algorithm>//sort
#include <conio.h>//getch
#include <iostream>//cin.getline
#include <thread>

using namespace std;

AmirProxyManager::AmirProxyManager()
{
    //strcpy(temp1, "CONNECT 190.109.167.9:57608 HTTP/1.1\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:60.0) Gecko/20100101 Firefox/60.0\nProxy-Connection: keep-alive\nConnection: keep-alive\nHost: 190.109.167.9:57608\n\n");//"GET http://190.109.167.9:57608 HTTP/1.1\n\n");
    //111
    pRepository.extra_folder = "ExtraProxy";
    pRepository.proxy_file_name = "Proxies.txt";
    loadSetting();
    saveSetting();
    AmirLib2LOG.logtype = log_type;
    CreateDirectory(pRepository.extra_folder.c_str(), NULL);
    pRepository.active_proxies.resize(pRepository.proxy_pool_size);
    pRepository.loadFromFile();
    for(unsigned int z1=0; z1<pRepository.proxy_pool_size; z1++)
        pRepository.setRandomProxy(z1);
}
//=============================================================
void AmirProxyManager::loadSetting()
{
    ifstream settingfile("Setting.ini");
    AmirString as;
    char tmp1[256];
    string tmp2;
    while(settingfile.good())
    {
        settingfile.getline(tmp1, 256);
        as = tmp1;
        as>>tmp2;
        if (tmp2 == "listen_port")
        {
            as>>listen_port;
            continue;
        }
        if (tmp2 == "check_receive_on_new_proxy")
        {
            as>>check_received_on_start;
            continue;
        }
        if (tmp2 == "check_stability_on_new_proxy")
        {
            as>>check_frequently_close;
            continue;
        }
        if (tmp2 == "log_type")
        {
            unsigned int tmp3;
            as>>tmp3;
            log_type=(LogType)tmp3;
            continue;
        }
        if (tmp2 == "try_to_reconnect")
        {
            as>>pRepository.renew_max;
            continue;
        }
        if (tmp2 == "set_new_activity_time")
        {
            as>>pRepository.set_new_activity_time;
            continue;
        }
        if (tmp2 == "new_cost")
        {
            as>>pRepository.new_cost;
            if (pRepository.new_cost>1<<12) pRepository.new_cost = 1<<12;
            continue;
        }
        if (tmp2 == "received_check_time")
        {
            as>>pRepository.received_check_time;
            continue;
        }
        if (tmp2 == "update_per_check_time")
        {
            as>>pRepository.updete_per_time;
            continue;
        }
        if (tmp2 == "updete_per_bytes")
        {
            as>>pRepository.updete_per_bytes;
            continue;
        }
        if (tmp2 == "speed_price")
        {
            as>>pRepository.speed_price;
            continue;
        }
        if (tmp2 == "proxy_pool_size")
        {
            as>>pRepository.proxy_pool_size;
            continue;
        }
        if (tmp2 == "extra_folder")
        {
            as>>pRepository.extra_folder;
            continue;
        }
        if (tmp2 == "proxy_file")
        {
            as>>pRepository.proxy_file_name;
            continue;
        }
    }
    settingfile.close();
}
//=============================================================
void AmirProxyManager::saveSetting()
{
    ofstream settingfile("Setting.ini");
    settingfile<<"listen_port                  "<<listen_port<<endl;
    settingfile<<"check_receive_on_new_proxy   "<<check_received_on_start<<endl;
    settingfile<<"check_stability_on_new_proxy "<<check_frequently_close<<endl;
    settingfile<<"log_type                     "<<log_type<<endl;
    settingfile<<"try_to_reconnect             "<<pRepository.renew_max<<endl;
    settingfile<<"set_new_activity_time        "<<pRepository.set_new_activity_time<<endl;
    settingfile<<"new_cost                     "<<pRepository.new_cost<<endl;
    settingfile<<"received_check_time          "<<pRepository.received_check_time<<endl;
    settingfile<<"update_per_check_time        "<<pRepository.updete_per_time<<endl;
    settingfile<<"updete_per_bytes             "<<pRepository.updete_per_bytes<<endl;
    settingfile<<"speed_price                  "<<pRepository.speed_price<<endl;
    settingfile<<"proxy_pool_size              "<<pRepository.proxy_pool_size<<endl;
    settingfile<<"extra_folder                 "<<pRepository.extra_folder<<endl;
    settingfile<<"proxy_file                   "<<pRepository.proxy_file_name<<endl;
    settingfile.close();
}
//=============================================================
void AmirProxyManager::start()
{
    sevent.owner = this;
    try
    {
        if (!serversocket.isStarted())
        {
            serversocket.setPortandEventClass(listen_port, sevent);
            serversocket.start();
        }
    }
    catch(AmirException &ae)
    {
        _AMIRLOG(AmirLib2LOG, _ERROR, ae.what());
    }
    //---------------manage commands--------------------------
    bool wanttoexit = false;
    char command[1024];
    AmirString as;
    string str;
    while(!wanttoexit)
    {
        cin.getline(command, 1023);
        as = command;
        as.toLowerCase();
        as >> str;
        if (str == "exit")
        {
            wanttoexit = true;
            serversocket.closeServerConnection();
            continue;
        }
        if (str == "renew")
        {
            for(unsigned int z1=0; z1<clientsockets.size(); z1++)
            {
                serversocket.closeClientConnection(z1);
                closeClientSocket(z1);//clientsockets[z1]->closeConnection();
            }
            for(unsigned int z1=0; z1<pRepository.proxy_pool_size; z1++)
                pRepository.setRandomProxy(z1);
            continue;
        }
        if (str == "addproxy")
        {
            pRepository.addExtraProxyFromFile();
            continue;
        }
        if (str == "close")
        {
            for(unsigned int z1=0; z1<clientsockets.size(); z1++)
            {
                serversocket.closeClientConnection(z1);
                closeClientSocket(z1);//clientsockets[z1]->closeConnection();
            }
            continue;
        }
        if (str == "closepid")
        {
            unsigned int pId = pRepository.proxies.size();
            as >> pId;
            if(pId < pRepository.proxies.size())
            {
                for(unsigned int z1=0; z1<cevents.size(); z1++)
                    if (cevents[z1]->proxiesID == pRepository.proxies[pId])
                    {
                        serversocket.closeClientConnection(z1);
                        closeClientSocket(z1);
                    }
            }
            continue;
        }
        if (str == "setpid")
        {
            unsigned int newpid = 0, activeid;
            as>>activeid;
            as>>newpid;
            pRepository.changePid(activeid, newpid);
            continue;
        }
        if (str == "save")
        {
            pRepository.saveProxiesToFile();
            continue;
        }
        if (str == "help")
        {
            cout<<"all commands: renew addproxy close closepid setpid save exit help"<<endl;
            continue;
        }
        cout<<str<<" was not declared!!! print help to see all commands."<<endl;
    }
    cout<<endl<<"!!!! waiting for server close !!!!"<<endl;
    serversocket.waitUntilServerSocketClosed();
    cout<<"!!!! server closed !!!!"<<endl;
}
//=============================================================
void AmirProxyManager::SEvent::onCreateClientSocket(unsigned int conectionID)
{
    //we should have some client connection in exchange for server connections.
    //while we have less connection we create and create and in this program we don't delete them
    //server socket guaranteed that use minimum number of connection so we can be sure client connections wont go to be so large.
    while(owner->clientsockets.size()<=conectionID)
    {
        AmirClientSocket *temp = new AmirClientSocket;
        owner->clientsockets.push_back(temp);
        CEvent *temp2 = new CEvent;
        temp2->owner = owner;
        temp2->ceventsID = owner->cevents.size();
        owner->cevents.push_back(temp2);
        _AMIRLOG(AmirLib2LOG, _INFO, AmirString("client connection: ")<<temp2->ceventsID<<" added to vector.");
    }
    try
    {
        if (owner->cevents[conectionID]->isclosed)// !owner->clientsockets[conectionID]->isStarted())
        {
            owner->activecnt++;
            owner->activecnt = owner->activecnt % owner->pRepository.proxy_pool_size;
            if(owner->clientsockets[conectionID]->isStarted())
            {
                _AMIRLOG(AmirLib2LOG, _IMPORTANT, AmirString("waiting to close connection: ")<<conectionID);
                owner->clientsockets[conectionID]->waitUntilClientSocketClosed();
            }
            owner->cevents[conectionID]->proxiesID = owner->pRepository.active_proxies[owner->activecnt].pptr;
            _AMIRLOG(AmirLib2LOG, _DEBUG, AmirString("try to start proxy ")<<owner->activecnt<<char(16)<<owner->pRepository.active_proxies[owner->activecnt].pid<<" "
                     <<owner->pRepository.active_proxies[owner->activecnt].pptr->host<<':'<<owner->pRepository.active_proxies[owner->activecnt].pptr->port);
            owner->clientsockets[conectionID]->setPortandEventClass(owner->cevents[conectionID]->proxiesID->host, owner->cevents[conectionID]->proxiesID->port, *(owner->cevents[conectionID]));
            //111owner->cevents[conectionID]->removefirstline = false;
            owner->clientsockets[conectionID]->start();
            owner->cevents[conectionID]->isclosed = false;
            owner->cevents[conectionID]->sendcounter = 0;
            owner->pRepository.reportConnectonSucceed(owner->cevents[conectionID]->proxiesID);
        }
        /*111else
        {
            owner->cevents[conectionID]->removefirstline = true;
        }*/
        //owner->cevents[conectionID]->isserverclosed = false;
    }
    catch(AmirException &ae)
    {
        owner->cevents[conectionID]->isclosed = true;
        owner->pRepository.reportConnectonFailed(owner->cevents[conectionID]->proxiesID);
        owner->serversocket.closeClientConnection(conectionID);
    }
}
//=============================================================
void AmirProxyManager::closeClientSocket(unsigned int i)
{
    clientsockets[i]->closeConnection();
    cevents[i]->isclosed = true;
}
//=============================================================
void AmirProxyManager::SEvent::onReciveData(unsigned int conectionID, char *buff, unsigned int bufflen)
{
    //111 if (owner->cevents[conectionID]->sendcounter==0) owner->clientsockets[conectionID]->sendToServer(owner->temp1,strlen(owner->temp1));
    //CONNECT 190.109.167.9:57608 HTTP/1.1\n
    /*111if (owner->cevents[conectionID]->removefirstline)
        for(unsigned int z1=0; z1<bufflen; z1++)
            if(buff[z1] == '\n')
            {
                owner->cevents[conectionID]->removefirstline = false;
                buff += z1+1;
                bufflen -= (z1+1);
                break;
            }
    if (owner->cevents[conectionID]->removefirstline) return;*/
    owner->clientsockets[conectionID]->sendToServer(buff, bufflen);

    Proxy *tpptr = owner->cevents[conectionID]->proxiesID;
    int poolId = owner->pRepository.getActiveProxyNu(tpptr);

    owner->pRepository.reportByteSent(tpptr, bufflen, poolId);

    owner->cevents[conectionID]->sendcounter++;
    //---------------------update availability-------------------------
    if (owner->cevents[conectionID]->sendcounter%500 == 20) owner->pRepository.reportStableConnecton(tpptr);
    //----------------------log on every 20 send---------------------------
    if (owner->cevents[conectionID]->sendcounter%20 == 1) owner->pRepository.logSendRecieve(tpptr, poolId);
    //-----------------------active proxy setting--------------------------
    if (owner->check_frequently_close && owner->pRepository.isFrequentlyClosed(poolId))
    {
        owner->serversocket.closeClientConnection(conectionID);
        owner->closeClientSocket(conectionID);//owner->clientsockets[conectionID]->closeConnection();
    }
    //--------------check if active proxy has low receive on start----------------
    if (owner->check_received_on_start && owner->pRepository.isSpeedLowOnStart(poolId))
    {
        owner->serversocket.closeClientConnection(conectionID);
        owner->closeClientSocket(conectionID);//owner->clientsockets[conectionID]->closeConnection();
    }
}
//=============================================================
void AmirProxyManager::SEvent::onCloseClientSocket(unsigned int conectionID)
{
    owner->cevents[conectionID]->proxiesID->duration += (owner->cevents[conectionID]->proxiesID->lastactivitytime - owner->cevents[conectionID]->proxiesID->firstactivitytime);
    _AMIRLOG(AmirLib2LOG, _DEBUG, AmirString("SEvent::onCloseClientSocket: ")<<conectionID);
    owner->closeClientSocket(conectionID);//owner->clientsockets[conectionID]->closeConnection();
    //111owner->cevents[conectionID]->isserverclosed = true;
}
//=============================================================
void AmirProxyManager::CEvent::onReciveData(char *buff, unsigned int bufflen)
{
    owner->serversocket.sendToClient(ceventsID, buff, bufflen);
    proxiesID->totalreceive += bufflen;
    int poolId = owner->pRepository.getActiveProxyNu(proxiesID);
    if (poolId > -1) owner->pRepository.active_proxies[poolId].totalreceive += bufflen;
}
//=============================================================
void AmirProxyManager::CEvent::onCloseServerSocket()
{
    isclosed = true;
    proxiesID->duration += (proxiesID->lastactivitytime - proxiesID->firstactivitytime);
    //111if (isserverclosed) return;
    _AMIRLOG(AmirLib2LOG, _DEBUG, AmirString("CEvent::onCloseServerSocket: ")<<ceventsID);
    owner->serversocket.closeClientConnection(ceventsID);
}
//=============================================================
AmirProxyManager::~AmirProxyManager()
{
    while(!clientsockets.empty())
    {
        delete clientsockets.back();
        delete cevents.back();
        clientsockets.pop_back();
        cevents.pop_back();
    }
}
//=============================================================
//=============================================================
ProxyRepository::ProxyRepository():distribution(32,1048576)
{
    generator.seed(GetTickCount());
}
//=============================================================
void ProxyRepository::loadFromFile()
{
    ifstream proxyfile(proxy_file_name);
    Proxy *tmp;
    while(proxies.size()<800 && proxyfile.good())
    {
        tmp = new Proxy;
        tmp->port = 0;
        proxyfile>>tmp->host;
        proxyfile>>tmp->port;
        proxyfile>>tmp->availability;
        proxyfile>>tmp->speed;
        proxyfile>>tmp->totalreceive;
        proxyfile>>tmp->totalsend;
        proxyfile>>tmp->duration;
        tmp->firstactivitytime = 0;
        tmp->lastactivitytime = 1000;
        addProxy(tmp);
    }
    proxyfile.close();
    addExtraProxyFromFile();
    _AMIRLOG(AmirLib2LOG, _DEBUG, AmirString(proxies.size())<<" proxies added to proxy List");
}
//=============================================================
void ProxyRepository::addExtraProxyFromFile()
{
    ifstream extrafile;
    AmirString as;
    Proxy *tmp1;
    char tmp2[256];
    AmirFile afile;
    vector<string> files;
    afile.getAllFiles(extra_folder, files);
    for(unsigned int z1=0; z1<files.size() && proxies.size()<800; z1++)
    {
        string file = files[z1];
        extrafile.open(file);
        try
        {
            while(proxies.size()<800 && extrafile.good())
            {
                tmp1 = new Proxy;
                tmp2[0] = 0;
                extrafile.getline(tmp2, 255);
                for (int t3=0; tmp2[t3]; t3++)
                    if (tmp2[t3]==':')
                    {
                        tmp2[t3] = ' ';
                        break;
                    }
                as = tmp2;
                as>>tmp1->host;
                tmp1->port = 0;
                as>>tmp1->port;
                tmp1->availability = 1<<11;
                tmp1->speed = 1<<11;
                tmp1->totalreceive = 0;
                tmp1->totalsend = 0;
                tmp1->duration = 500;
                tmp1->firstactivitytime = 0;
                tmp1->lastactivitytime = 1000;
                addProxy(tmp1);
            }
        }
        catch(exception &e)
        {
            _AMIRLOG(AmirLib2LOG, _ERROR, e.what());
        }
        if (extrafile.good())
        {
            ofstream nextfile;
            string nextfilename = afile.nextFile(file);
            nextfile.open(nextfilename);
            while(extrafile.good())
            {
                extrafile.getline(tmp2, 255);
                nextfile<<tmp2<<'\n';
            }
            nextfile.close();
        }
        extrafile.close();
        remove(file.c_str());
    }
    sortProxies();
    saveProxiesToFile();
    //while(proxies.size()>750) proxies.pop_back();
    _AMIRLOG(AmirLib2LOG, _INFO, AmirString("proxies size: ")<<proxies.size());
}
//=============================================================
void ProxyRepository::saveProxiesToFile()
{
    savemtx.lock();
    ofstream proxyfile(proxy_file_name);
    if (proxyfile.is_open())
    {
        for(unsigned int i=0; i<proxies.size(); i++)
        {
            proxyfile<<proxies[i]->host;
            unsigned int z1, spaces = 16 - proxies[i]->host.length();
            for (z1 = 0; z1<spaces; z1++)
                proxyfile<<' ';
            proxyfile<<'\t'<<proxies[i]->port;
            spaces = 10 - getLength(proxies[i]->port);
            for (z1 = 0; z1<spaces; z1++)
                proxyfile<<' ';
            proxyfile<<'\t'<<proxies[i]->availability;
            spaces = 10 - getLength(proxies[i]->availability);
            for (z1 = 0; z1<spaces; z1++)
                proxyfile<<' ';
            proxyfile<<'\t'<<proxies[i]->speed;
            spaces = 10 - getLength(proxies[i]->speed);
            for (z1 = 0; z1<spaces; z1++)
                proxyfile<<' ';
            proxyfile<<'\t'<<proxies[i]->totalreceive;
            spaces = 10 - getLength(proxies[i]->totalreceive);
            for (z1 = 0; z1<spaces; z1++)
                proxyfile<<' ';
            proxyfile<<'\t'<<proxies[i]->totalsend;
            spaces = 10 - getLength(proxies[i]->totalsend);
            for (z1 = 0; z1<spaces; z1++)
                proxyfile<<' ';
            proxyfile<<'\t'<<proxies[i]->duration<<'\n';
        }
        proxyfile.close();
    }
    savemtx.unlock();
}
//=============================================================
void ProxyRepository::addProxy(Proxy *pr)
{
    if (pr->port == 0)
    {
        delete pr;
        return;
    }
    for(unsigned int i=0; i<proxies.size(); i++)
        if(pr->host == proxies[i]->host && pr->port == proxies[i]->port)
        {
            delete pr;
            return;
        }
    _AMIRLOG(AmirLib2LOG, _INFO, AmirString(proxies.size())<<'\t'<<pr->host<<':'<<pr->port<<" added to server proxy");
    proxies.push_back(pr);
}
//=============================================================
void ProxyRepository::sortProxies()
{
    sortmtx.lock();
    sort(proxies.begin(), proxies.end(), isBetter);
    sortmtx.unlock();
}
//=============================================================
bool ProxyRepository::isBetter(Proxy *a, Proxy *b)
{
    return (static_cast<unsigned long long>(a->availability)*static_cast<unsigned long long>(a->speed) >
            static_cast<unsigned long long>(b->availability)*static_cast<unsigned long long>(b->speed));
}
//=============================================================
void ProxyRepository::setRandomProxy(unsigned int active_id)
{
    if (proxies.empty()) throw AmirException("Add some proxies to http proxy.");
    sortcnt++;
    if (sortcnt>10)
    {
        sortcnt = 0;
        sortProxies();
        saveProxiesToFile();
        _AMIRLOG(AmirLib2LOG, _IMPORTANT, ">>Proxies sorted and saved to file.");
    }
    unsigned int t = distribution(generator);
    unsigned int proxyid = log2(t)*proxies.size()/15-proxies.size()/3;//0..814
    changePid(active_id, proxies.size() - proxyid - 1);
    _AMIRLOG(AmirLib2LOG, _INFO, AmirString("random:")<<t<<" \tproxyid:"<<active_proxies[active_id].pid);
    //return proxies.size() - proxyid - 1;
}
//=============================================================
void ProxyRepository::changePid(unsigned int active_id, unsigned int pid)
{
    active_id = active_id%proxy_pool_size;
    if (pid < 0) pid = 0;
    else if(pid >= proxies.size()) pid = proxies.size() -1;
    active_proxies[active_id].pid = pid;
    active_proxies[active_id].pptr = proxies[pid];
    active_proxies[active_id].firstrequesttime = 0;
    active_proxies[active_id].totalreceive = 0;
    active_proxies[active_id].is_speed_checked = false;
    active_proxies[active_id].renewcnt = renew_max;
    active_proxies[active_id].sendcnt = 0;
    active_proxies[active_id].startcnt = 0;
    _AMIRLOG(AmirLib2LOG, _IMPORTANT, AmirString(">>pid changed to: ")<<active_id<<char(16)<<pid);
}
//=============================================================
int ProxyRepository::getActiveProxyNu(Proxy* proxy)
{
    for(unsigned int z1=0; z1<proxy_pool_size; z1++)
        if (proxy == active_proxies[z1].pptr) return z1;
    return -1;
}
//=============================================================
unsigned short ProxyRepository::getLength(unsigned int i)
{
    if (i==0) return 1;
    unsigned short len = 0;
    while(i)
    {
        len++;
        i /= 10;
    }
    return len;
}
//=============================================================
void ProxyRepository::reportConnectonFailed(Proxy *p)
{
    p->availability *= ((1<<13) - new_cost);
    p->availability /= (1<<13);
    int poolId = getActiveProxyNu(p);
    if (poolId>-1)
    {
        _AMIRLOG(AmirLib2LOG, _ERROR, AmirString("proxy: ")<<poolId<<char(16)<<active_proxies[poolId].pid<<" "
                 <<active_proxies[poolId].pptr->host<<':'<<active_proxies[poolId].pptr->port<<" failed to start.");
        active_proxies[poolId].renewcnt--;
        if (active_proxies[poolId].renewcnt < 1) setRandomProxy(poolId);
        else _AMIRLOG(AmirLib2LOG, _DEBUG, AmirString("renew after '")<<active_proxies[poolId].renewcnt<<"' times.");
    }
    else _AMIRLOG(AmirLib2LOG, _ERROR, AmirString("proxy: ")<<p->host<<':'<<p->port<<" failed to start.");
}
//=============================================================
void ProxyRepository::reportConnectonSucceed(Proxy *p)
{
    int poolId = getActiveProxyNu(p);
    if (poolId>-1)
    {
        if (active_proxies[poolId].renewcnt < renew_max) active_proxies[poolId].renewcnt = renew_max;
        active_proxies[poolId].startcnt++;
        _AMIRLOG(AmirLib2LOG, _DEBUG, AmirString("proxy ")<<poolId<<char(16)<<active_proxies[poolId].pid
                 <<" "<<p->host<<':'<<p->port<<" started.");
    }
    else _AMIRLOG(AmirLib2LOG, _DEBUG, AmirString("proxy ")<<p->host<<':'<<p->port<<" started.");
}
//=============================================================
void ProxyRepository::reportStableConnecton(Proxy *p)
{
    p->availability *= ((1<<13) - new_cost);
    p->availability /= (1<<13);
    p->availability += new_cost*4;//1<<15*new_cost/1<<13
}
//=============================================================
void ProxyRepository::reportByteRecieved(Proxy *p, unsigned int bytes)
{
}
//=============================================================
void ProxyRepository::reportByteSent(Proxy *p, unsigned int bytes, int poolId)
{
    p->totalsend += bytes;
    unsigned long currenttime = GetTickCount();
    //--------------set time duration for calculate speed----------------
    //_AMIRLOG(AmirLib2LOG, _TEST, AmirString("Dif:")<<currenttime-(owner->cevents[conectionID]->lastactivitytime)<<"\tCur:"<<currenttime<<"\tlst:"<<owner->cevents[conectionID]->lastactivitytime<<"\tFA:"<<owner->cevents[conectionID]->firstactivitytime);
    updatemtx.lock();
    if (currenttime > p->lastactivitytime + set_new_activity_time)
    {
        p->duration += (p->lastactivitytime - p->firstactivitytime);
        p->lastactivitytime = currenttime;
        p->firstactivitytime = currenttime - 200;
    }
    else
    {
        p->lastactivitytime = currenttime;
    }
    //--------------if total bytes large update speed----------------
    if (p->totalreceive+p->totalsend > updete_per_bytes || p->duration > updete_per_time)
    {
        p->duration += (p->lastactivitytime - p->firstactivitytime);
        p->lastactivitytime = currenttime;
        p->firstactivitytime = currenttime - 200;
        _AMIRLOG(AmirLib2LOG, _IMPORTANT, AmirString("start to update speed ")<<p->host<<':'<<p->port);
        p->speed *= ((1<<13) - new_cost);
        p->speed /= (1<<13);
        p->speed += (((p->totalreceive+p->totalsend)/p->duration)*((new_cost*speed_price)/(1<<11)));
        p->totalreceive = 0;
        p->totalsend = 0;
        p->duration = 500;
    }
    updatemtx.unlock();
    if (poolId>-1)
    {
        active_proxies[poolId].sendcnt++;
        //------------------if send many times, stable active proxy----------------------
        if (active_proxies[poolId].sendcnt%400 == 10 && active_proxies[poolId].renewcnt < renew_max + 3) active_proxies[poolId].renewcnt++;
        //------------------save file----------------------
        if (active_proxies[poolId].sendcnt%2000 == 300)
        {
            thread(saveProxiesToFile, this).detach();
            _AMIRLOG(AmirLib2LOG, _IMPORTANT, "Proxy file saved.");
        }
        //------------------prevent sendcnt startcnt Overflow----------------------
        if (active_proxies[poolId].sendcnt > static_cast<unsigned int>(-1) - 1000)
        {
            active_proxies[poolId].sendcnt = 10;
            active_proxies[poolId].startcnt = 0;
        }
    }
}
//=============================================================
void ProxyRepository::ProxyRepository::logSendRecieve(Proxy *p, int poolId)
{
    if (poolId>-1)
    {
        _AMIRLOG(AmirLib2LOG, _DEBUG, AmirString("Rcv:")<<p->totalreceive<<"\tSnd:"<<p->totalsend<<"\tDur:"
                 <<p->duration<<'\t'<<poolId<<char(16)<<active_proxies[poolId].pid<<'\t'<<p->host<<':'<<p->port);
    }
    else
    {
        _AMIRLOG(AmirLib2LOG, _DEBUG, AmirString("Rcv:")<<p->totalreceive<<"\tSnd:"<<p->totalsend
                 <<"\tDur:"<<p->duration<<'\t'<<p->host<<':'<<p->port);
    }
}
//=============================================================
bool ProxyRepository::isFrequentlyClosed(int poolId)
{
    if (poolId<0) return false;
    if (active_proxies[poolId].sendcnt+6 < active_proxies[poolId].startcnt*2)
    {
        active_proxies[poolId].pptr->availability *= ((1<<13) - new_cost);
        active_proxies[poolId].pptr->availability /= (1<<13);
        _AMIRLOG(AmirLib2LOG, _IMPORTANT, AmirString("--proxy ")<<poolId<<char(16)<<active_proxies[poolId].pid<<' '<<active_proxies[poolId].pptr->host<<':'<<active_proxies[poolId].pptr->port<<" frequently closed and reseted.");
        setRandomProxy(poolId);
        return true;
    }
    else return false;
}
//=============================================================
bool ProxyRepository::isSpeedLowOnStart(int poolId)
{
    if (poolId<0 || active_proxies[poolId].is_speed_checked) return false;
    unsigned long currenttime = GetTickCount();
    if (active_proxies[poolId].firstrequesttime == 0)
    {
        active_proxies[poolId].firstrequesttime = currenttime;
        _AMIRLOG(AmirLib2LOG, _IMPORTANT, AmirString("--set first send time:")<<currenttime);
    }
    if (active_proxies[poolId].firstrequesttime + received_check_time < currenttime)
    {
        active_proxies[poolId].is_speed_checked = true;
        _AMIRLOG(AmirLib2LOG, _IMPORTANT, AmirString("--Total receive:")<<active_proxies[poolId].totalreceive);
        if (active_proxies[poolId].totalreceive == 0)//owner->totalsend > owner->totalreceive*10
        {
            active_proxies[poolId].pptr->speed *= ((1<<13) - new_cost);
            active_proxies[poolId].pptr->speed /= (1<<13);
            _AMIRLOG(AmirLib2LOG, _IMPORTANT, AmirString("--proxy ")<<poolId<<char(16)<<active_proxies[poolId].pid<<' '<<active_proxies[poolId].pptr->host<<':'<<active_proxies[poolId].pptr->port<<" speed is low and reseted.");
            setRandomProxy(poolId);
            return true;
        }
    }
    return false;
}
//=============================================================
ProxyRepository::~ProxyRepository()
{
    saveProxiesToFile();
    while(!proxies.empty())
    {
        delete proxies.back();
        proxies.pop_back();
    }
}
//=============================================================




