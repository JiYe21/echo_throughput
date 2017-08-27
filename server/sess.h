#ifndef _SESS_H
#define _SESS_H
#include <queue>


#include "pkt.h"
#include "util.h"
#include "ev.h"


typedef enum {
    STATE_CONN,
    STATE_OPEN,
    STATE_CLOSED
} tState;
class Session {
    public:
        Session();
        ~Session();
        int init(const char* ip, int port, int fd, tState state);
        int rcvMsg();
        int parse();

        tPkt* nextHandlerMsg();
        void addSdQueue(tPkt* pkt);
        int  sendMsg();
        int  sendData();

    public:
        int                     m_fd;
    private:
        int                     m_port;
        char                    m_ip[32];


        uint32_t                m_events;
        std::queue<tPkt*>       m_wbufq;
        tPkt*                   m_wbufpending;
        std::queue<tPkt*>       m_rbufq;
        tPkt*                   m_rbufpending;
        int                     m_pktlen;//totle len

        char                    m_buf[16*1024];
        int                     m_rcvlen;
        int                     m_less_pkt_len;

    public:
        tState                   m_state;
    private:
        Session(const Session& rhs);
        Session& operator=(const Session& rhs);

};


#endif
