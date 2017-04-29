#ifndef KING_LIB_HEADER_NET_TCP_TYPE
#define KING_LIB_HEADER_NET_TCP_TYPE

#include <k0/bytes/type.hpp>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>

#include <list>
namespace k0
{
namespace net
{
namespace tcp
{
	/**
	*	\brief asio 服務器
	*/
    typedef boost::asio::io_service io_service_t;
    
	/**
	*	\brief asio 接受器
	*/
	typedef boost::asio::ip::tcp::acceptor acceptor_t;

	/**
	*	\brief byte 字節 定義
	*/
	typedef k0::byte_t byte_t;

	/**
	*	\brief 網路數據 字節數組 智能指針
	*/
    typedef boost::shared_ptr<k0::bytes::bytes_t> bytes_spt;

    /**
	*	\brief 對 boost socket 結構的 擴展
	*
	*	使用 模板 爲 boost socket 綁定 用戶 自定義結構\n
	*	不要使用此結構的 任何 public 成員 這是爲 內部設計的 否則可能將引發未定義行爲\n
	*	要操作 此 class 始終應該調用其 public function
	*/
    template<typename T>
    class socket_t
    {
    protected:
		/**
		*	\brief boost socket
		*/
        typedef boost::asio::ip::tcp::socket socket_bt;
		/**
		*	\brief boost socket
		*/
        socket_bt _s;

        /**
		*	\brief 用戶 綁定的 自定義結構
		*/
        T _user;
    public:
		/**
		*	\brief 構造 socket
		*/
        explicit socket_t(io_service_t& io_s):_s(io_s),_wait(false)
        {

        }
	private:
		socket_t(const socket_t&);
		socket_t& operator=(const socket_t&);
	public:
		/**
		*	\brief 返回 boost socket 的引用
		*/
        inline socket_bt& socket()
        {
            return _s;
        }
        /**
		*	\brief 返回 綁定結構 的引用
		*/
		virtual T& get_t()
        {
            return _user;
        }
		/**
		*	\brief 返回 socket native 句柄
		*/
        inline std::size_t native()
        {
            return _s.native();
        }

        /**
		*	\brief 待發送數據列表 (不要操作此屬性)
		*/
		std::list<bytes_spt> _datas;
        
		/**
		*	\brief 同步 對象 (不要操作此屬性)
		*/
        boost::mutex _mutex;

		/**
		*	\brief 是否正在 等待上次 write (不要操作此屬性)
		*/
        bool _wait;

    };

};
};
};

#endif // KING_LIB_HEADER_NET_TCP_TYPE
