#ifndef KING_LIB_HEADER_NET_TCP_EXCEPTION
#define KING_LIB_HEADER_NET_TCP_EXCEPTION
#include <k0/net/exception.hpp>
#include <string>

#define KING_NET_TCP_EXCEPTION "[k0::net::tcp::exception]"

#define KING_NET_TCP_THROW(e) {\
	std::string emsg(KING_NET_TCP_EXCEPTION);\
	emsg += " ";\
	emsg += e.what();\
	throw exception(emsg);\
}
namespace k0
{
namespace net
{
namespace tcp
{
	/**
	*	\brief tcp 異常
	*/
	class exception:public k0::net::exception
	{
	protected:
		std::string _emsg;
	public:
		exception(const std::string& emsg = std::string(KING_NET_TCP_EXCEPTION))
			:_emsg(emsg)
		{
			
		}
		virtual const char* what() const
        {
            return _emsg.c_str();
        }
	};
	
};
};
};
#endif	//KING_LIB_HEADER_NET_TCP_EXCEPTION