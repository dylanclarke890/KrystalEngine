#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/String.hpp"

namespace Krys::Serialisation
{
#pragma region NamedField

  namespace Impl
  {
    class NamedField
    {
    };
  }

  /// @brief A wrapper around a field to be serialised, to provide a more readable name for the appropriate
  /// archives to display. Use CreateNamedField() or the KRYS_NAMED_FIELD() macro for convenience.
  template <typename T>
  class NamedField : public Impl::NamedField
  {
    NO_COPY(NamedField)

    static_assert(!DerivedFrom<T, Impl::NamedField>, "T must not derive from NamedField");

    // If array   - store as is so we preserve the array type/size.
    // if l-value - take l-value reference.
    // else       - make a copy of the data (not ideal).
    using FieldType =
      Conditional<IsArray<RemoveCvRef<T>>, RemoveCv<T>, Conditional<IsLValueRef<T>, T, Decay<T>>>;

  public:
    constexpr NamedField(char const *name, T &&value) noexcept : Name(name), Value(std::forward<T>(value))
    {
    }

    char const *Name;
    FieldType Value;
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

  template <typename Archive, typename T>
  void Transfer(Archive &archive, NamedField<T> &field) noexcept
  {
    // Every NamedField ultimately just serialises its value as normal for a given archive, its name is
    // handled by the Before/AfterTransfer hooks, specialised by each Archive implementation.
    archive(field.Value);
  }

#define KRYS_NAMED_FIELD(var) ::Krys::Serialisation::CreateNamedField(#var, var)

#pragma endregion

#pragma region ContainerSize

  namespace Impl
  {
    class ContainerSize
    {
    };
  }

  /// @brief Allows archives to prepare to serialise a container of known size (not possible with an arbitrary
  /// int). Archives can either serialise the size to make deserialisation easier, or use it to set up
  /// internal state as needed (e.g. start a JSON array) by utilising the
  /// BeforeTransfer/Transfer/AfterTransfer hooks.
  template <typename T>
  class ContainerSize : public Impl::ContainerSize
  {
    NO_COPY(ContainerSize)

  public:
    // Stores a reference if passed an lvalue reference, otherwise makes a copy of the data.
    using SizeType = Conditional<IsLValueRef<T>, T, Decay<T>>;

    ContainerSize(T &&value) noexcept : Size(std::forward<T>(value))
    {
    }

    SizeType Size;
  };

  template <typename T>
  auto CreateContainerSize(T &&value) noexcept
  {
    return ContainerSize<T>(std::forward<T>(value));
  }

#define KRYS_CONTAINER_SIZE(var) ::Krys::Serialisation::CreateContainerSize(var)

  template <typename Archive, typename T>
  void Transfer(Archive &archive, ContainerSize<T> &container) noexcept
  {
    // By default, just serialise the size as normal.
    archive(container.Size);
  }

#pragma endregion

#pragma region KeyValuePair

  namespace Impl
  {
    class KeyValuePair
    {
    };
  }

  template <typename TKey, typename TValue>
  struct KeyValuePair : public Impl::KeyValuePair
  {
    NO_COPY(KeyValuePair)

    using KeyType = Conditional<IsLValueRef<TKey>, TKey, Decay<TKey>>;
    using ValueType = Conditional<IsLValueRef<TValue>, TValue, Decay<TValue>>;

    KeyValuePair(TKey &&key, TValue &&value) noexcept
        : Key(std::forward<TKey>(key)), Value(std::forward<TValue>(value))
    {
    }

    KeyType Key;
    ValueType Value;
  };

  template <typename TKey, typename TValue>
  KeyValuePair<TKey, TValue> CreateKeyValuePair(TKey &&key, TValue &&value) noexcept
  {
    return {std::forward<TKey>(key), std::forward<TValue>(value)};
  }

#define KRYS_KEY_VALUE_PAIR(key, value) ::Krys::Serialisation::CreateKeyValuePair(key, value)

  template <class Archive, typename TKey, typename TValue>
  void Transfer(Archive &archive, KeyValuePair<TKey, TValue> &pair) noexcept
  {
    archive(CreateNamedField("key", pair.Key), CreateNamedField("value", pair.Value));
  }

#pragma endregion
}