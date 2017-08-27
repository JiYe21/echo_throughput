#include <sys/socket.h>
#include <cstdio>
#include <arpa/inet.h>
#include <cerrno>
#include <iostream>
#include <unistd.h>
#include<cstdlib>
#include <cstring>

#include "tcpManager.h"
#include "app.h"

using namespace std;
extern int g_bStop ;

void TcpManager::evCallback(tEvent ev, int fd) {
    getInstance()->handleEvent(ev, fd);
}

static void initServerConf(tServerConf& m_conf) {
    m_conf.port = PORT;
    strcpy(m_conf.ip, IP);
}

void TcpManager::handleEvent(tEvent ev, int fd) {
    Session* pSess = NULL;
    char* ip = NULL;
    int ret;
    int port;
    struct sockaddr_in sa;
    socklen_t  len = sizeof(struct sockaddr_in);
    switch(ev) {

        case EVENT_ACCEPT:
            pSess = new(std::nothrow) Session;
            if(NULL == pSess) {
                cout << "new Session fail" << endl;
                return;
            }
            getpeername(fd, (struct sockaddr *) &sa, &len);
            port = ntohs(sa.sin_port);
            ip = inet_ntoa(sa.sin_addr);
            cout << "fd: " << fd << "ip: " << ip << ' ' << "port: " << port << endl;
            pSess->init(ip, port, fd, STATE_CONN);
            addSession(fd, pSess);
            break;

        case EVENT_READ:
            recvMsg(fd);
            break;

        case EVENT_WRITE:
            pSess = getSession(fd);
            if(NULL == pSess) {
                return;
            }
            sendMsg(pSess);
            break;

        default:
            break;
    }
}

TcpManager* TcpManager::getInstance() {
    static TcpManager instance;
    return &instance;
}

int TcpManager::recvMsg(int fd)
{
    Session* pSess = getSession(fd);
    if (NULL == pSess) {
        return -1;
    }
    int res=pSess->rcvMsg();
    if (res == SD_CLOSE) {
        printf("sock:%d close\n", fd);
    }
    else if (res == SD_FAIL) {
        printf("sock:%d recv fail\n", fd);
    }
    else if (res == SD_SCCCESS) {
        if (TCP_RCV_SUCCESS == pSess->parse()) {
            processMsg(pSess);
            return 0;
        }
    }
    m_pEvloop->delEvent(fd, SD_READABLE | SD_WRITABLE);
    m_pSessMap.erase(fd);
    delete pSess;
    return 0;
}

int TcpManager::sendMsg(Session* pSess)
{
    int fd = pSess->m_fd;

    int res =pSess->sendMsg();
    if (res == SD_AGAIN) {
        m_pEvloop->addEvent(fd, SD_WRITABLE);
    }
    else if (res == SD_FAIL) {
        printf("sock:%d send fail", fd);
        m_pEvloop->delEvent(fd, SD_READABLE | SD_WRITABLE);
        m_pSessMap.erase(fd);
        delete pSess;
    }
    else if (res == SD_NODATA) {
        m_pEvloop->delEvent(fd, SD_WRITABLE);
    }
}

int TcpManager::processMsg(Session* pSess)
{
    tPkt* pkt = NULL;
    while (pkt=pSess->nextHandlerMsg()) {
        handlerMsg(pkt, pSess);
    }
}

TcpManager::TcpManager() {
    m_pEvloop = NULL;
    initServerConf(m_conf);

}

void TcpManager::addSession(int fd, Session* pSess) {
    if(NULL == pSess) {
        return;
    }
    m_pSessMap[fd] = pSess;
}

Session* TcpManager::getSession(int fd) {
    map<int, Session*>::const_iterator iter = m_pSessMap.find(fd);
    if(iter == m_pSessMap.end()) {
        cout << "not find fd" << endl;
        return NULL;
    }
    return iter->second;
}
int TcpManager::initServer() {
    m_pEvloop = Evloop::getInstance();
    if(NULL == m_pEvloop) {
        cout << "init server fail" << endl;
        return S_ERR;
    }
    return m_pEvloop->evloopInit(m_conf.ip, m_conf.port, evCallback);

}

void TcpManager::startServer() {
    m_pEvloop->evMain();

}

