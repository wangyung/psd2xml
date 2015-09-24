#pragma once
#ifndef PSDUTILS_INT_H
#define PSDUTILS_INT_H

#define CHECK_FOUR_CHAR(s, a, b, c, d) \
    ((s)[0] == a && (s)[1] == b && (s)[2] == c && (s)[3] == d)

#define PAD2(x) (((x)+1) & -2) // same or next even
#define PAD4(x) (((x)+3) & -4) // same or next multiple of 4

#if defined(__GNUC__)
    #define ALWAYS_INLINE(f) inline f __attribute__((always_inline)); inline f
#elif defined(_MSC_VER)
    #define ALWAYS_INLINE(f) __forceinline f
#else
    #define ALWAYS_INLINE(f) inline f
#endif

namespace PsdUtils
{
    inline uint16_t fetch16(const uint8_t *p)
    {
        return ((uint16_t) p[0] << 8)
            | (uint16_t) p[1];
    }

    inline uint32_t fetch32(const uint8_t *p)
    {
        return ((uint32_t) p[0] << 24)
            | ((uint32_t) p[1] << 16)
            | ((uint32_t) p[2] << 8)
            | (uint32_t) p[3];
    }

    inline string fetchPascalString(const uint8_t *p)
    {
        return string((const char *) p + 1, (size_t) *p);
    }

    void fetchRect(PsdRect *dst, const PsdRect *src);

#define DEFINE_BSWAP_T(n, t, r) \
    ALWAYS_INLINE(t n(t v)) { return r; } \
    ALWAYS_INLINE(u##t n(u##t v)) { return r; }
#define DEFINE_BSWAP(n) \
    DEFINE_BSWAP_T(n, int32_t, n##l(v)) \
    DEFINE_BSWAP_T(n, int16_t, n##s(v)) \
    DEFINE_BSWAP_T(n, int8_t, v)

    DEFINE_BSWAP(ntoh)
    DEFINE_BSWAP(hton)
}

#ifdef _WIN32
    class LastErrorMsgA
    {
    public:
        LastErrorMsgA() : lpErrMsgBuffer(NULL)
        { FormatMessage(GetLastError()); }
        LastErrorMsgA(int iError) : lpErrMsgBuffer(NULL)
        { FormatMessage(iError); }
        ~LastErrorMsgA() { LocalFree(lpErrMsgBuffer); }
        operator LPCSTR() {return (LPCSTR) lpErrMsgBuffer;}
    private:
        void FormatMessage(int iError)
        {
            DWORD dwLen = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, iError, 0, (LPSTR) &lpErrMsgBuffer, 0, NULL);
            if (dwLen > 0) {
                if (lpErrMsgBuffer[dwLen - 1] == '\n')
                    lpErrMsgBuffer[dwLen - 1] = '\0';
            }
        }
        LPSTR lpErrMsgBuffer;
    };

    class LastErrorMsgW
    {
    public:
        LastErrorMsgW() : lpErrMsgBuffer(NULL)
        { FormatMessage(GetLastError()); }
        LastErrorMsgW(int iError) : lpErrMsgBuffer(NULL)
        { FormatMessage(iError); }
        ~LastErrorMsgW() { LocalFree(lpErrMsgBuffer); }
        operator LPCWSTR() {return (LPCWSTR) lpErrMsgBuffer;}
    private:
        void FormatMessage(int iError)
        {
            DWORD dwLen = ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, iError, 0, (LPWSTR) &lpErrMsgBuffer, 0, NULL);
            if (dwLen > 0) {
                if (lpErrMsgBuffer[dwLen - 1] == L'\n')
                    lpErrMsgBuffer[dwLen - 1] = L'\0';
            }
        }
        LPWSTR lpErrMsgBuffer;
    };
    #ifdef _UNICODE
        #define lastErrorMsg(...) ((LPCWSTR) LastErrorMsgW(__VA_ARGS__))
    #else
        #define lastErrorMsg(...) ((LPCSTR) LastErrorMsgA(__VA_ARGS__))
    #endif
#else
    // ! _WIN32
    inline const char *lastErrorMsg() { return strerror(errno); }
    inline const char *lastErrorMsg(int iError) { return strerror(iError); }
#endif

#endif // PSDUTILS_INT_H
