/**
 * @file
 * @author chu
 * @date 2017/5/28
 */
#include <Moe.Core/Encoding.hpp>

using namespace std;
using namespace moe;
using namespace Encoding;

//////////////////////////////////////////////////////////////////////////////// UTF8

const char* const Utf8::kName = "Utf8";

EncodingResult Utf8::Decoder::operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out,
    uint32_t& count)noexcept
{
    // http://bjoern.hoehrmann.de/utf-8/decoder/dfa/
    static const uint8_t kUtf8Dfa[] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
        8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
        0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
        0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
        0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
        1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
        1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
        1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
    };

    auto b = static_cast<uint8_t>(ch);
    uint32_t type = kUtf8Dfa[b];
    m_iTmp = (m_iState != 0) ? (b & 0x3Fu) | (m_iTmp << 6u) : (0xFFu >> type) & b;
    m_iState = kUtf8Dfa[256 + m_iState * 16 + type];

    count = 0;
    switch (m_iState)
    {
        case 0:
            out[0] = m_iTmp;
            m_iTmp = 0;
            count = 1;
            return EncodingResult::Accept;
        case 1:
            m_iState = 0;
            m_iTmp = 0;
            return EncodingResult::Reject;
        default:
            return EncodingResult::Incomplete;
    }
}

EncodingResult Utf8::Encoder::operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out,
    uint32_t& count)noexcept
{
    auto cp = static_cast<uint32_t>(ch);

    count = 0;
    if (cp <= 0x7Fu)
    {
        out[0] = static_cast<char>(cp & 0xFFu);
        count = 1;
    }
    else if (cp <= 0x7FFu)
    {
        out[0] = static_cast<char>(0xC0u | ((cp >> 6u) & 0xFFu));
        out[1] = static_cast<char>(0x80u | (cp & 0x3Fu));
        count = 2;
    }
    else if (cp <= 0xFFFFu)
    {
        out[0] = static_cast<char>(0xE0u | ((cp >> 12u) & 0xFFu));
        out[1] = static_cast<char>(0x80u | ((cp >> 6u) & 0x3Fu));
        out[2] = static_cast<char>(0x80u | (cp & 0x3Fu));
        count = 3;
    }
    else if (cp <= 0x10FFFFu)
    {
        out[0] = static_cast<char>(0xF0u | ((cp >> 18u) & 0xFFu));
        out[1] = static_cast<char>(0x80u | ((cp >> 12u) & 0x3Fu));
        out[2] = static_cast<char>(0x80u | ((cp >> 6u) & 0x3Fu));
        out[3] = static_cast<char>(0x80u | (cp & 0x3Fu));
        count = 4;
    }
    else
        return EncodingResult::Reject;

    return EncodingResult::Accept;
}

//////////////////////////////////////////////////////////////////////////////// UTF16

const char* const Utf16::kName = "Utf16";

EncodingResult Utf16::Decoder::operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out,
    uint32_t& count)noexcept
{
    auto word = static_cast<uint16_t>(ch);

    count = 0;
    switch (m_iState)
    {
        case 0:
            if (word < 0xD800u || word > 0xDFFFu)
            {
                out[0] = word;
                count = 1;
                return EncodingResult::Accept;
            }
            else if (word <= 0xDBFFu)
            {
                m_iLastWord = word;
                m_iState = 1;
                return EncodingResult::Incomplete;
            }
            else
                return EncodingResult::Reject;
        case 1:
            if (!(word >= 0xDC00u && word <= 0xDFFFu))
            {
                m_iState = 0;
                return EncodingResult::Reject;
            }
            out[0] = (m_iLastWord & 0x3FFu) << 10u;
            out[0] |= word & 0x3FFu;
            out[0] += 0x10000u;
            count = 1;
            return EncodingResult::Accept;
        default:
            assert(false);
            return EncodingResult::Reject;
    }
}

EncodingResult Utf16::Encoder::operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out,
    uint32_t& count)noexcept
{
    auto cp = static_cast<uint32_t>(ch);

    count = 0;
    if (cp <= 0xFFFFu)
    {
        out[0] = static_cast<char16_t>(cp);
        count = 1;
    }
    else if (cp <= 0x10FFFFu)
    {
        cp -= 0x10000u;
        out[0] = static_cast<char16_t>(0xD800u | (cp >> 10u));
        out[1] = static_cast<char16_t>(0xDC00u | (cp & 0x3FFu));
        count = 2;
    }
    else
        return EncodingResult::Reject;

    return EncodingResult::Accept;
}

//////////////////////////////////////////////////////////////////////////////// UTF32

const char* const Utf32::kName = "Utf32_Dummy";

//////////////////////////////////////////////////////////////////////////////// Base64

const char* const Base64::kName = "Base64";

static const char kBase64EncodingTable[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
};
static_assert(sizeof(kBase64EncodingTable) == 64, "Please check data");

static const uint8_t kBase64DecodingTable[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   62, 0xFF, 0xFF, 0xFF,   63,
      52,   53,   54,   55,   56,   57,   58,   59,   60,   61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF,    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
      15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,
      41,   42,   43,   44,   45,   46,   47,   48,   49,   50,   51, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};
static_assert(sizeof(kBase64DecodingTable) == 256, "Please check data");

bool Base64::Decoder::operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>&, uint32_t& count)noexcept
{
    count = 0;
    bool ret = (m_iState == 0 || m_iState == 0xFFFFFFFFu);  // -1表示遇到了终止字符

    m_iState = 0;
    return ret;
}

EncodingResult Base64::Decoder::operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out,
    uint32_t& count)noexcept
{
    count = 0;

    // 允许消耗掉空白符
    if (StringUtils::IsWhitespace(ch))
        return EncodingResult::Accept;

    // 解码流已经结束了
    if (m_iState == 0xFFFFFFFF)
    {
        m_iState = 0;
        return EncodingResult::Reject;
    }

    switch (m_iState)
    {
        case 0:
            m_stBuf[0] = static_cast<uint8_t>(ch);
            m_iState = 1;
            if (kBase64DecodingTable[m_stBuf[0]] == 0xFF)
            {
                m_iState = 0;
                return EncodingResult::Reject;
            }
            break;
        case 1:
            m_stBuf[1] = static_cast<uint8_t>(ch);
            m_iState = 2;
            if (kBase64DecodingTable[m_stBuf[1]] == 0xFF)
            {
                m_iState = 0;
                return EncodingResult::Reject;
            }
            break;
        case 2:
            m_stBuf[2] = static_cast<uint8_t>(ch);
            m_iState = 3;
            if (ch != '=' && kBase64DecodingTable[m_stBuf[2]] == 0xFF)
            {
                m_iState = 0;
                return EncodingResult::Reject;
            }
            break;
        case 3:
            if (ch != '=' && kBase64DecodingTable[static_cast<uint8_t>(ch)] == 0xFF)
            {
                m_iState = 0;
                return EncodingResult::Reject;
            }
            else
            {
                m_iState = 0;
                uint32_t tmp = kBase64DecodingTable[m_stBuf[0]] << 18u;
                tmp += kBase64DecodingTable[m_stBuf[1]] << 12u;
                out[0] = static_cast<uint8_t>((tmp & 0x00FF0000u) >> 16u);
                ++count;
                if (m_stBuf[2] != '=')
                {
                    tmp += kBase64DecodingTable[m_stBuf[2]] << 6u;
                    out[1] = static_cast<uint8_t>((tmp & 0x0000FF00u) >> 8u);
                    ++count;
                    if (ch != '=')
                    {
                        tmp += kBase64DecodingTable[static_cast<uint8_t>(ch)];
                        out[2] = static_cast<uint8_t>(tmp & 0xFFu);
                        ++count;
                    }
                    else
                        m_iState = 0xFFFFFFFF;  // 输入完全处理完毕
                }
                else
                {
                    if (ch != '=')
                    {
                        m_iState = 0;
                        return EncodingResult::Reject;
                    }

                    m_iState = 0xFFFFFFFF;  // 输入完全处理完毕
                }
                return EncodingResult::Accept;
            }
        default:
            assert(false);
            return EncodingResult::Reject;
    }
    return EncodingResult::Incomplete;
}

bool Base64::Encoder::operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count)noexcept
{
    count = 0;
    switch (m_iState)
    {
        case 0:
            break;
        case 1:
            out[0] = kBase64EncodingTable[(m_stBuf[0] & 0xFC) >> 2];
            out[1] = kBase64EncodingTable[(m_stBuf[0] & 0x03) << 4];
            out[2] = '=';
            out[3] = '=';
            count = 4;
            break;
        case 2:
            out[0] = kBase64EncodingTable[(m_stBuf[0] & 0xFC) >> 2];
            out[1] = kBase64EncodingTable[((m_stBuf[0] & 0x03) << 4) | ((m_stBuf[1] & 0xF0) >> 4)];
            out[2] = kBase64EncodingTable[(m_stBuf[1] & 0x0F) << 2];
            out[3] = '=';
            count = 4;
            break;
        default:
            assert(false);
            return false;
    }
    return true;
}

EncodingResult Base64::Encoder::operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out,
    uint32_t& count)noexcept
{
    count = 0;
    switch (m_iState)
    {
        case 0:
            m_stBuf[0] = ch;
            m_iState = 1;
            break;
        case 1:
            m_stBuf[1] = ch;
            m_iState = 2;
            break;
        case 2:
            out[0] = kBase64EncodingTable[(m_stBuf[0] & 0xFC) >> 2];
            out[1] = kBase64EncodingTable[((m_stBuf[0] & 0x03) << 4) + ((m_stBuf[1] & 0xF0) >> 4)];
            out[2] = kBase64EncodingTable[((m_stBuf[1] & 0x0F) << 2) + ((ch & 0xC0) >> 6)];
            out[3] = kBase64EncodingTable[ch & 0x3F];
            count = 4;
            m_iState = 0;
            return EncodingResult::Accept;
        default:
            assert(false);
            return EncodingResult::Reject;
    }
    return EncodingResult::Incomplete;
}
