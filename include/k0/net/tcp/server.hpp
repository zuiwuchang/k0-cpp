//一個 asio tcp 服務器
#ifndef KING_LIB_HEADER_NET_TCP_SERVER
#define KING_LIB_HEADER_NET_TCP_SERVER

#include "type.hpp"


#include <iostream>

#include <boost/bind.hpp>


namespace k0
{
namespace net
{
namespace tcp
{
	/**
	*	\brief 使用 boost asio 完成的一個 服務器
	*	\param T 與 socket 綁定 的一個 自定義結構
	*	\param N recv 緩衝區大小
	*/
    template<typename T,std::size_t N=1024*4,typename TP=std::size_t>
    class server_t
    {
	public:
		/**
		*	\brief socket 定義
		*/
        typedef socket_t<T,TP> socket_t;
		/**
		*	\brief socket 智能指針
		*/
        typedef std::shared_ptr<socket_t> socket_spt;
	protected:
		/**
		*	\brief asio 服務
		*/
        io_service_t _io_s;

		/**
		*	\brief 運行的最大連接數量
		*/
		std::size_t _max;

		/**
		*	\brief 線程數
		*/
		std::size_t _count;

		/**
		*	\brief 成功連接數量
		*/
		std::size_t _conns;

		/**
		*	\brief 待連接數量
		*/
		std::size_t _accepts;
		
		/**
		*	\brief recv 同步對象
		*/
		boost::mutex _mutex;

		/**
		*	\brief recv 緩衝區 大小
		*/
        std::size_t _buffer;

		/**
		*	\brief 連接 接受器
		*/
        acceptor_t* _acceptor;

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
		*	\brief 構造 server_t 並監聽指定 地址
		*	\param addr 形如 dns:port 的服務器 地址
		*	\param conns 最大的連接數量
		*	\return throw k0::net::bad_address k0::net::tcp::exception
		*/
        explicit server_t(const std::string& addr,const std::size_t conns=1024)
			:_acceptor(NULL),
			_max(conns),
			_conns(0),
			_accepts(0)
        {
			//驗證 地址
			std::string::size_type find = addr.find_last_of(':');
			if(find == std::string::npos)
			{
				throw k0::net::bad_address();
			}
			std::string dns = addr.substr(0,find);
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


			//監聽服務器
			try
			{
				if("" == dns)
				{
					_acceptor = new acceptor_t(_io_s,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port));
				}
				else
				{
					_acceptor = new acceptor_t(
						_io_s,
						boost::asio::ip::tcp::endpoint(
							boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(dns),port)
						)
					);
				}

				//線程數
				_count = (boost::thread::hardware_concurrency() + 1 ) * 2; 
				
				
				//異步 接受 連接
				post_accepts();
				

				//啓動工作線程
				for(std::size_t i = 0 ; i < _count ; ++i)
				{
					_threads.add_thread(new boost::thread(boost::bind(&server_t::work_thread,this)));
				}
			}
			catch(const std::bad_alloc& e)
			{
				KING_NET_TCP_THROW(e);
			}
			catch(const boost::system::system_error& e)
			{
				KING_NET_TCP_THROW(e);
			}
        }
	private:
        server_t& operator=(const server_t&);
        server_t(const server_t&);
	public:
		/**
		*	\brief 析構 關閉連接 釋放資源
		*/
		virtual ~server_t()
        {
            _io_s.stop();
            _threads.join_all();

			if(_acceptor)
			{
				delete _acceptor;
			}
        }
		/**
		*	\brief 子類實現 當和客戶端成功連接後回調
		*/
		virtual void on_accept(socket_spt& s)
		{
		}
		/**
		*	\brief 子類實現 客戶端斷開前回調
		*/
		virtual void on_close(socket_spt& s)
		{
		}
		/**
		*	\brief 子類實現 當接收到數據時回調
		*	\param s 收到數據的 socket
		*	\param b 數據緩衝區
		*	\param n 數據長度
		*	\return	true 數據處理完畢 false 數據錯誤 斷開連接
		*/
		virtual bool on_recv(socket_spt& s,byte_t* b,std::size_t n)
		{
			return true;
		}
		
		/**
		*	\brief 子類實現 當數據發送成功後 回調
		*	\param s 發送數據的 socket
		*	\param buffer 被發送的 數據
		*/
		virtual void on_send(socket_spt& s,bytes_spt& buffer)
		{
		}
	protected:
		/**
		*	\brief 返回 最大 接受連接數
		*/
		inline std::size_t max()const
		{
			return _max;
		}
		/**
		*	\brief 設置 最大 接受連接數
		*/
		inline void max(const std::size_t n)
		{
			_max = n;
		}

		/**
		*	\brief 異步接受連接
		*/
		void post_accepts()
        {
			boost::mutex::scoped_lock lock(_mutex);
			std::size_t count = _count;
			if(_accepts >= count)
			{
				return;
			}
			count -= _accepts;

			//異步 接受連接
			for(std::size_t i = 0 ; i < count ; ++i)
			{
					post_accept();
			}
			
        }
		/**
		*	\brief 異步接受連接
		*/
        void post_accept()
        {
			try
			{
				socket_spt s = std::make_shared<socket_t>(_io_s);
				_acceptor->async_accept(s->socket(),
					boost::bind(&server_t::post_accept_handler,
					this,
					boost::asio::placeholders::error,
					s)
				);
				++_accepts;
			}
			catch(const std::bad_alloc& e)
			{
				std::cout<<"post_accept error : "<<e.what()<<"\n";
			}
        }
		/**
		*	\brief 連接處理器
		*/
        void post_accept_handler(const boost::system::error_code& e,socket_spt s)
        {
			//減少 _accepts 計數
			_mutex.lock();
			--_accepts;
			_mutex.unlock();

            //投遞 新的 接受 操作
            post_accepts();

            //連接錯誤 直接返回
            if(e)
            {
                return;
            }

            //創建 recv 緩衝區
            bytes_spt buffer;
            try
            {
                buffer = boost::make_shared<k0::bytes::bytes_t>(N);
            }
            catch(const std::bad_alloc&)
            {
                //創建 recv 緩衝區失敗 直接 斷開連接
                return;
            }
			
			//超過 最大連接 不再 接受新連接
			if(_conns >= _max)
			{
				//關閉 連接 釋放資源
				boost::system::error_code e0;
                s->socket().close(e0);
				return;
			}

			//增加 coons 計數
			_mutex.lock();
			++_conns;
			_mutex.unlock();

            //通知 用戶
			on_accept(s);
            

            //投遞 異步 recv
            post_recv(s,buffer);
        }
		
		/**
		*	\brief 異步讀取數據
		*/
		inline void post_recv(socket_spt s,bytes_spt buffer)
        {
            s->socket().async_read_some(boost::asio::buffer(buffer->get(),buffer->size()),
                boost::bind(&server_t::post_recv_handler,
                this,
                boost::asio::placeholders::error,
                s,
                buffer,
                boost::asio::placeholders::bytes_transferred)
            );
        }
        /**
		*	\brief 讀取處理器
		*/
		void post_recv_handler(const boost::system::error_code& e,socket_spt s,bytes_spt buffer,std::size_t n)
        {
            if(e)
            {
                //通知 用戶
                on_close(s);

                //錯誤 斷開 連接
                if(s->socket().is_open())
                {
                    boost::system::error_code e0;
                    s->socket().close(e0);
                }

				//減少 conns 計數
				_mutex.lock();
				--_conns;
				_mutex.unlock();

				post_accepts();
                return;
            }
			
            //通知 用戶
			if(!on_recv(s,buffer->get(),n))
			{
				//協議錯誤 直接斷開連接

				//通知 用戶
                on_close(s);

				//錯誤 斷開
                if(s->socket().is_open())
                {
                    boost::system::error_code e0;
                    s->socket().close(e0);
                }

				//減少 conns 計數
				_mutex.lock();
				--_conns;
				_mutex.unlock();

				post_accepts();
				return;
			}
         

            //投遞 新的 recv
            post_recv(s,buffer);
        }
    
	public:
        /**
		*	\brief 返回 工作 線程 數量
		*/
        inline std::size_t work_threads()const
        {
            return _threads.size();
        }

        /**
		*	\brief 等待 線程 停止 工作
		*/
        inline void join()
        {
            _threads.join_all();
        }
		/**
		*	\brief 向 客戶端 發送 隊列 寫入一條 發送 數據
		*/
        bool push_send(socket_spt s,const byte_t* bytes,std::size_t n)
        {
            if(!s->socket().is_open())
            {
                return false;
            }

            //創建 write 緩衝區
            bytes_spt buffer;
            try
            {
                buffer = boost::make_shared<k0::bytes::bytes_t>(n);
            }
            catch(const std::bad_alloc&)
            {
                //創建 失敗
                return false;
            }
            //copy 待write 數據
            std::copy(bytes,bytes+n,buffer->get());

            return push_send(s,buffer);
        }
        /**
		*	\brief 向 客戶端 發送 隊列 寫入一條 發送 數據
		*/
		bool push_send(socket_spt s,bytes_spt buffer)
        {
            if(!s->socket().is_open())
            {
                return false;
            }

            boost::mutex::scoped_lock lock(s->_mutex);
            std::list<bytes_spt>& datas = s->_datas;
            bool& wait = s->_wait;

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
                    post_send(s,buffer);
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
                        post_send(s,buffer);
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
		/**
		*	\brief 異步 發送 數據
		*/
        inline void post_send(socket_spt s,bytes_spt buffer)
        {
            s->socket().async_write_some(boost::asio::buffer(buffer->get(),buffer->size()),
                boost::bind(&server_t::post_send_handler,
                this,
                boost::asio::placeholders::error,
                s,
                buffer
                )
            );
        }
        /**
		*	\brief 發送 處理器
		*/
		void post_send_handler(const boost::system::error_code& e,socket_spt s,bytes_spt buffer)
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
            on_send(s,buffer);
			
            boost::mutex::scoped_lock lock(s->_mutex);
			std::list<bytes_spt>& datas = s->_datas;
            if(datas.empty())
            {
                //接受 數據 發送
                s->_wait = false;
                return;
            }
            //繼續 發送 數據
            buffer = datas.front();
            datas.pop_front();
            post_send(s,buffer);
        }

    };


};
};
};

#endif // KING_LIB_HEADER_NET_TCP_SERVER
