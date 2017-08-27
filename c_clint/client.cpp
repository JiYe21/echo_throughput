#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <cstdio>
#include <pthread.h>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include "mtype.h"
using namespace std;
char chatMsg[1000];
int clifd;
int num;

static void sendLoginReq(int fd){
    char msgBody[32]={0};
    gethostname(msgBody,32);
    msg_st lr;
    lr.head.flag=HEAD_FLAG;
    lr.head.type=LoginReq;
    strcpy(lr.body,msgBody);
    lr.head.len=sizeof(msgHead_st)+strlen(lr.body);
    send(fd,&lr,lr.head.len,0);

}
static int  recvLoginRsp(int fd){

    char msg[2048]={0};

    int res=recv(fd,msg,2048,0);
    msg_st* la=(msg_st*)msg;
    msgHead_st head=la->head;
    if(head.type!=LoginRsp)
        return -1;
    return 0;
}

static void sendMsg(int fd){
    msg_st mr;
    memset(&mr,0,sizeof(mr)); 
    mr.head.flag=HEAD_FLAG;
    mr.head.type=Msg;
    strcpy(mr.body,chatMsg);
    mr.head.len=sizeof(msgHead_st)+strlen(mr.body);
    int len=mr.head.len;
    char* ptr=(char*)&mr;
    while(len){
        int res=  send(fd,ptr,len,0);
        if(res<0){
            cout<<"send fail"<<endl;
            return;
        }
        if(res==0){
            cout<<"server close"<<endl;
            return;
        }

        len-=res;
        ptr+=res;
        // cout<<"len: "<<len<<endl;
    }

}

static int  recvMsgAck(int fd){
    char msg[16*1024]={0};
    int res=recv(fd,msg,16*1024,0);
    if(res==-1){
        cout<<"recv fail"<<endl;
    }
    //cout<<"recv msg answer: "<<endl;
    return 0;

}
int pts=100000;
int fd;
void* workThread(void* arg){
    char sendmsg[]="he";
    char recvmsg[20]={0};
    struct timeval start,stop;
    gettimeofday(&start,NULL);
    while(num--){
        sendMsg(clifd);
    }
    printf("send finish\n");
    int ms,sec;
    gettimeofday(&stop,NULL);
    if(stop.tv_usec<start.tv_usec){
        ms=(stop.tv_usec+1000000-start.tv_usec)/1000;
        stop.tv_sec--;
    }
    else{
        ms=(stop.tv_usec-start.tv_usec)/1000;
    }
    sec=stop.tv_sec-start.tv_sec;

    printf("total %d ms\n",sec*1000+ms);
}
int main(int argc,char* argv []){
    if(argc<3){
        printf("usage : ./a.out datasize pktnum\n");
        return 0;
    }
    memset(chatMsg,'d',atoi(argv[1]));
    num=atoi(argv[2]);
    clifd=socket(AF_INET,SOCK_STREAM,0);
    if(clifd==-1)
        return 0;
    struct sockaddr_in cliaddr;
    memset(&cliaddr,0,sizeof(cliaddr));
    cliaddr.sin_family=AF_INET;
    cliaddr.sin_port=htons(8888);
    inet_pton(AF_INET,"127.0.0.1",&cliaddr.sin_addr);

    if(connect(clifd,(struct sockaddr*)&cliaddr,sizeof(cliaddr))==-1)
    {		perror("connect");
        return 0;
    }

    fd=clifd;
    cout<<"connect success fd: "<<clifd<<endl;
    sendLoginReq(clifd);
    if(recvLoginRsp(clifd)){
        cout<<"login fail"<<endl;
        return 0;
    }
    cout<<"login success"<<endl;
    pthread_t thread_id;
    pthread_create(&thread_id,NULL,workThread,NULL);
    while(1){
        while(true){
            recvMsgAck(clifd);
        }
    }

}
