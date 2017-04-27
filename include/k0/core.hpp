//基本數據 型別定義
#ifndef KING_LIB_HEADER_CORE
#define KING_LIB_HEADER_CORE

#include <cstdint>
#include <memory>


namespace k0
{
	//為兼容舊風格 的保留代碼
	
	/**
	*	\brief 有符號 8 bit 整型
	*/
	typedef std::int8_t int8_t;
	/**
	*	\brief 有符號 16 bit 整型
	*/
	typedef std::int16_t int16_t;
	/**
	*	\brief 有符號 32 bit 整型
	*/
	typedef std::int32_t int32_t;
	/**
	*	\brief 有符號 32 bit 整型
	*/
	typedef std::int64_t int64_t;

	/**
	*	\brief 無有符號 8 bit 整型
	*/
	typedef std::uint8_t uint8_t;
	/**
	*	\brief 無有符號 16 bit 整型
	*/
	typedef std::uint16_t uint16_t;
	/**
	*	\brief 無有符號 32 bit 整型
	*/
	typedef std::uint32_t uint32_t;
	/**
	*	\brief 無有符號 64 bit 整型
	*/
	typedef std::uint64_t uint64_t;

	/**
	*	\brief 1個字節
	*/
	typedef std::uint8_t byte_t;

};

#endif // KING_LIB_HEADER_CORE
