package main

import (
	"encoding/binary"
	"fmt"
	"log"
	"net"
)

func main() {
	addr := ":1102"
	lister, e := net.Listen("tcp", addr)
	if e != nil {
		log.Fatalln(e)
	}
	fmt.Println("listen at ", addr)
	for {
		c, e := lister.Accept()
		if e != nil {
			log.Println(e)
		}
		go doRead(c)
	}
}
func doRead(c net.Conn) {
	defer func() {
		fmt.Println("one out ", c.RemoteAddr())
		c.Close()
	}()
	fmt.Println("one in ", c.RemoteAddr())
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
	if str == "i'm king" {
		return postStr(c, "oh!master what can i do")
	} else if str == "good,i need some soldier" {
		return postStr(c, "i'm a soldier")
	} else if str == "welcome dog" {
		return postStr(c, "oh!master what can i do")
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
