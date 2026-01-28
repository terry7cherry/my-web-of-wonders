#include "RngKiss.h"

void RngKiss::setSeed(const uint32 newSeed)
{
    seed = kiss_i = newSeed;
    kiss_j = kiss_i * 1103515245L + 12345;
    kiss_k = kiss_j * 1103515245L + 12345;
    if (kiss_k == 0)
        kiss_k = 12345;
    kiss_j = ((kiss_i + kiss_k) % 2580) + 1;
}

uint32 RngKiss::randInt(const uint32 range, [[maybe_unused]] const std::string& sz)
{
    return static_cast<uint32>((static_cast<uint64>(algorithm()) * range) >> 32);
}

uint32 RngKiss::randInt(const uint32 range)
{
    return randInt(range, "");
}

uint32 RngKiss::algorithm()
{
    kiss_j = kiss_j ^ (kiss_j << 17);
    kiss_j = kiss_j ^ (kiss_j >> 15);
    kiss_k = (kiss_k ^ (kiss_k << 18)) & 0x7FFFFFFFL;
    kiss_k = kiss_k ^ (kiss_k >> 13);
    kiss_i = 69069L * kiss_i + 23606797L;
    return kiss_i + kiss_j + kiss_k;
}
