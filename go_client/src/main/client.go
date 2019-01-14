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
	tm   int64
	body []byte
}

var service string = "127.0.0.1:8889"
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
	request.tm = time.Now().UnixNano() / 1e6
	request.len = 12 + len(data) + 8
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

	binerr = binary.Write(sendbuf, binary.LittleEndian, request.tm)
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
func response(conn net.Conn, vt []int) int {
	buf := make([]byte, 1024)
	_, err := conn.Read(buf)
	if err != nil {
		fmt.Println("Server is dead ...", err)
		conn.Close()
		return -1
	}

	cmd := binary.LittleEndian.Uint32(buf[4:8])
	if cmd != uint32(LoginRsp) {
		tm := binary.LittleEndian.Uint64(buf[12:20])
		endtime := time.Now().UnixNano() / 1e6
		and := endtime - int64(tm)
		index := uint32(and)
		vt[index]++
	}

	//	fmt.Printf("recv  rsp cmd:%d\n", cmd)
	return int(cmd)
}

func worker(ch chan int, vt []int) {
	conn, err := net.Dial("tcp", service)
	checkError(err)
	request(conn, LoginReq)
	if response(conn, vt) != int(LoginRsp) {
		fmt.Println("login fail")
		ch <- -1
		return
	}
	fmt.Println("login success")
	for i := 0; i < num; i++ {

		request(conn, Msg)
		if response(conn, vt) != int(MsgAck) {
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
	vts := make([][]int, *threads)

	for i := 0; i < *threads; i++ {
		vts[i] = make([]int, 1000)
	}
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
		go worker(chs[i], vts[i])
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
	var vet [1000]int
	for i := 0; i < *threads; i++ {
		for j, value := range vts[i] {
			vet[j] += value
		}
	}

	var num int = 0

	for i, value := range vet {
		num += value
		fmt.Printf("<%v ms: %v pkts(%0.2f%%)\n", i, num, (float32(num)/float32(total))*100)
		if num == total {
			break
		}
	}
}
