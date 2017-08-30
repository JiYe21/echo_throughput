#include "app.h"
int handlerMsg(tPkt* pkt, Session* pSess) {
    tMsg*  msg = (tMsg*)(pkt->data);
    tMsgHead* head = &msg->head;
    if(STATE_CONN == pSess->m_state  && LoginReq == head->type) {
        head->type = LoginRsp;
        pSess->m_state = STATE_OPEN;
        cout<<"login success"<<endl; 
    }
    else if(STATE_OPEN == pSess->m_state  &&  Msg == head->type) {
        head->type = MsgAck;
    }
    else {
        cout << "packet err,will drop" << endl;
        freePkt( pkt);
    }
    pSess->addSdQueue(pkt);
}

