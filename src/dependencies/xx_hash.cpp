#include "xx_hash.h"

#define XXH_VECTOR XXH_AVX2

// only from Zen 4
// #define XXH_VECTOR XXH_AVX512

#include "xxHash/xxh3.h"
#include "xxHash/xxhash.c"
#include "xxHash/xxhash.h"

namespace lython {

std::size_t xx_hash_3(void const* buffer, std::size_t size) noexcept {
    return XXH3_64bits(buffer, size);
}

}  // namespace lython