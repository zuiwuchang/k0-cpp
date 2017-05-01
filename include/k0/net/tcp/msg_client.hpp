#ifndef KING_LIB_HEADER_NET_TCP_MSG_CLIENT
#define KING_LIB_HEADER_NET_TCP_MSG_CLIENT

#include "type.hpp"
#include "exception.hpp"
#include "msg_reader.hpp"
#include "client.hpp"

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
	*	\brief 使用 boost asio 完成的一個 自動解包 客戶端
	*	\param T 與 socket 綁定 的一個 自定義結構
	*	\param N recv 緩衝區大小
	*/
    template<typename T,std::size_t N=1024*4>
    class msg_client_t:public client_t<T,N>
    {
	protected:
		/**
		*	\brief recv 緩存
		*/
		k0::bytes::buffer_t _buffer;
		/**
		*	\brief 消息長度
		*/
		std::size_t _size;
		/**
		*	\brief 消息頭長度
		*/
		std::size_t _header_size;

		/**
		*	\brief 消息頭 緩存
		*/
		byte_t* _header;

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
        explicit msg_client_t(const std::string& addr,std::size_t header_size=4,reader_header_bft reader_header_bf=boost::bind(&msg_client_t::reader_header,_1,_2))
			:client_t(addr),_header_size(header_size),_reader_header_bf(reader_header_bf),_buffer(N),_size(KING_NET_TCP_WAIT_MSG_HEADER),_header(NULL)
        {
			try
			{
				_header = new byte_t[header_size];
			}
			catch(const std::bad_alloc& e)
			{
				KING_NET_TCP_THROW(e);
			}
        }
		virtual ~msg_client_t()
		{
			_io_s.stop();
            _threads.join_all();

			if(_header)
			{
				delete _header;
			}
		}
	private:
        msg_client_t& operator=(const msg_client_t&);
        msg_client_t(const msg_client_t&);
	public:
		/**
		*	\brief 子類實現 當接收到 1個完整消息 時回調
		*	\param msg 數據緩衝區
		*	\return	true 數據處理完畢 false 數據錯誤 斷開連接
		*/
		virtual bool on_msg(bytes_spt& msg)
		{
			return true;
		}
		/**
		*	\brief 不要重載此函數
		*/
		virtual bool on_recv(byte_t* b,std::size_t n)
		{
			//寫入緩存 失敗
			if( n != _buffer.write(b,n))
			{
				//返回false 斷開連接
				return false;
			}

			//解析消息
			while(true)
			{
				std::size_t size = _buffer.size();
				if(!size)
				{
					break;
				}
				//解析消息頭
				if(KING_NET_TCP_WAIT_MSG_HEADER == _size)
				{
					if(size < _header_size)
					{
						//等待消息頭
						return true;
					}
					else
					{
						//讀取消息頭
						if(_header_size != _buffer.copy_to(_header,_header_size))
						{
							return false;
						}

						//解析 消息頭
						_size = _reader_header_bf(_header,_header_size);
						if(_size == KING_NET_TCP_ERROR_MSG || _size < _header_size)
						{
							return false;
						}
					}
				}

				//獲取 body
				if(_size < size)
				{
					//等待 body
					return true;
				}
				try
				{
					bytes_spt buffer = boost::make_shared<k0::bytes::bytes_t>(_size);
					if(buffer->size() != _size)
					{
						return false;
					}
					if(buffer->size() != _buffer.read(buffer->get(),buffer->size()))
					{
						return false;
					}

					//通知用戶
					if(!on_msg(buffer))
					{
						return false;
					}
					_size = KING_NET_TCP_WAIT_MSG_HEADER;
				}
				catch(const std::bad_alloc&)
				{
					return false;
				}

			}
			return true;
		}
    };


};
};
};

#endif // KING_LIB_HEADER_NET_TCP_MSG_CLIENT
