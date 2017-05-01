// test_server.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <k0/net/tcp/msg_server.hpp>
typedef k0::byte_t byte_t;
typedef k0::bytes::bytes_t bytes_t;
typedef boost::shared_ptr<k0::bytes::bytes_t> bytes_spt;


typedef k0::net::tcp::msg_server_t<int> msg_server_t;

class server_t:public msg_server_t
{
public:
	server_t(const std::string& addr)
		:msg_server_t(addr)
	{
	}

	virtual void on_accept(socket_spt& s)
	{
		std::cout<<"one in : "<<s->socket().remote_endpoint()<<"\n";

	}
	virtual void on_close(socket_spt& s)
	{
		std::cout<<"one out : "<<s->socket().remote_endpoint()<<"\n";
	}
	virtual bool on_msg(socket_spt& s,bytes_spt& msg)
	{
		std::size_t n = msg->size() - 4;
		std::string str((char*)msg->get()+4,n);
		//std::cout<<str<<"\n";


		if(str == "i'm king")
		{
			return push_str(s, "oh!master what can i do");
		}
		else if(str == "good,i need some soldier")
		{
			return push_str(s, "i'm a soldier");
		}
		else if(str == "welcome dog")
		{
			return push_str(s, "oh!master what can i do");
		}
		exit(0);
		return false;
	}
	bool push_str(socket_spt& s,const std::string& str)
	{
		bytes_spt buf;
		try
		{
			k0::uint32_t size = (k0::uint32_t)str.size() + 4;
			buf = boost::make_shared<bytes_t>(size);
			byte_t* ptr = buf->get();
			*(k0::uint32_t*)ptr = size;
			ptr += 4;
			
			memcpy(ptr,str.data(),str.size());
		}
		catch(...)
		{
			puts("push_str error");
			return false;
		}
		return this->push_send(s,buf);
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		std::string addr = ":1102";
		server_t s(addr);
		std::cout<<"work at : "<<addr<<std::endl;
		
		s.join();		
	}
	catch(const k0::exception& e)
	{
		std::cout<<e.what()<<std::endl;
	}

	std::system("pause");
	return 0;
}

