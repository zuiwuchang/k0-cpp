#ifndef KING_LIB_HEADER_NET_TCP_MSG_SERVER
#define KING_LIB_HEADER_NET_TCP_MSG_SERVER

#include "type.hpp"
#include "exception.hpp"
#include "msg_reader.hpp"
#include "server.hpp"

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>



namespace k0
{
namespace net
{
namespace tcp
{
	/**
	*	\brief 使用 消息解析狀態 只在內部使用
	*/
	class msg_buffer_t
	{
	public:
		typedef k0::bytes::buffer_t buffer_t;
		/**
		*	\brief 緩衝區
		*/
		buffer_t buffer;
		/**
		*	\brief 當前讀取狀態
		*/
		std::size_t size;
		msg_buffer_t(std::size_t capacity):size(KING_NET_TCP_WAIT_MSG_HEADER),buffer(capacity)
		{
		}
	};
	typedef boost::shared_ptr<msg_buffer_t> msg_buffer_spt;
	/**
	*	\brief 使用 boost asio 完成的一個 自動解包 客戶端
	*	\param T 與 socket 綁定 的一個 自定義結構
	*	\param N recv 緩衝區大小
	*/
    template<typename T,std::size_t N=1024*4,typename TP=msg_buffer_spt>
    class msg_server_t:public server_t<T,N,TP>
    {
	protected:
		/**
		*	\brief 消息緩衝區 定義
		*/
		typedef boost::shared_ptr<k0::bytes::buffer_t> buffer_spt; 
		/**
		*	\brief 消息頭長度
		*/
		std::size_t _header_size;

		/**
		*	\brief 包頭解析函數
		*	成功返回 消息長度
		*	返回 KING_NET_TCP_ERROR_MSG	協議錯誤 斷開tcp
		*/
        typedef boost::function<std::size_t(const byte_t*,std::size_t)> reader_header_bft;
		/**
		*	\brief 默認 包頭解析函數
		*	\param b 消息頭 緩衝區
		*	\param n 緩衝區大小
		*	\return	成功返回 消息長度 失敗返回 KING_NET_TCP_ERROR_MSG
		*/
		static std::size_t reader_header(const byte_t* b,std::size_t n)
		{
			std::size_t size = (std::size_t)(*(k0::uint32_t*)b);
			if(size > KING_NET_TCP_MAX_MSG_SIZE)
			{
				return KING_NET_TCP_ERROR_MSG;
			}
			return size;
		}
		/**
		*	\brief 包頭解析函數
		*/
		reader_header_bft _reader_header_bf;
	public:
		/**
		*	\brief 構造 client 並連接到指定 地址
		*	\param addr 形如 dns:port 的服務器 地址
		*	\return throw k0::net::bad_address k0::net::tcp::exception
		*/
        explicit msg_server_t(const std::string& addr,const std::size_t conns=1024,std::size_t header_size=4,reader_header_bft reader_header_bf=boost::bind(&msg_server_t::reader_header,_1,_2))
			:server_t(addr,conns),_header_size(header_size),_reader_header_bf(reader_header_bf)
        {
			
        }
		virtual ~msg_server_t()
		{
		}
	private:
        msg_server_t& operator=(const msg_server_t&);
        msg_server_t(const msg_server_t&);
	public:
		/**
		*	\brief 子類實現 當接收到 1個完整消息 時回調
		*	\param s 接受到消息的 socket
		*	\param msg 數據緩衝區
		*	\return	true 數據處理完畢 false 數據錯誤 斷開連接
		*/
		virtual bool on_msg(socket_spt& s,bytes_spt& msg)
		{
			return true;
		}

		/**
		*	\brief 不要重載此函數
		*/
		virtual bool on_recv(socket_spt& s,byte_t* b,std::size_t n)
		{
			try
			{
				//創建 消息 緩衝區
				msg_buffer_spt tp = s->_tp;
				if(!tp)
				{
					tp = boost::make_shared<msg_buffer_t>(N);
					s->_tp = tp;
				}
				msg_buffer_t::buffer_t& buffer = tp->buffer;
				std::size_t& size = tp->size;

				//寫入緩存 失敗
				if( n != buffer.write(b,n))
				{
					//返回false 斷開連接
					return false;
				}

				//解析 消息
				//解析消息
				while(true)
				{
					std::size_t size_buffer = buffer.size();
					if(!size_buffer)
					{
						break;
					}
					//解析消息頭
					if(KING_NET_TCP_WAIT_MSG_HEADER == size)
					{
						if(size_buffer < _header_size)
						{
							//等待消息頭
							return true;
						}
						else
						{
							
							boost::shared_array<byte_t> header(new byte_t[_header_size]);
							//讀取消息頭
							if(_header_size != buffer.copy_to(header.get(),_header_size))
							{
								return false;
							}
							
							//解析 消息頭
							size = _reader_header_bf(header.get(),_header_size);
							if(size == KING_NET_TCP_ERROR_MSG || size < _header_size)
							{
								return false;
							}
						}
					}
					

					//獲取 body
					if(size < size_buffer)
					{
						//等待 body
						return true;
					}
					
					//獲取 消息
					bytes_spt msg = boost::make_shared<k0::bytes::bytes_t>(size);
					if(msg->size() != size)
					{
						return false;
					}
					if(size != buffer.read(msg->get(),size))
					{
						return false;
					}

					//通知用戶
					if(!on_msg(s,msg))
					{
						return false;
					}
					size = KING_NET_TCP_WAIT_MSG_HEADER;
				}
			}
			catch(const std::bad_alloc&)
			{
				return false;
			}

			return true;
		}
    };


};
};
};

#endif // KING_LIB_HEADER_NET_TCP_MSG_SERVER
