#ifndef _EV_H
#define _EV_H
#include <map>
using namespace std;
#define SD_NONE     0
#define SD_READABLE 1
#define SD_WRITABLE 2

#define PENDING_CONN_QUEUE_LEN      128
#define MAX_EVENT                   10000
#define EPOLL_WAIT_TIME             500
#define MAX_
typedef enum {
    EVENT_ACCEPT,
    EVENT_READ,
    EVENT_WRITE,
} tEvent;

typedef struct sdFileEvent {
    int fd;
    uint32_t events;
}sdFileEvent;

typedef void (*EvHandlerCbk)(tEvent ev, int fd);

class Evloop {
    private:
        Evloop();
    public:
        ~Evloop(){}
        static Evloop* getInstance();
        int evloopInit(const char* ip, int port, EvHandlerCbk cb);
        int evMain();

        void addEvent(int fd,short mask) ;
        void delEvent(int fd, short mask) ;

        static void setNonBlock(int fd) ;

    private:
        int             m_num;
        sdFileEvent*    m_fileEvents;
        int             m_epollfd;
        int             m_serverfd;
        EvHandlerCbk    pEvCbk;
};

#endif
