#ifndef KING_LIB_HEADER_NET_TCP_CLIENT
#define KING_LIB_HEADER_NET_TCP_CLIENT

#include "type.hpp"
#include "exception.hpp"

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
	*	\brief 使用 boost asio 完成的一個 客戶端
	*	\param T 與 socket 綁定 的一個 自定義結構
	*/
    template<typename T>
    class client_t
    {
    protected:
		/**
		*	\brief socket 定義
		*/
        typedef socket_t<T> socket_t;
		/**
		*	\brief socket 智能指針
		*/
        typedef boost::shared_ptr<socket_t> socket_spt;

		
        /**
		*	\brief asio 服務
		*/
        io_service_t _io_s;
		
		/**
		*	\brief 與服務器的連接 socket
		*/
        socket_spt _socket;
		
		/**
		*	\brief 工作 線程
		*/
        boost::thread_group _threads;
	private:
        void work_thread()
        {
            _io_s.run();
        }
    public:
		/**
		*	\brief 構造 client 並連接到指定 地址
		*	\param addr 形如 dns:port 的服務器 地址
		*	\return throw k0::net::bad_address k0::net::tcp::exception
		*/
        explicit client_t(const std::string& addr,const std::size_t buffer = 1024 * 4)
        {
			//驗證 地址
			std::string::size_type find = addr.find_last_of(':');
			if(find == std::string::npos)
			{
				throw k0::net::bad_address();
			}
			std::string dns = addr.substr(0,find);
			if(dns.empty())
			{
				throw k0::net::bad_address();
			}
			std::string sport = addr.substr(find+1);
			unsigned short port = 0;
			try
			{
				port = boost::lexical_cast<unsigned short>(sport);
			}
			catch(const boost::bad_lexical_cast& )
			{
				
			}
			if(port == 0)
			{
				throw k0::net::bad_address();
			}

			socket_spt s;
			try
			{
				//連接 socket
				s = boost::make_shared<socket_t>(_io_s);
				s->socket().connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(dns),port));
			}
			catch(const std::bad_alloc& e)
			{
				KING_NET_TCP_THROW(e);
			}
			catch(const boost::system::system_error& e)
			{
				KING_NET_TCP_THROW(e);
			}
			
            //創建 recv 緩衝區
            bytes_spt buf;
			try
			{
			   buf = boost::make_shared<k0::bytes::bytes_t>(buffer);
			}
			catch(const std::bad_alloc& e)
			{
				KING_NET_TCP_THROW(e);
			}
            _socket = s;

			
            //異步 recv
			post_recv(buf);


			//啓動工作線程
            _threads.add_thread(new boost::thread(boost::bind(&client_t::work_thread,this)));
        }
	private:
        client_t& operator=(const client_t&);
        client_t(const client_t&);
	public:
		/**
		*	\brief 析構 關閉連接 釋放資源
		*/
        virtual ~client_t()
        {
            _io_s.stop();
            _threads.join_all();
        }
		/**
		*	\brief 子類實現 當和服務器斷開時回調
		*/
		virtual void on_close()
		{
		}
		/**
		*	\brief 子類實現 當接收到數據時回調
		*/
		virtual bool on_recv()
		{
		}
    protected:
		/**
		*	\brief 異步讀取數據
		*/
        inline void post_recv(bytes_spt buffer)
        {
            _socket->socket().async_read_some(boost::asio::buffer(buffer->get(),buffer->size()),
                boost::bind(&client_t::post_recv_handler,
                this,
                boost::asio::placeholders::error,
                buffer,
                boost::asio::placeholders::bytes_transferred)
            );
        }
		
        void post_recv_handler(const boost::system::error_code& e,bytes_spt buffer,std::size_t n)
        {
           if(e)
            {
                //通知 用戶
                on_close();
                

                //錯誤 斷開 連接
                if(_socket->socket().is_open())
                {
                    boost::system::error_code e0;
                    _socket->socket().close(e0);
                }
                return;
            }
		    /*
            //通知 用戶
            if(_recv_bf)
            {
                _recv_bf(this,s,buffer->get(),n);
            }
			*/
            //投遞 新的 recv
            post_recv(buffer);
        }

    public:
  
		/*
        //返回 工作 線程 數量
        inline std::size_t work_threads()const
        {
            return _threads.size();
        }

        //等待 線程 停止 工作
        inline void join()
        {
            _threads.join_all();
        }

        //向 服務器 發送 隊列 寫入一條 發送 數據
        bool push_back_write(socket_spt s,const byte_t* bytes,std::size_t n)
        {
            if(!s->socket().is_open())
            {
                return false;
            }

            //創建 write 緩衝區
            bytes_spt buffer;
            try
            {
                buffer = std::make_shared<king::bytes::bytes_t>(n);
                if(buffer->empty())
                {
                    //創建 失敗
                    return false;
                }
            }
            catch(const std::bad_alloc&)
            {
                //創建 失敗
                return false;
            }
            //copy 待write 數據
            std::copy(bytes,bytes+n,buffer->get());

            return push_back_write(s,buffer);
        }
        bool push_back_write(socket_spt s,bytes_spt buffer)
        {
            if(!s->socket().is_open())
            {
                return false;
            }

            boost::mutex::scoped_lock lock(s->_mutex);
            auto& datas = s->_datas;
            auto& wait = s->wait;

            //等待 上次 write 完成 直接 push
            if(wait)
            {
                datas.push_back(buffer);
                return true;
            }
            else
            {
                //需要 發送 數據

                if(datas.empty())
                {
                    //直接 write
                    post_write(s,buffer);
                    wait = true;
                    return true;
                }
                else
                {
                    try
                    {
                        //寫入 隊列
                        datas.push_back(buffer);

                        //發送 隊列 首數據
                        buffer = datas.front();
                        datas.pop_front();
                        post_write(s,buffer);
                        wait = true;
                        return true;
                    }
                    catch(const std::bad_alloc&)
                    {
                        return false;
                    }

                }

            }
            return false;
        }
    protected:
        inline void post_write(socket_spt s,bytes_spt buffer)
        {
            s->socket().async_write_some(boost::asio::buffer(buffer->get(),buffer->size()),
                boost::bind(&client_t::post_write_handler,
                this,
                boost::asio::placeholders::error,
                s,
                buffer
                )
            );
        }
        void post_write_handler(const boost::system::error_code& e,socket_spt s,bytes_spt buffer)
        {
            if(e)
            {
                //send 錯誤 直接 關閉
                if(s->socket().is_open())
                {
                    boost::system::error_code e0;
                    s->socket().close(e0);
                }
                return;
            }

            //通知 客戶
            if(_send_bf)
            {
                _send_bf(this,s,buffer);
            }


            boost::mutex::scoped_lock lock(s->_mutex);
            auto& datas = s->_datas;
            if(datas.empty())
            {
                //接受 數據 發送
                s->wait = false;
                return;
            }
            //繼續 發送 數據
            buffer = datas.front();
            datas.pop_front();
            post_write(s,buffer);

        }
		*/
    };


};
};
};

#endif // KING_LIB_HEADER_NET_TCP_CLIENT
