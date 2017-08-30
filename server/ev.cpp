#include <sys/epoll.h>
#include <cerrno>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include<arpa/inet.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>


#include "ev.h"
#include "util.h"

void Evloop::addEvent(int fd, short mask ) {
    sdFileEvent* pFileEvent = &m_fileEvents[fd];
    struct epoll_event ev = { 0 };
    ev.events = 0;
    int op = pFileEvent->events == SD_NONE ?
        EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    pFileEvent->events |=mask; 
    mask = pFileEvent->events;

    if (mask & SD_READABLE) ev.events |= EPOLLIN;
    if (mask & SD_WRITABLE) ev.events |= EPOLLOUT;
    ev.data.fd = fd;

    epoll_ctl(m_epollfd, op, fd, &ev);
}
void Evloop::delEvent( int fd, short mask) {

    sdFileEvent* pFileEvent = &m_fileEvents[fd];
    if (pFileEvent->events & mask==SD_NONE) {
        return;
    }
    pFileEvent->events &=(~mask);
    mask = pFileEvent->events;
    struct epoll_event ev ={ 0 };
    ev.events = 0;
    if (mask & SD_READABLE) ev.events |= EPOLLIN;
    if (mask & SD_WRITABLE) ev.events |= EPOLLOUT;
    ev.data.fd = fd;

    if (mask != SD_NONE) {
        epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &ev);
    }
    else {
        epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, &ev);

    }
}

void Evloop:: setNonBlock(int fd) {
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

Evloop::Evloop() {
    m_epollfd = 0;
    m_serverfd = 0;
    pEvCbk = NULL;
    m_num = 1024;
    m_fileEvents = (sdFileEvent*)malloc(m_num * sizeof(sdFileEvent));
    memset(m_fileEvents, 0, sizeof(sdFileEvent)*m_num);
}

Evloop* Evloop:: getInstance() {
   static  Evloop instance;
    return &instance;
}

int Evloop::evloopInit(const char* ip, int port, EvHandlerCbk cb) {
    if(NULL == cb || NULL == ip) {
        cout << "evloopInit fail" << endl;
        return S_ERR;
    }
    pEvCbk = cb;
    m_epollfd = epoll_create(1024);
    if(-1 == m_epollfd) {
        cout << "epoll_create fail" << endl;
        return S_ERR;
    }

    m_serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == m_serverfd) {
        cout << "socket fail" << endl;
        return S_ERR;
    }
    struct sockaddr_in seraddr;
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &seraddr.sin_addr) == 0) {
        cout << "invalid ip: " << ip << endl;
        return S_ERR;
    };
    int opt=1;
    setsockopt(m_serverfd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
    if(bind(m_serverfd, (struct sockaddr*)&seraddr, sizeof(seraddr)) == -1) {
        cout << "bind fail" << endl;
        return S_ERR;
    }

    listen(m_serverfd, PENDING_CONN_QUEUE_LEN);

    addEvent(m_serverfd, SD_READABLE);
    return S_OK;
}

int  Evloop::evMain() {
    struct epoll_event evs[MAX_EVENT] = {0};
    int ret = epoll_wait(m_epollfd, evs, MAX_EVENT, EPOLL_WAIT_TIME);
    if(0 == ret) {
        return S_OK;
    }
    if(-1 == ret && errno!=EINTR) {
        cout << "epoll_wait fail" << endl;
        return S_ERR;
    }

    for(int i = 0; i < ret; i++) {
        int fd = evs[i].data.fd;
        if(fd == m_serverfd) {
            struct sockaddr_in pSessaddr;
            socklen_t  len = sizeof(pSessaddr);
            int clifd = accept(m_serverfd, (struct sockaddr*)&pSessaddr, &len);
            if(-1 == clifd) {
                cout << "accept fail" << endl;
                continue;
            }
            setNonBlock(clifd);

            if (fd > m_num) {
                int size = 2 * fd;
                m_fileEvents = (sdFileEvent*)realloc(m_fileEvents, (size) * sizeof(sdFileEvent));
                memset(m_fileEvents + m_num * sizeof(sdFileEvent), 0, (size - m_num) * sizeof(sdFileEvent));
                m_num=size;
            }
            addEvent(clifd,SD_READABLE);
            pEvCbk(EVENT_ACCEPT, clifd);
        }
        else {

            //cout<<"recv packet"<<endl;
            if(evs[i].events & EPOLLIN) {
                pEvCbk(EVENT_READ, fd);

            }
            else if(evs[i].events & EPOLLOUT) {
                pEvCbk(EVENT_WRITE, fd);
            }
        }
    }
    return S_OK;
}
