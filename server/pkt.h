#ifndef _MTYPE_H_
#define _MTYPE_H_

#define PKT_HEAD_LEN          12
#define  HEAD_FLAG            0x12345678
class Session;
enum mtype {
    LoginReq,
    LoginRsp,
    Msg,
    MsgAck
};

typedef struct msgHead {
    int     flag;
    mtype   type;
    int     len;
} tMsgHead;
typedef struct _msg {
    tMsgHead    head;
    char        body[];
} tMsg;


typedef struct pkt {
    char*     data;
    int       len;
} tPkt;

tPkt*  newPkt(int dataLen = 0);
void  freePkt(tPkt*  pkt);
int getPktLen(char* data, int len);

#endif
