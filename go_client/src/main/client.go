package main

import (
	"bytes"
	"encoding/binary"
	"flag"
	"fmt"
	"net"
	"sync/atomic"
	"time"
)

type type_t int

const (
	LoginReq type_t = iota
	LoginRsp
	Msg
	MsgAck
)

type commonMsg struct {
	flag int
	cmd  type_t
	len  int
	body []byte
}

var service string = "120.25.75.150:8888"
var data []byte
var num int = 100000
var ops uint64
var last_second_pkts uint64
var finish bool = false

func timer(result chan int) {
	timer1 := time.NewTicker(time.Second)
	for {
		select {
		case <-timer1.C:
			v := atomic.LoadUint64(&ops)
			fmt.Printf("%v pkt/s,total recv: %v pkt\n", v-last_second_pkts, v)
			last_second_pkts = v
		}
		if finish {
			break
		}
	}
	result <- 0

}
func checkError(err error) {
	if err != nil {
		fmt.Printf("error:%s", err.Error())
	}
}
func request(conn net.Conn, cmd type_t) {
	var request commonMsg
	request.flag = 0x12345678
	request.cmd = cmd
	request.len = 12 + len(data)
	//fmt.Println("pkt len:", request.len)
	sendbuf := new(bytes.Buffer)
	var binerr error
	binerr = binary.Write(sendbuf, binary.LittleEndian, int32(request.flag))
	if binerr != nil {
		fmt.Println("binary.Write failed:", binerr)
		return
	}
	binerr = binary.Write(sendbuf, binary.LittleEndian, int32(request.cmd))
	if binerr != nil {
		fmt.Println("binary.Write failed:", binerr)
		return
	}
	binerr = binary.Write(sendbuf, binary.LittleEndian, int32(request.len))
	if binerr != nil {
		fmt.Println("binary.Write failed:", binerr)
		return
	}
	binerr = binary.Write(sendbuf, binary.LittleEndian, data)
	if binerr != nil {
		fmt.Println("binary.Write failed:", binerr)
		return
	}
	conn.Write(sendbuf.Bytes())
}
func response(conn net.Conn) int {
	buf := make([]byte, 1024)
	_, err := conn.Read(buf)
	if err != nil {
		fmt.Println("Server is dead ...", err)
		conn.Close()
		return -1
	}

	cmd := binary.LittleEndian.Uint32(buf[4:8])
	//	fmt.Printf("recv  rsp cmd:%d\n", cmd)
	return int(cmd)
}

func worker(ch chan int) {
	conn, err := net.Dial("tcp", service)
	checkError(err)
	request(conn, LoginReq)
	if response(conn) != int(LoginRsp) {
		fmt.Println("login fail")
		ch <- -1
		return
	}
	fmt.Println("login success")
	for i := 0; i < num; i++ {

		request(conn, Msg)
		if response(conn) != int(MsgAck) {
			fmt.Println("recv fail")
			ch <- -1
			return
		}
		atomic.AddUint64(&ops, 1)
		//fmt.Println("recv success")

	}
	ch <- num
}
func main() {
	threads := flag.Int("c", 1, "thread num")
	size := flag.Int("d", 100, "send data size")
	pkts := flag.Int("n", 100000, "every thread send packet num")
	flag.Parse()
	num = *pkts
	data = make([]byte, *size)
	fmt.Println("data len:", cap(data), data[0])
	chs := make([]chan int, *threads)

	/*	test := make(chan int)
		for i := 0; i < 5; i++ {
			select {
			case test <- 1:
			case test <- 2:
			}
			fmt.Println(<-test)

		}*/
	var result = make(chan int)
	go timer(result)
	for i := 0; i < *threads; i++ {
		chs[i] = make(chan int)
		go worker(chs[i])
	}
	var total int = 0
	for _, ch := range chs {
		total += <-ch
		//fmt.Println(<-ch)
	}
	time.Sleep(2 * time.Second)
	finish = true
	<-result
	fmt.Println("total send :", total)
}
