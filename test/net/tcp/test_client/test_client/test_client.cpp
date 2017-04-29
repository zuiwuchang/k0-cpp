// test_client.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <k0/net/tcp/msg_reader.hpp>
#include <k0/net/tcp/msg_client.hpp>

typedef k0::net::tcp::msg_client_t<int> msg_client_t;
typedef k0::byte_t byte_t;
typedef k0::bytes::bytes_t bytes_t;
typedef k0::net::tcp::bytes_spt bytes_spt;
class client_t
	:public msg_client_t
{
public:
	client_t(const std::string& addr):msg_client_t(addr)
	{

	}
	virtual bool on_msg(bytes_spt buffer)
	{
		std::size_t n = buffer->size() - 4;
		std::string str((char*)buffer->get()+4,n);
		//std::cout<<str<<"\n";

		static int v=0;
		if(str == "oh!master what can i do")
		{
			if(v!=0)
			{
				return false;
			}

			push_str("good,i need some soldier");
			v = 1;
		}
		else if (str=="i'm a soldier")
		{	
			if(v!=1)
			{
				return false;
			}
			push_str("welcome dog");
			v=0;
		}

		return true;
	}
	virtual void on_close ()
	{
		puts("socket close");
	}
	bool push_str(const std::string& str)
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
		return this->push_send(buf);
	}
};
	
int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		client_t c("127.0.0.1:1102");
		c.push_str("hi");
		c.push_str("i'm king");

		c.join();
	}
	catch(const k0::exception& e)
	{
		std::cout<<e.what()<<std::endl;
	}
	std::system("pause");
	return 0;
}

