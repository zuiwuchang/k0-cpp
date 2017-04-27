#ifndef KING_LIB_HEADER_NET_EXCEPTION
#define KING_LIB_HEADER_NET_EXCEPTION
#include <k0/exception.hpp>

#define KING_NET_EXCEPTION	"[k0::net::exception]"
#define KING_NET_BAD_ADDRESS	"[k0::net::bad_address]"

namespace k0
{
namespace net
{
	class exception:public k0::exception
	{
		virtual const char* what() const
        {
            return KING_NET_EXCEPTION;
        }
	};
	class bad_address:public k0::exception
	{
		virtual const char* what() const
        {
            return KING_NET_BAD_ADDRESS;
        }
	};
};
};
#endif	//KING_LIB_HEADER_NET_EXCEPTION