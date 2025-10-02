#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/String.hpp"

namespace Krys::Impl
{
  class NamedField
  {
  };
}

namespace Krys::Serialisation
{
  template <typename T>
  class NamedField : public Impl::NamedField
  {
    NO_COPY(NamedField)

    static_assert(!DerivedFrom<T, Impl::NamedField>, "T must not derive from NamedField");

    // If array   - store as is so we preserve the array type/size.
    // if l-value - take l-value reference.
    // else       - make a copy of the data (not ideal).
    using Type = std::conditional_t<IsArray<RemoveRef<T>>, RemoveCv<T>,
                                    std::conditional_t<IsLValueRef<T>::value, T, std::decay_t<T>>>;

  public:
    constexpr NamedField(char const *name, T &&value) noexcept : Name(name), Value(std::forward<T>(value))
    {
    }

    char const *Name;
    Type Value;
  };

  template <typename T>
  class SizeTag
  {
    NO_COPY(SizeTag)

    // Store a reference if passed an lvalue reference, otherwise make a copy of the data
    using SizeType = std::conditional_t<IsLValueRef<T>::type, T, Decay<T>>;

  public:
    SizeTag(T &&value) noexcept : Size(std::forward<T>(value))
    {
    }

    SizeType Size;
  };

  template <typename T>
  auto CreateNamedField(const string &name, T &&value) noexcept
  {
    return NamedField<T>(name, std::forward<T>(value));
  }

  template <typename T>
  auto CreateNamedField(const char *name, T &&value) noexcept
  {
    return NamedField<T>(name, std::forward<T>(value));
  }

#define KRYS_NAMED_FIELD(var) ::Krys::Serialisation::CreateNamedField(#var, var)

  template <typename Archive, typename T>
  void Transfer(Archive &archive, NamedField<T> &field) noexcept
  {
    archive(field.Value);
  }
}