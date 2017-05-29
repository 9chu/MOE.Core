/**
 * @file
 * @date 2017/5/28
 */
#include <Moe.Core/Exception.hpp>

using namespace std;
using namespace moe;

const std::string& Exception::ToString()const
{
    if (m_strFullDescCache.empty())
    {
        StringUtils::FormatInPlace(m_strFullDescCache, "[{0}:{1}:{2}] {3}", GetSourceFile(), GetFunctionName(),
            GetLineNumber(), GetDescription());
    }

    return m_strFullDescCache;
}

const char* Exception::what()const noexcept
{
    return ToString().c_str();
}
