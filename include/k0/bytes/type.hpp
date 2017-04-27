//需要用到的 型別 定義
#ifndef KING_LIB_HEADER_BYTES_TYPE
#define KING_LIB_HEADER_BYTES_TYPE

#include "../core.hpp"
#include <boost/smart_ptr.hpp>
namespace k0
{

/**
*	\brief 字節相關
*/
namespace bytes
{
/**
*	\brief 字節數組
*
*	k0::byte_t 的字節數組\n
*	保存類數組 長度
*/
class bytes_t
{
protected:
    typedef k0::byte_t byte_t;
public:
    /**
	*	\brief 構造一個 字節數組
	*
	*	\param size 數組大小
	*/
    explicit bytes_t(const std::size_t size)//no throw
    {
		try
		{
			_bytes = new byte_t[size];
			_size = size;
		}
		catch(const std::bad_alloc&)
		{
			_size = 0;
		}
    }
    /**
	*	\brief move 語義
	*/
    bytes_t(bytes_t&& m)
    {
        _bytes = m._bytes;
        _size = m._size;

        m._bytes = NULL;
        m._size = 0;
    }
    /**
	*	\brief move 語義
	*/
    bytes_t& operator=(bytes_t&& m)
    {
        if(_bytes)
        {
            delete[] _bytes;
        }

        _bytes = m._bytes;
        _size = m._size;

        m._bytes = NULL;
        m._size = 0;

        return *this;
    }
private:
    bytes_t& operator=(const bytes_t&);
    bytes_t(const bytes_t&);
public:
	/**
	*	\brief 析構函數 釋放數組
	*/
	virtual ~bytes_t()
    {
        if(_bytes)
        {
            delete[] _bytes;
        }
    }
    /**
	*	\brief 返回數組 是否不為 空
	*
	*	如果構造函數 失敗 則數組爲空
	*/
    inline operator bool()const
    {
        return _size != 0;
    }
    /**
	*	\brief 返回數組 是否為 空
	*
	*	如果構造函數 失敗 則數組爲空
	*/
    inline bool empty()const
    {
        return _size == 0;
    }

    /**
	*	\brief 返回數組內部保存的 c 指針
	*
	*/
    inline byte_t* get()const
    {
        return _bytes;
    }
    /**
	*	\brief 返回數組內部保存的 c 指針
	*
	*/
    inline operator byte_t*()const
    {
        return _bytes;
    }

	/**
	*	\brief 返回數組 大小
	*/
    inline std::size_t size()const
    {
        return _size;
    }

    /**
	*	\brief 手動釋放數組
	*
	*	如果數組不爲空 釋放內部指針 之後 數組變爲空
	*/
    void reset()
    {
        if(_bytes)
        {
            delete[] _bytes;
            _bytes = NULL;
            _size = 0;
        }
    }
private:
    byte_t* _bytes;
    std::size_t _size;
};



/**
*	\brief 數據 分片緩衝區
*
*	bytes_t 的分片數據\n
*	爲建立 bytes_t 緩存 提供了 方便操作的 函數
*/
class fragmentation_t
{
	
protected:
    typedef k0::byte_t byte_t;

    /**
	*	\brief 內部保存的 數組指針
	*
	*	bytes_t 的智能指針
	*/
    boost::shared_ptr<bytes_t> _array;

	/**
	*	\brief 內部數組 容量
	*/
    std::size_t _capacity;
	/**
	*	\brief 內部數組 偏移
	*/
    std::size_t _offset;
	/**
	*	\brief 內部數組 大小
	*/
    std::size_t _size;

public:
    /**
	*	\brief 構造一個指定容量的 分片數據
	*/
    explicit fragmentation_t(const std::size_t size):
		_capacity(0),_offset(0),_size(0)
    {
		try
		{
		   _array = boost::make_shared<bytes_t>(size);
		   _capacity = size;
		}
		catch(const std::bad_alloc&)
		{
		}
    }
private:
	fragmentation_t(const fragmentation_t& copy);
    fragmentation_t& operator=(const fragmentation_t& copy);
public:
	/**
	*	\brief 返回 分片 是否不為空
	*/
    inline operator bool()const
    {
        return _capacity != 0;
    }
    /**
	*	\brief 返回 分片 是否為空
	*/
    inline bool empty()const
    {
        return _capacity == 0;
    }

    /**
	*	\brief 重置 分片
	*
	*	此後分片 大小 偏移 爲0  容量不變
	*/
    inline void init()
    {
        _offset = _size = 0;
    }
    /**
	*	\brief 釋放 分片
	*
	*	此後分片 大小 偏移 容量 爲0\n
	*	並且釋放數組
	*/
    inline void reset()
    {
        _array.reset();
        _capacity = _offset = _size = 0;
    }

    /**
	*	\brief 返回 容量
	*/
    inline std::size_t capacity() const
    {
        return _capacity;
    }
	/**
	*	\brief 返回 有效數據 實際大小
	*/
    inline std::size_t size() const
    {
        return _size;
    }
	/**
	*	\brief 返回 空閒 容量
	*/
    inline std::size_t get_free()
    {
        return _capacity - _offset - _size;
    }
	
    /**
	*	\brief 在分片尾寫入數據
	*
	*	\param bytes 待寫入數據指針
	*	\param n 待寫入數據大小
	*
	*	\return	實際寫入大小
	*/
    std::size_t write(const byte_t* bytes,const std::size_t n)
    {
        std::size_t free = get_free();
        std::size_t need = n;
        if(need > free)
        {
            need = free;
        }
        memcpy(_array->get() + _offset + _size,bytes,need * sizeof(byte_t));
        _size += need;

        return need;
    }
	
	/**
	*	\brief 讀取 數據
	*
	*	從分片頭讀取數據\n
	*	被讀取的 數據 將被 移除 緩衝區
	*
	*	\param bytes 待讀取數據指針
	*	\param n 待讀取數據大小
	*
	*	\return	實際讀取大小
	*/
    std::size_t read(byte_t* bytes,const std::size_t n)
    {
        std::size_t need = n;
        if(need > _size)
        {
            need = _size;
        }

        memcpy(bytes,_array->get() + _offset,need * sizeof(byte_t));
        _size -= need;
        _offset += need;

        return need;
    }

    /**
	*	\brief 拷貝 數據
	*
	*	同 fragmentation_t::read \n
	*	被讀取的 數據 不會 從緩衝區 移除
	*
	*	\param bytes 待讀取數據指針
	*	\param n 待讀取數據大小
	*
	*	\return	實際讀取大小
	*/
    std::size_t copy_to(byte_t* bytes,const std::size_t n)const
    {
        std::size_t need = n;
        if (n > _size)
        {
            need = _size;
        }
        memcpy(bytes,_array->get() + _offset,need * sizeof(byte_t));

        return need;
    }
    /**
	*	\brief 拷貝 數據
	*
	*	同 fragmentation_t::copy \n
	*	但 跳過 緩衝區 前 skip 個 字節
	*
	*	\param skip 要跳過的字節數
	*	\param bytes 待讀取數據指針
	*	\param n 待讀取數據大小
	*
	*	\return	實際讀取大小
	*
	*/
    std::size_t copy_to(const std::size_t skip,byte_t* bytes,const std::size_t n)const
    {
        if(skip >= _size)
        {
            return 0;
        }
        std::size_t offset = _offset + skip;
        std::size_t size = _size - skip;

        std::size_t need = n;
        if (need > size)
        {
            need = size;
        }
        memcpy(bytes,_array->get() + offset,need * sizeof(byte_t));

        return need;
    }
};

};
};

#endif // KING_LIB_HEADER_BYTES_TYPE
