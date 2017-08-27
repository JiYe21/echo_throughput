#include "pkt.h"
#include "util.h"
#include <cstdlib>
#include <cstring>
tPkt* newPkt(int dataLen) {
    tPkt* pkt = (tPkt*)malloc(sizeof(tPkt));
    if(NULL == pkt) {
        return NULL;
    }
    pkt->len = 0;
    pkt->data = NULL;
    if(dataLen > 0) {
        pkt->data = (char*)malloc(dataLen);
        if(NULL == pkt->data) {
            return NULL;
        }
    }
    return pkt;
}
void  freePkt(tPkt* pkt) {
    if(NULL != pkt) {
        if(pkt->data){
            free(pkt->data);
        }
        free(pkt);
    }
}

int getPktLen(char* data, int len) {
    if(NULL == data || len < PKT_HEAD_LEN) {
        return S_ERR;
    }
    tMsgHead* head = (tMsgHead*)data;
    return head->len;
}
int parsePkt() {
}

