#ifndef _UTIL_H
#define _UTIL_H
#include <iostream>
using namespace std;

typedef unsigned int     uint32_t;
#define  S_OK            0
#define  S_ERR           -1

#define  SD_FAIL        -1
#define  SD_SCCCESS      0
#define  SD_AGAIN        1
#define  SD_CLOSE        2
#define  SD_NODATA        3

#define TCP_RCV_ERROR       -1
#define TCP_RCV_SUCCESS      0
#define TCP_RCV_CONTINUE     1

#define Assert(ptr)   if(NULL==ptr) {                         \
       					   cout<<"alloc memory fail"<<endl;     \
                           exit(-1);                            \
                      }

#endif
