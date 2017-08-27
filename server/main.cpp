#include<signal.h>
#include <sys/select.h>
#include <pthread.h>
#include "tcpManager.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int g_bStop = false;
int g_sendCount = 0;
int g_recvCount=0;
int last_sec_pkts=0;
static void sigHandler(int signo) {
    cout << "recv signo: " << signo << endl;
    if(signo==SIGINT){
        last_sec_pkts=0;
        g_sendCount=0;
        g_recvCount=0;
    }
}
void setTimer(int seconds, int mseconds)  {
    struct timeval temp;
    temp.tv_sec = seconds;
    temp.tv_usec = mseconds;
    select(0, NULL, NULL, NULL, &temp);
}

static void*  workThread(void* arg) {
    if(NULL == arg) {
        g_bStop = true;
        return NULL;
    }
    TcpManager* ser = reinterpret_cast<TcpManager*>(arg);
    while(!g_bStop) {
        setTimer(1, 0);
        pthread_mutex_lock(&mutex);
        cout << g_recvCount<<" [total pkts:"<<g_sendCount<<"] " << g_sendCount-last_sec_pkts << "pkts/sec" << endl;
        last_sec_pkts=g_sendCount;
        pthread_mutex_unlock(&mutex);
    }
}

int main() {
    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);
    TcpManager* pTcpManager = TcpManager::getInstance();
    if(NULL == pTcpManager) {
        cout << "get TcpManager instance fail" << endl;
        return 0;
    }
    if(S_ERR == pTcpManager->initServer()) {
        cout << "initServer fail" << endl;
        return 0;
    }
    pthread_t  thread_id;
    pthread_create(&thread_id, NULL, workThread, pTcpManager);
    while(!g_bStop) {
        pTcpManager->startServer();
    }
    cout<<"shutdown"<<endl;
    return 0;
}

