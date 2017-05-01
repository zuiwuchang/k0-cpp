package main

import (
	"encoding/binary"
	"fmt"
	"log"
	"net"
)

func main() {
	conns := 1
	addr := "127.0.0.1:1102"
	ch := make(chan bool)
	for i := 0; i < conns; i++ {
		go connect(ch, addr)
	}
	for {
		<-ch
		conns--
		if conns == 0 {
			break
		}
	}

}
func connect(ch chan bool, addr string) {
	c, e := net.Dial("tcp", addr)
	if e != nil {
		ch <- true
		fmt.Println(e)
		return
	}
	fmt.Println("ok")
	doRead(c)

	ch <- true
}
func doRead(c net.Conn) {
	defer c.Close()
	if nil != postStr(c, "good,i need some soldier") {
		return
	}

	b := make([]byte, 1024, 1024)
	reader := Reader{}
	for {
		n, e := c.Read(b)
		if e != nil {
			break
		}
		reader.Write(b[:n])
		for {
			msg := reader.GetMsg()
			if msg == nil {
				break
			}
			e = doMsg(c, msg.GetBody())
			if e != nil {
				log.Println(e)
				return
			}
		}
	}
}
func doMsg(c net.Conn, b []byte) error {
	str := string(b)
	//fmt.Println(str)
	if str == "oh!master what can i do" {
		return postStr(c, "good,i need some soldier")
	} else if str == "i'm a soldier" {
		return postStr(c, "welcome dog")
	}
	return nil
}
func postStr(c net.Conn, str string) error {
	return postMsg(c, []byte(str))
}
func postMsg(c net.Conn, b []byte) error {
	size := len(b) + 4
	dist := make([]byte, size)
	binary.LittleEndian.PutUint32(dist, uint32(size))
	copy(dist[4:], b)
	_, e := c.Write(dist)
	return e
}
