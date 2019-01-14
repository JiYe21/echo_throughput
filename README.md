# echo_throughput
a simple echo server for testing throughput

## START
**1 start server**

  cd server   
  make    
  ./server
  
  Then will print  how many pkt is receiving  every second and total pkts have been recevied;
  such as:
  ```
 4195747 [total pkts:4195746] 155371pkts/sec
4348805 [total pkts:4348804] 153058pkts/sec
4501128 [total pkts:4501128] 152324pkts/sec
4658869 [total pkts:4658868] 157740pkts/sec
4816970 [total pkts:4816969] 158101pkts/sec
4974117 [total pkts:4974117] 157148pkts/sec
5128648 [total pkts:5128647] 154530pkts/sec
  ```
  
  **2 start client**   
  cd go_client       
  ./build.sh
  
 execuate ./bin/main -h 
  ```
Usage of ./bin/main:
  -c int
        thread num (default 1)
  -d int
        send data size (default 100)
  -n int
        every thread send packet num (default 100000)
  ```
    
    
  -c how manay client connect the server      
  -d every pkt size     
  -n  how manay pkt will be send every client     
  
  for example :
  ./bin/main -c 100 -n 100000 -d 100
  
  When the client finish ,will print the latency percentage
  ```
  total send : 10000000
<0 ms: 3274029 pkts(32.74%)
<1 ms: 9859062 pkts(98.59%)
<2 ms: 9996330 pkts(99.96%)
<3 ms: 9998881 pkts(99.99%)
<4 ms: 9999483 pkts(99.99%)
<5 ms: 9999680 pkts(100.00%)
<6 ms: 9999837 pkts(100.00%)
<7 ms: 9999884 pkts(100.00%)
<8 ms: 9999890 pkts(100.00%)
<9 ms: 9999895 pkts(100.00%)
<10 ms: 9999985 pkts(100.00%)
<11 ms: 9999994 pkts(100.00%)
<12 ms: 9999997 pkts(100.00%)
<13 ms: 10000000 pkts(100.00%)
  ```
  ## todo
  
  I want to test the performance different process/thread model 、go/c language、and protocal and compare to redis so that improve 
  performance
