#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

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
  class NamedField : public Impl::NamedField, NonCopyable<NamedField<T>>
  {
    static_assert(!DerivedFrom<T, Impl::NamedField>, "T must not derive from NamedField");

    // If array   - store as is so we preserve the array type/size.
    // if l-value - take l-value reference.
    // else       - make a copy of the data (not ideal).
    using FieldType =
      conditional_t<IsArray<remove_cvref_t<T>>, remove_cv_t<T>, conditional_t<LValueRef<T>, T, decay_t<T>>>;

  public:
    constexpr NamedField(stringview name, T &&value) noexcept : Name(name), Value(std::forward<T>(value))
    {
    }

    stringview Name;
    FieldType Value;
  };

  template <typename T>
  auto CreateNamedField(stringview name, T &&value) noexcept
  {
    return NamedField<T>(name, std::forward<T>(value));
  }

  template <typename T>
  auto CreateNamedField(const char *name, T &&value) noexcept
  {
    return NamedField<T>(name, std::forward<T>(value));
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
  class ContainerSize : public Impl::ContainerSize, NonCopyable<ContainerSize<T>>
  {
  public:
    // Stores a reference if passed an lvalue reference, otherwise makes a copy of the data.
    using SizeType = conditional_t<LValueRef<T>, T, decay_t<T>>;

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

#pragma endregion

#pragma region KeyValuePair

  namespace Impl
  {
    class KeyValuePair
    {
    };
  }

  template <typename TKey, typename TValue>
  struct KeyValuePair : public Impl::KeyValuePair, NonCopyable<KeyValuePair<TKey, TValue>>
  {
    using KeyType = conditional_t<LValueRef<TKey>, TKey, decay_t<TKey>>;
    using ValueType = conditional_t<LValueRef<TValue>, TValue, decay_t<TValue>>;

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

#pragma endregion

#pragma region Version

  struct Version
  {
    using ValueType = uint32;

    constexpr static const char *FieldName = "version";

    constexpr explicit Version(ValueType version) noexcept : Value(version)
    {
    }

    constexpr auto operator<=>(const Version &) const noexcept = default;

    constexpr auto operator<=>(const ValueType value) const noexcept
    {
      return Value <=> value;
    }

    ValueType Value;
  };

  template <typename T>
  struct VersionTraits
  {
    // Purposely empty - specialisations should define ClassVersion.
  };

  /// @brief Macro to define a class version inside a class definition.
#define KRYS_CLASS_VERSION(version)                                                                          \
  constexpr static Krys::Serialisation::Version ClassVersion = Krys::Serialisation::Version(version);        \
  KRYS_NODISCARD constexpr Version GetVersion() const noexcept                                               \
  {                                                                                                          \
    return ClassVersion;                                                                                     \
  }

  /// @brief Macro to define a trait for a class version outside of a class definition. Must be outside
  /// of any namespace, and the type must be fully qualified.
#define KRYS_CLASS_VERSION_TRAIT(type, version)                                                              \
  namespace Krys::Serialisation                                                                              \
  {                                                                                                          \
    template <>                                                                                              \
    struct VersionTraits<type>                                                                               \
    {                                                                                                        \
      constexpr static Version ClassVersion = Version(version);                                              \
    };                                                                                                       \
  }

  template <typename T>
  concept HasTraitVersion = requires { VersionTraits<remove_cvref_t<T>>::ClassVersion; };

#pragma endregion
}