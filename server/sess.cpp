#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <sys/socket.h>
#include <unistd.h>

#include "sess.h"
#include "app.h"
extern int g_sendCount;
extern int g_recvCount;
extern pthread_mutex_t mutex;
Session::Session() {
    m_wbufpending = NULL;
    m_rbufpending = NULL;
    m_pktlen = 0;
    m_rcvlen = 0;
    memset(m_buf, 0, sizeof(m_buf));
    m_less_pkt_len=0;
}
Session::~Session() {
    close(m_fd);
    if(m_wbufpending) {
        freePkt(m_wbufpending);
    }
    if(m_rbufpending) {
        freePkt(m_rbufpending);
    }
    while(!m_wbufq.empty()) {
        tPkt* pkt = m_wbufq.front();
        freePkt(pkt);
        m_wbufq.pop();
    }
    while(!m_rbufq.empty()) {
        tPkt* pkt = m_rbufq.front();
        freePkt(pkt);
        m_rbufq.pop();
    }
}

int Session::init(const char* ip, int port, int fd, tState state) {
    strcpy(m_ip, ip);
    m_port = port;
    m_fd = fd;
    m_state = state;
}

int Session::parse() {
    int ret;
    char* data=m_buf;
    while(m_rcvlen >= PKT_HEAD_LEN) {
        if(!m_less_pkt_len){
            if((*(int*)data)!=HEAD_FLAG){
                cout<<"head flag error"<<endl;
                return TCP_RCV_ERROR;;
            }

            m_pktlen = getPktLen(data, m_rcvlen);
            if(m_pktlen < PKT_HEAD_LEN) {
                cout << "get m_pktlen fail 3" << endl;
                return TCP_RCV_ERROR;
            }
            m_rbufpending = newPkt(m_pktlen);
            int len=m_rcvlen>=m_pktlen?m_pktlen:m_rcvlen;
            memcpy(m_rbufpending->data,data,len);
            m_rbufpending->len=len;
            if(m_rcvlen>=m_pktlen){
                data+=m_pktlen;
                m_rcvlen-=m_pktlen;
                ++g_recvCount;
                m_rbufq.push(m_rbufpending);
                m_rbufpending=NULL;


            }else{
                m_less_pkt_len=m_pktlen-len;
                m_rcvlen=0;
                break;
            }
        }        
        else{
            int len=m_rcvlen>=m_less_pkt_len?m_less_pkt_len:m_rcvlen;
            memcpy(m_rbufpending->data+m_rbufpending->len,data,len);
            m_rbufpending->len+=len;
            if(m_rcvlen>=m_less_pkt_len){
                data+=len;
                m_rcvlen-=len;
                m_less_pkt_len=0;
                ++g_recvCount;
                m_rbufq.push(m_rbufpending);
                m_rbufpending=NULL;
            }
            else{
                m_rcvlen=0;
                m_less_pkt_len-=len;
            }

        }

    }
    if(m_rcvlen && data!=m_buf){
        memmove(m_buf,data,m_rcvlen);
    }

    return TCP_RCV_SUCCESS;
}

int Session::rcvMsg() {

    int len = 0;
    int avail=16*1024-m_rcvlen;
    while(avail) {
        len = recv(m_fd, m_buf + m_rcvlen, avail, 0);
        if(0 == len ) {
            cout << "client close" << endl;
            return SD_CLOSE;
        }
        if(-1 == len ) {
            if(errno == EAGAIN) {
                break;
            }
            if(errno == EINTR) {
                continue;
            }
            else {
                cout << "recv fail" << endl;
                return  SD_FAIL;
            }

        }

        m_rcvlen += len;
        avail-=len;
    }
    return SD_SCCCESS;
}
int  Session::sendData() {
    if(m_wbufpending) {
        int len = send(m_fd, m_wbufpending->data, m_wbufpending->len, 0);
        //      msg_st* msg=(msg_st*)(m_wbufpending->data);
        //        cout<<"send type:"<<(msg->head).type<<"send msg: "<<msg->body<<"len: "<<len<<endl;
        if(-1 == len) {
            if(errno == EAGAIN || errno == EINTR) {
                return SD_AGAIN;
            }
            else {
                return SD_FAIL;
            }
        }
        if(0 == len) {
            return SD_FAIL;
        }
        if(len < m_wbufpending->len) {
            m_wbufpending->data += len;
            m_wbufpending->len -= len;
            // pEvCbk(EVENT_WANT_WRITE, m_fd);
            return SD_AGAIN;
        }
        freePkt(m_wbufpending);
        m_wbufpending = NULL;
        pthread_mutex_lock(&mutex);
        ++g_sendCount;
        pthread_mutex_unlock(&mutex);

        return SD_SCCCESS;
    }
    return SD_NONE;

}
int  Session::sendMsg() {
    int ret;
    if (NULL == m_wbufpending) {
        while (!m_wbufq.empty()) {
            m_wbufpending = m_wbufq.front();
            m_wbufq.pop();
            int res=sendData();
            if(res==SD_SCCCESS){
                continue;
            }
            return res;
        }
    }
    return SD_NONE;

}

tPkt* Session::nextHandlerMsg() {
    tPkt* pkt = NULL;
    if(!m_rbufq.empty()) {
        pkt = m_rbufq.front();
        m_rbufq.pop();

    }
    return pkt;
}
void Session::addSdQueue(tPkt* pkt) {
    m_wbufq.push(pkt);
}
