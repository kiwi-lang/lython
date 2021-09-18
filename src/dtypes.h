#ifndef LYTHON_TYPES_HEADER
#define LYTHON_TYPES_HEADER

#include <functional>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <sstream>

#include "utilities/allocator.h"

// ---------------
namespace lython {

using uint64 = std::uint64_t;
using int64  = std::int64_t;

using uint32 = std::uint32_t;
using int32  = std::int32_t;

using uint16 = std::uint16_t;
using int16  = std::int16_t;

using uint8 = std::uint8_t;
using int8  = std::int8_t;

using float32 = float;
using float64 = double;

using uchar = unsigned char;

template <typename V>
using AllocatorCPU = Allocator<V, device::CPU>;

template <typename V>
using Array = std::vector<V, AllocatorCPU<V>>;

template <typename V>
using List = std::list<V, AllocatorCPU<V>>;

using String = std::basic_string<char, std::char_traits<char>, AllocatorCPU<char>>;

using StringStream = std::basic_stringstream<char, std::char_traits<char>, AllocatorCPU<char>>;

using StringView = std::string_view;
} // namespace lython

// ------------
namespace std {

//*
template <typename Char, typename Allocator>
struct hash<std::basic_string<Char, std::char_traits<Char>, Allocator>> {
    using Key = std::basic_string<Char, std::char_traits<Char>, Allocator>;

    std::size_t operator()(Key const &k) const noexcept {
        auto a = std::hash<std::string>();
        // FIXME: find a way to reuse the string hashing without transforming the string
        return a(std::string(k.c_str()));
    }
};
//*/

} // namespace std

// ---------------
namespace lython {

template <class _Ty, class _Dx = std::default_delete<_Ty>>
using Unique = std::unique_ptr<_Ty, _Dx>;

template <class _Ty>
using Shared = std::shared_ptr<_Ty>;

template <typename... Args>
using Tuple = std::tuple<Args...>;

template <typename A, typename B>
using Pair = std::pair<A, B>;

template <typename K, typename V, typename H = std::hash<K>>
using Dict = std::unordered_map<K, V, H, std::equal_to<K>, AllocatorCPU<std::pair<K const, V>>>;

template <typename V>
using Set = std::unordered_set<V, std::hash<V>, std::equal_to<V>, AllocatorCPU<V>>;

} // namespace lython

#endif
