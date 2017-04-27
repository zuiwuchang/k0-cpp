//異常定義
#ifndef KING_LIB_HEADER_EXCEPTION
#define KING_LIB_HEADER_EXCEPTION

#include <string>

namespace k0
{
    /**
    *   \brief 異常基類
    */
    class exception
    {
    public:
        explicit exception(){}
        virtual ~exception(){}
        /**
        *   \brief 返回 異常 描述字符串
        */
        virtual const char* what() const
        {
            return "k0::exception";
        }
    };
};

#endif // KING_LIB_HEADER_EXCEPTION
