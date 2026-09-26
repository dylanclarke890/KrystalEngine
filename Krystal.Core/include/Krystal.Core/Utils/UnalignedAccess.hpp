#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <bit>

namespace krys
{
  template <TriviallyCopyable Type>
  KRYS_NODISCARD constexpr Type UnalignedLoad(const byte *pointer) noexcept
  {
    Array<byte, sizeof(Type)> bytes;
    for (size_t i = 0uz; i < sizeof(Type); ++i)
    {
      bytes[i] = pointer[i];
    }

    return std::bit_cast<Type>(bytes);
  }

  template <TriviallyCopyable Type>
  constexpr void UnalignedStore(byte *pointer, Type value) noexcept
  {
    const auto bytes = std::bit_cast<Array<byte, sizeof(Type)>>(value);
    for (size_t i = 0uz; i < sizeof(Type); ++i)
    {
      pointer[i] = bytes[i];
    }
  }
}