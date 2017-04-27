#ifndef KING_LIB_HEADER_BYTES_BUFFER
#define KING_LIB_HEADER_BYTES_BUFFER

#include <list>

#include <boost/foreach.hpp>

#include "type.hpp"
namespace k0
{
namespace bytes
{

/**
*   \brief k0::byte_t 流緩衝區
*
*   一個類似 golang bytes.Buffer 的 io 緩衝區
*/
class buffer_t
{
public:
	/** 
    *    \brief 字節定義
    */
    typedef k0::byte_t byte_t;
	/** 
    *   \brief 分片定義
    */
    typedef boost::shared_ptr<fragmentation_t> fragmentation_spt;
protected:
    /** 
    *   \brief 創建分片時 分片的參考大小
    */
    int _capacity;

    /** 
    *   \brief 分片 緩存
    */
    std::list<fragmentation_spt> _fragmentations;

    /** 
    *   \brief 緩存一個 分片 以便 可以重複利用
    */
	fragmentation_spt _cache;
public:
	/** 
    *   \brief 構造一個 緩衝區
    *   \param capacity 當需要創建新分片時 分片參考大小
    */
    explicit buffer_t(int capacity = 1024):_capacity(capacity)
    {
    }
private:
	buffer_t& operator=(const buffer_t&);
    buffer_t(const buffer_t&);
public:
    /** 
    *   \brief 清空流中的 數據
    *   \param clearcache  是否刪除 this._cache
    */
    void reset(bool clearcache = true)
    {
        if(clearcache)
        {
            _cache.reset();
        }
        else
        {
            if(!_cache && !_fragmentations.empty())
            {
                _cache = _fragmentations.back();
            }
        }
        _fragmentations.clear();
    }
    /** 
    *   \brief 刪除 this._cache
    */
    inline void reset_cache()
    {
         _cache.reset();
    }

    /** 
    *   \brief 返回 流中 待讀字節數
    */
    std::size_t size()const
    {
       std::size_t sum = 0;
       BOOST_FOREACH(const fragmentation_spt& f,_fragmentations)
       {
           sum += f->size();
       }
       return sum;
    }

    /** 
    *   \brief 向流中 寫入數據
    *
    *   如果 發生任何 錯誤 將 不寫入 數據 並返回 0
    *   \param bytes    待 寫入字節指針
    *   \param n    寫入長度
    *   \return 實際寫入長度 
    */
    std::size_t write(const byte_t* bytes,const std::size_t n)
    {
        if(!n)
        {
            return 0;
        }

        if (_fragmentations.empty())
        {
            //1次 寫入
            fragmentation_spt f = create_fragmentation(n);
            if(!f)
            {
                //創建 分片 失敗
                return 0;
            }

            try
            {
                //添加新 分片
                _fragmentations.push_back(f);

                //寫入 分片
                f->write(bytes,n);
            }
            catch(const std::bad_alloc&)
            {
                return 0;
            }

            return n;
        }

        fragmentation_spt& f0 = _fragmentations.back();
        std::size_t free = f0->get_free();
        if(free >= n)
        {
            //1次 寫入
            f0->write(bytes,n);
            return n;
        }

        //2次 寫入
        std::size_t need = n - free;
        std::size_t capacity = _capacity;
        if(capacity < need)
        {
            capacity = need;
        }
        //創建 新分片
        fragmentation_spt f = create_fragmentation(capacity);
        if(!f)
        {
            //創建 分片 失敗
            return 0;
        }
        try
        {
            //添加新 分片
            _fragmentations.push_back(f);

            //寫入 數據
            f0->write(bytes,free);
            f->write(bytes + free,need);
        }
        catch(const std::bad_alloc&)
        {
            return 0;
        }

        return n;
    }
protected:
    /** 
    *   \brief 創建 一個大小至少為 capacity 的分片
    *
    *   \param capacity  分片參考大小
    */
    fragmentation_spt create_fragmentation(const std::size_t capacity)
    {
        try
        {
            if(!_cache)
            {
                return boost::make_shared<fragmentation_t>(capacity);
            }
            if(_cache->capacity() < capacity)
            {
                return boost::make_shared<fragmentation_t>(capacity);
            }
        }
        catch(const std::bad_alloc&)
        {
            return fragmentation_spt();
        }

        fragmentation_spt f = _cache;
        f->init();
        _cache.reset();

        return f;
    }

public:
    /** 
    *   \brief 將緩衝區 copy 到指定內存 返回實際 copy數據長
    *
    *   被copy的數據 不會從 緩衝區中 刪除\n
    *   如果 n > 緩衝區 數據 將 只copy 緩衝區 否則 copy n 字節數據
    *
    *   \param bytes    待 讀緩衝區
    *   \param n    緩衝區長度
    *   \return 返回實際 copy數據長
    */
    std::size_t copy_to(byte_t* bytes,std::size_t n)const
    {
        std::size_t sum = 0;
        std::size_t count;
        BOOST_FOREACH(const fragmentation_spt& f,_fragmentations)
        {
            count = f->copy_to(bytes,n);
            n -= count;
            bytes += count;
            sum += count;

            if(n < 1)
            {
                break;
            }
        }
        return sum;
    }
    /** 
    *   \brief 將緩衝區 copy 到指定內存 返回實際 copy數據長
    *
    *   被copy的數據 不會從 緩衝區中 刪除\n
    *   如果 n > 緩衝區 數據 將 只copy 緩衝區 否則 copy n 字節數據
    *
    *   \param skip    忽略 Buffer 中前skip個字節
    *   \param bytes    待 讀緩衝區
    *   \param n    緩衝區長度
    *   \return 返回實際 copy數據長
    */
	std::size_t copy_to(std::size_t skip,byte_t* bytes,std::size_t n)const
    {
        std::size_t sum = 0;
        std::size_t need_skip = 0;
        std::size_t size;
        std::size_t count;
        BOOST_FOREACH(const fragmentation_spt& f,_fragmentations)
        {
            if(skip)
            {
                need_skip = skip;

                size = f->size();
                if(need_skip > size)
                {
                    need_skip = size;
                }
                skip -= need_skip;

            }
            if(need_skip)
            {
                count = f->copy_to(need_skip,bytes,n);
            }
            else
            {
                count = f->copy_to(bytes,n);
            }
            n -= count;
            bytes += count;
            sum += count;

            if(n < 1)
            {
                break;
            }
        }
        return sum;
    }


    /** 
    *   \brief 從流中 讀取數據 被讀取的數據 將被刪除
    *
    *   \param bytes    待 讀緩衝區
    *   \param n    緩衝區長度
    *   \return 實際 讀取數據流 長度
    */
    std::size_t read(byte_t* bytes,std::size_t n)
    {
        std::size_t sum = 0;
        std::size_t count;
        while(n)
        {
            //返回 可讀 分片
            fragmentation_spt f = get_read_fragmentation();
            if(!f)
            {
                break;
            }
            count = f->read(bytes,n);
            n -= count;
            bytes += count;
            sum += count;
        }
        remove_no_read_fragmentation();

        return sum;
    }
protected:
	/** 
    *   \brief 返回當前 可讀 緩衝區 或 空指針
    */
    fragmentation_spt get_read_fragmentation()
    {
        while(!_fragmentations.empty())
        {
            const fragmentation_spt& f = _fragmentations.front();
            if(f->size() < 1)
            {
                cache_fragmentation(f);
                _fragmentations.pop_front();
                continue;
            }
            return f;
        }

        return fragmentation_spt();
    }
    /** 
    *   \brief 將 f 設置爲 緩存
    */
    inline void cache_fragmentation(const fragmentation_spt& f)
    {
        if(!_cache || _cache->size() < f->size())
        {
            _cache = f;
        }
    }

    /** 
    *   \brief 刪除 無數據可讀的 分片
    */
    void remove_no_read_fragmentation()
    {
        while(!_fragmentations.empty())
        {
            const fragmentation_spt& f = _fragmentations.front();
            if(f->size())
            {
                break;
            }
            cache_fragmentation(f);
            _fragmentations.pop_front();
        }
    }
};


};
};

#endif // KING_LIB_HEADER_BYTES_BUFFER
