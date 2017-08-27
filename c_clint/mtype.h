#ifndef _MTYPE_H_
#define _MTYPE_H_
enum mtype{
    LoginReq,
    LoginRsp,
    Msg,
    MsgAck
};

#define   HEAD_FLAG         0X12345678
typedef struct msgHead{
    int  flag;
    mtype type;
    int len;
}msgHead_st;
typedef struct msg{
    msgHead_st  head;
    char body[1024];
}msg_st;

#endif
