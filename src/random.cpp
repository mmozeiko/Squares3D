#if WIN32
    #define VC_EXTRALEAN
    #include <windows.h>
    #include <wincrypt.h>
#else
    #include <cstdio>
#endif

#include <ctime>
#include <climits>

#include "random.h"

static const unsigned int N = 624;
static const unsigned int M = 397;
    
static unsigned int  state[N];
static unsigned int* pNext;
static unsigned int  left;

inline unsigned int twist(unsigned int m, unsigned int s0, unsigned int s1)
{
    return m ^ ( ((s0&0x80000000UL) | (s1&0x7fffffffUL)) >> 1 )
             ^ ( -static_cast<int>(s1&0x00000001UL) & 0x9908b0dfUL);
}

inline void seed(unsigned int seed)
{
    unsigned int* s = state;
    unsigned int* r = state;
    *s++ = seed & 0xffffffffUL;
    for (int i=1 ; i<N; ++i)
    {
        *s++ = (1812433253UL * (*r ^ (*r >> 30)) + i) & 0xffffffffUL;
        r++;
    }
}

inline void seed(const unsigned int* bigSeed, unsigned int seedLength)
{
    seed(19650218UL);
    unsigned int i = 1;
    unsigned int j = 0;
    unsigned int k = (N > seedLength ? N : seedLength);

    for ( ; k; --k)
    {
        state[i] = state[i] ^ ((state[i-1] ^ (state[i-1] >> 30)) * 1664525UL);
        state[i] += (bigSeed[j] & 0xffffffffUL ) + j;
        state[i] &= 0xffffffffUL;
        ++i;
        ++j;
        if (i >= N)
        {
            state[0] = state[N-1];
            i = 1;
        }
        if (j >= seedLength) j = 0;
    }

    for (k=N-1; k; --k)
    {
        state[i] = state[i] ^ ((state[i-1] ^ (state[i-1] >> 30)) * 1566083941UL);
        state[i] -= i;
        state[i] &= 0xffffffffUL;
        ++i;
        if (i >= N)
        {
            state[0] = state[N-1];
            i = 1;
        }
    }
    state[0] = 0x80000000UL;
}

void reload()
{
    unsigned int* p = state;

#pragma warning ( push )
#pragma warning ( disable : 4307 )

    for (int i=N-M; i--; ++p) *p = twist(p[M], p[0], p[1]);
    for (int i=M; --i; ++p)   *p = twist(p[M-N], p[0], p[1]);

    *p = twist(p[M-N], p[0], state[0]);

#pragma warning ( pop )

    left = N;
    pNext = state;
}

inline unsigned int hash(const time_t& t, const clock_t& c)
{
    unsigned int h1=0, h2=0;
    const unsigned char* p;

    p = reinterpret_cast<const unsigned char*>(&t);
    for (size_t i=0; i<sizeof(t); ++i) h1 = h1 * (UCHAR_MAX + 2U) + p[i];

    p = reinterpret_cast<const unsigned char*>(&c);
    for (size_t i=0; i<sizeof(c); ++i) h2 = h2 * (UCHAR_MAX + 2U) + p[i];

    return h1 ^ h2;
}

void Random::init()
{
    clog << "Initializing random seed." << endl;

    left = 0;
    bool success = false;
    unsigned int bigSeed[N];

#if WIN32
    HCRYPTPROV hProv;
    if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0)==TRUE ||
        CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET)==TRUE)
    {
        success = (CryptGenRandom(hProv, 
                                  N*sizeof(unsigned int), 
                                  reinterpret_cast<BYTE*>(bigSeed))==TRUE);
        CryptReleaseContext(hProv, 0);
    }
#elif LINUX
    FILE* urandom = std::fopen("/dev/urandom", "rb");
    if (urandom)
    {
        success = std::fread(bigSeed, sizeof(unsigned int), N, urandom)==N;
        std::fclose(urandom);
    }
#endif
    if (success)
    {
        seed(bigSeed, N);
    }
    else
    {
        seed(hash(time(NULL), std::clock()));
    }
}

unsigned int Random::getInt()
{
    if (left == 0) reload();
    --left;

    unsigned int s1 = *pNext++;
    s1 ^= (s1 >> 11);
    s1 ^= (s1 <<  7) & 0x9d2c5680UL;
    s1 ^= (s1 << 15) & 0xefc60000UL;
    return s1 ^ (s1 >> 18);
}

unsigned int Random::getIntN(unsigned int n)
{
    unsigned int used = n;
    used |= used >> 1;
    used |= used >> 2;
    used |= used >> 4;
    used |= used >> 8;
    used |= used >> 16;

    unsigned int i;
    do
    {
        i = getInt() & used;
    } while (i >= n);
    return i;
}

float Random::getFloat()
{
    return getInt() * (1.0f / 4294967295.0f);
}

float Random::getFloatN(float n)
{
    return getFloat() * n;
}
