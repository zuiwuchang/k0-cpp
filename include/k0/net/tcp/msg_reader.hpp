//消息解析器
#ifndef KING_LIB_HEADER_NET_TCP_MSG_READER
#define KING_LIB_HEADER_NET_TCP_MSG_READER

#include <k0/bytes/buffer.hpp>
#include <boost/function.hpp>


namespace k0
{
namespace net
{
namespace tcp
{

/**
*	\brief 消息錯誤
*/
#define KING_NET_TCP_ERROR_MSG          ((std::size_t)-1)
/**
*	\brief 等待消息頭
*/
#define KING_NET_TCP_WAIT_MSG_HEADER    ((std::size_t)-2)

/**
*	\brief 最大 消息長
*/
#ifndef KING_NET_TCP_MAX_MSG_SIZE
#define KING_NET_TCP_MAX_MSG_SIZE   1024 * 10
#endif
	/**
	*	\brief 消息解析器 用於解析 tcp黏包
	*/
    class msg_reader_t
    {
    protected:
		/**
		*	\brief 包頭長度
		*/
        std::size_t _header_size;
    public:
		/**
		*	\brief 包頭解析函數
		*	成功返回 消息長度
		*	返回 KING_NET_TCP_ERROR_MSG	協議錯誤 斷開tcp
		*	返回 KING_NET_TCP_WAIT_MSG_HEADER 等待 header
		*/
        typedef boost::function<std::size_t(const byte_t*,std::size_t)> reader_header_bft;
    protected:
        /**
		*	\brief 包頭解析函數
		*/
		reader_header_bft _reader_header_bf;
    public:
		/**
		*	\brief 創建消息解析器
		*	\param header_size 指定消息頭長度
		*	\param reader_header_bf 消息解析函數
		*/
        explicit msg_reader_t(const std::size_t header_size,reader_header_bft reader_header_bf)
            :_header_size(header_size),
            _reader_header_bf(reader_header_bf)
        {

        }

		/**
		*	\brief 返回 包頭長度
		*/
        inline std::size_t header_size()const
        {
            return _header_size;
        }

		/**
		*	\brief 解析消息頭
		*	\param b 消息頭 緩衝區
		*	\return	成功返回 消息長度 (header + body) 或 KING_NET_TCP_ERROR_MSG 或 KING_NET_TCP_WAIT_MSG_HEADER
		*/
        inline std::size_t reader_header(const byte_t* b,std::size_t n)const
        {
            return _reader_header_bf(b,n);
        }

    };

	/*
	//解析器 分片大小
    template<typename T,std::size_t N>
    class msg_data
    {
    protected:
        T _user;
        std::shared_ptr<king::bytes::buffer_t> _buffer;
    public:
        std::size_t _size = KING_NET_TCP_WAIT_MSG_HEADER; // 消息 長度

        msg_data()
        {
            _buffer = std::make_shared<king::bytes::buffer_t>();
        }
        inline T& get_t()
        {
            return _user;
        }
        inline std::shared_ptr<king::bytes::buffer_t> buffer()const
        {
            return _buffer;
        }

    };
	*/
};
};
};

#endif // KING_LIB_HEADER_NET_TCP_MSG_READER
