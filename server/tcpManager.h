#ifndef _SERVER_H_
#define _SERVER_H_

#include <map>
#include "sess.h"
#include "util.h"
#include "ev.h"
#define  PORT       8889
#define  IP        "0.0.0.0"


typedef void (*SigHandlerCbk)(int signo);
typedef struct serverConf {
    int    port;
    char   ip[32];
} tServerConf;


class TcpManager {
    private:
        TcpManager();
    public:
        int initServer();
        void  startServer();
        void  addSession(int fd, Session* pSess);
        void handleEvent(tEvent ev, int fd);
        Session* getSession(int fd);
        static void evCallback(tEvent ev, int fd);
        static TcpManager* getInstance();

        int recvMsg(int fd);
        int sendMsg(Session * pSess);
        int processMsg(Session * pSess);

    private:
        std::map<int, Session*>     m_pSessMap;
        tServerConf                 m_conf;
        Evloop*                     m_pEvloop;

};

#endif
