#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/TypeTraits.hpp"

namespace krys
{
  template <typename T>
  constexpr bool EnableCompactPointers = false;

#define KRYS_ENABLE_COMPACT_POINTERS_FOR_INCOMPLETE_TYPE(T)                                                  \
  template <>                                                                                                \
  constexpr bool EnableCompactPointers<T *> = true

#define KRYS_ENABLE_COMPACT_POINTERS_IMPL constexpr static bool EnableCompactPointers = true

#define KRYS_ENABLE_COMPACT_POINTERS                                                                         \
public:                                                                                                      \
  KRYS_ENABLE_COMPACT_POINTERS_IMPL;                                                                         \
                                                                                                             \
private:                                                                                                     \
  using _forceSemicolonAllowCompactPointers = int

#define KRYS_ALLOW_STRUCT_COMPACT_POINTERS                                                                   \
public:                                                                                                      \
  KRYS_ALLOW_COMPACT_POINTERS_IMPL;                                                                          \
  using _forceSemicolonAllowCompactPointers = int

  template <typename T>
  constexpr bool AllowsCompactPointers() noexcept
  {
    if constexpr (krys::IsTypeComplete<remove_ptr_t<T>>)
    {
      return remove_ptr_t<T>::EnableCompactPointers;
    }
    else
    {
      // We want to support compact pointers to incomplete types too, so we have this fallback:
      // if a type is incomplete, AllowCompactPointers can be specialized on its pointer type,
      // in which case we'll return its value. This is mostly accomplished using the
      // KRYS_ENABLE_COMPACT_POINTERS_FOR_INCOMPLETE_TYPE macro.
      return EnableCompactPointers<remove_const_t<remove_ptr_t<T>> *>;
    }
  }
}