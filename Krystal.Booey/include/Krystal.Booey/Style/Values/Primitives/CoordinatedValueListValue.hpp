#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Attributes.hpp"
#include <tuple>

// Utilities and macros for implementing coordinating list property groups.
// https://drafts.csswg.org/css-values-4/#coordinating-list-property
//
// The property macros take `(ownerType, property, type, lowercaseName, uppercaseName)`. The owner stores each
// property as `_lowercaseName` with its state in `_lowercaseNameState`, and declares the getter
// `GetUppercaseName()`; the macros declare the rest.

namespace krys::boo::style
{
  template <typename T>
  concept CoordinatedValueListValue = requires(T value) {
    { T::ComputedValueUsesUsedValues } -> SameType<const bool &>;
    { T::BaseProperty.value } -> SameType<const css::PropertyId &>;
    { T::Clone(std::declval<const T &>()) } -> SameType<T>;
    { value.IsInitial() } -> SameType<bool>;
  };

  enum class CoordinatedValueListPropertyState : uint8
  {
    Unset,
    Set,
    Filled,
  };

  template <css::PropertyId>
  struct CoordinatedValueListPropertyAccessor;

  template <css::PropertyId>
  struct CoordinatedValueListPropertyConstAccessor;
}

#define KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY(ownerType, property, type, lowercaseName,               \
                                                     uppercaseName)                                          \
  css::PropertyNameConstant<css::PropertyId::property> {},

#define KRYS_INTERNAL_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_COMMON(ownerType, property, type,     \
                                                                              lowercaseName, uppercaseName)  \
  KRYS_NODISCARD static type Initial() noexcept                                                              \
  {                                                                                                          \
    return ownerType::Initial##uppercaseName();                                                              \
  }                                                                                                          \
  KRYS_NODISCARD bool IsUnset() const noexcept                                                               \
  {                                                                                                          \
    return Value.Is##uppercaseName##Unset();                                                                 \
  }                                                                                                          \
  KRYS_NODISCARD bool IsSet() const noexcept                                                                 \
  {                                                                                                          \
    return Value.Is##uppercaseName##Set();                                                                   \
  }                                                                                                          \
  KRYS_NODISCARD bool IsFilled() const noexcept                                                              \
  {                                                                                                          \
    return Value.Is##uppercaseName##Filled();                                                                \
  }                                                                                                          \
  KRYS_NODISCARD CoordinatedValueListPropertyState State() const noexcept                                    \
  {                                                                                                          \
    return Value.uppercaseName##State();                                                                     \
  }

#define KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_REFERENCE(ownerType, property, type,           \
                                                                        lowercaseName, uppercaseName)        \
  template <>                                                                                                \
  struct CoordinatedValueListPropertyAccessor<css::PropertyId::property>                                     \
  {                                                                                                          \
    using Type = type;                                                                                       \
                                                                                                             \
    ownerType &Value;                                                                                        \
                                                                                                             \
    KRYS_NODISCARD const type &Get() const noexcept                                                          \
    {                                                                                                        \
      return Value.Get##uppercaseName();                                                                     \
    }                                                                                                        \
    void Set(type &&lowercaseName) noexcept                                                                  \
    {                                                                                                        \
      Value.Set##uppercaseName(krys::move(lowercaseName));                                                   \
    }                                                                                                        \
    void Fill(type &&lowercaseName) noexcept                                                                 \
    {                                                                                                        \
      Value.Fill##uppercaseName(krys::move(lowercaseName));                                                  \
    }                                                                                                        \
    void Clear() noexcept                                                                                    \
    {                                                                                                        \
      Value.Clear##uppercaseName();                                                                          \
    }                                                                                                        \
                                                                                                             \
    KRYS_INTERNAL_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_COMMON(ownerType, property, type,         \
                                                                          lowercaseName, uppercaseName)      \
                                                                                                             \
    KRYS_NODISCARD bool operator==(                                                                          \
      const CoordinatedValueListPropertyAccessor<css::PropertyId::property> &other) const noexcept           \
    {                                                                                                        \
      return Get() == other.Get() && State() == other.State();                                               \
    }                                                                                                        \
  };                                                                                                         \
                                                                                                             \
  template <>                                                                                                \
  struct CoordinatedValueListPropertyConstAccessor<css::PropertyId::property>                                \
  {                                                                                                          \
    using Type = type;                                                                                       \
                                                                                                             \
    const ownerType &Value;                                                                                  \
                                                                                                             \
    KRYS_NODISCARD const type &Get() const noexcept                                                          \
    {                                                                                                        \
      return Value.Get##uppercaseName();                                                                     \
    }                                                                                                        \
                                                                                                             \
    KRYS_INTERNAL_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_COMMON(ownerType, property, type,         \
                                                                          lowercaseName, uppercaseName)      \
                                                                                                             \
    KRYS_NODISCARD bool operator==(                                                                          \
      const CoordinatedValueListPropertyConstAccessor<css::PropertyId::property> &other) const noexcept      \
    {                                                                                                        \
      return Get() == other.Get() && State() == other.State();                                               \
    }                                                                                                        \
  };

#define KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_VALUE(ownerType, property, type,               \
                                                                    lowercaseName, uppercaseName)            \
  template <>                                                                                                \
  struct CoordinatedValueListPropertyAccessor<css::PropertyId::property>                                     \
  {                                                                                                          \
    using Type = type;                                                                                       \
                                                                                                             \
    ownerType &Value;                                                                                        \
                                                                                                             \
    KRYS_NODISCARD type Get() const noexcept                                                                 \
    {                                                                                                        \
      return Value.Get##uppercaseName();                                                                     \
    }                                                                                                        \
    void Set(type lowercaseName) noexcept                                                                    \
    {                                                                                                        \
      Value.Set##uppercaseName(lowercaseName);                                                               \
    }                                                                                                        \
    void Fill(type lowercaseName) noexcept                                                                   \
    {                                                                                                        \
      Value.Fill##uppercaseName(lowercaseName);                                                              \
    }                                                                                                        \
    void Clear() noexcept                                                                                    \
    {                                                                                                        \
      Value.Clear##uppercaseName();                                                                          \
    }                                                                                                        \
                                                                                                             \
    KRYS_INTERNAL_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_COMMON(ownerType, property, type,         \
                                                                          lowercaseName, uppercaseName)      \
                                                                                                             \
    KRYS_NODISCARD bool operator==(                                                                          \
      const CoordinatedValueListPropertyAccessor<css::PropertyId::property> &other) const noexcept           \
    {                                                                                                        \
      return Get() == other.Get() && State() == other.State();                                               \
    }                                                                                                        \
  };                                                                                                         \
                                                                                                             \
  template <>                                                                                                \
  struct CoordinatedValueListPropertyConstAccessor<css::PropertyId::property>                                \
  {                                                                                                          \
    using Type = type;                                                                                       \
                                                                                                             \
    const ownerType &Value;                                                                                  \
                                                                                                             \
    KRYS_NODISCARD type Get() const noexcept                                                                 \
    {                                                                                                        \
      return Value.Get##uppercaseName();                                                                     \
    }                                                                                                        \
                                                                                                             \
    KRYS_INTERNAL_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_COMMON(ownerType, property, type,         \
                                                                          lowercaseName, uppercaseName)      \
                                                                                                             \
    KRYS_NODISCARD bool operator==(                                                                          \
      const CoordinatedValueListPropertyConstAccessor<css::PropertyId::property> &other) const noexcept      \
    {                                                                                                        \
      return Get() == other.Get() && State() == other.State();                                               \
    }                                                                                                        \
  };

#define KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_ENUM(ownerType, property, type, lowercaseName, \
                                                                   uppercaseName)                            \
  KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_VALUE(ownerType, property, type, lowercaseName,      \
                                                              uppercaseName)

#define KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_SHORTHAND(ownerType, property, type,           \
                                                                        lowercaseName, uppercaseName)        \
  template <>                                                                                                \
  struct CoordinatedValueListPropertyAccessor<css::PropertyId::property>                                     \
  {                                                                                                          \
    using Type = type;                                                                                       \
                                                                                                             \
    ownerType &Value;                                                                                        \
                                                                                                             \
    KRYS_NODISCARD type Get() const noexcept                                                                 \
    {                                                                                                        \
      return Value.Get##uppercaseName();                                                                     \
    }                                                                                                        \
    void Set(type &&lowercaseName) noexcept                                                                  \
    {                                                                                                        \
      Value.Set##uppercaseName(krys::move(lowercaseName));                                                   \
    }                                                                                                        \
    void Fill(type &&lowercaseName) noexcept                                                                 \
    {                                                                                                        \
      Value.Fill##uppercaseName(krys::move(lowercaseName));                                                  \
    }                                                                                                        \
    void Clear() noexcept                                                                                    \
    {                                                                                                        \
      Value.Clear##uppercaseName();                                                                          \
    }                                                                                                        \
                                                                                                             \
    KRYS_NODISCARD static type Initial() noexcept                                                            \
    {                                                                                                        \
      return ownerType::Initial##uppercaseName();                                                            \
    }                                                                                                        \
    KRYS_NODISCARD bool IsUnset() const noexcept                                                             \
    {                                                                                                        \
      return Value.Is##uppercaseName##Unset();                                                               \
    }                                                                                                        \
    KRYS_NODISCARD bool IsSet() const noexcept                                                               \
    {                                                                                                        \
      return Value.Is##uppercaseName##Set();                                                                 \
    }                                                                                                        \
    KRYS_NODISCARD bool IsFilled() const noexcept                                                            \
    {                                                                                                        \
      return Value.Is##uppercaseName##Filled();                                                              \
    }                                                                                                        \
  };                                                                                                         \
                                                                                                             \
  template <>                                                                                                \
  struct CoordinatedValueListPropertyConstAccessor<css::PropertyId::property>                                \
  {                                                                                                          \
    using Type = type;                                                                                       \
                                                                                                             \
    const ownerType &Value;                                                                                  \
                                                                                                             \
    KRYS_NODISCARD type Get() const noexcept                                                                 \
    {                                                                                                        \
      return Value.Get##uppercaseName();                                                                     \
    }                                                                                                        \
                                                                                                             \
    KRYS_NODISCARD static type Initial() noexcept                                                            \
    {                                                                                                        \
      return ownerType::Initial##uppercaseName();                                                            \
    }                                                                                                        \
    KRYS_NODISCARD bool IsUnset() const noexcept                                                             \
    {                                                                                                        \
      return Value.Is##uppercaseName##Unset();                                                               \
    }                                                                                                        \
    KRYS_NODISCARD bool IsSet() const noexcept                                                               \
    {                                                                                                        \
      return Value.Is##uppercaseName##Set();                                                                 \
    }                                                                                                        \
    KRYS_NODISCARD bool IsFilled() const noexcept                                                            \
    {                                                                                                        \
      return Value.Is##uppercaseName##Filled();                                                              \
    }                                                                                                        \
  };

#define KRYS_DECLARE_COORDINATED_VALUE_LIST_IS_SET_AND_IS_FILLED_MEMBERS(ownerType, property, type,          \
                                                                         lowercaseName, uppercaseName)       \
  uint8 _##lowercaseName##State : 2 {static_cast<uint8>(CoordinatedValueListPropertyState::Unset)};

#define KRYS_INTERNAL_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_COMMON(ownerType, property, type,    \
                                                                               lowercaseName, uppercaseName) \
  KRYS_NODISCARD CoordinatedValueListPropertyState uppercaseName##State() const noexcept                     \
  {                                                                                                          \
    return static_cast<CoordinatedValueListPropertyState>(Data()._##lowercaseName##State);                   \
  }                                                                                                          \
  KRYS_NODISCARD bool Is##uppercaseName##Unset() const noexcept                                              \
  {                                                                                                          \
    return uppercaseName##State() == CoordinatedValueListPropertyState::Unset;                               \
  }                                                                                                          \
  KRYS_NODISCARD bool Is##uppercaseName##Set() const noexcept                                                \
  {                                                                                                          \
    return uppercaseName##State() == CoordinatedValueListPropertyState::Set;                                 \
  }                                                                                                          \
  KRYS_NODISCARD bool Is##uppercaseName##Filled() const noexcept                                             \
  {                                                                                                          \
    return uppercaseName##State() == CoordinatedValueListPropertyState::Filled;                              \
  }

#define KRYS_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_REFERENCE(ownerType, property, type,          \
                                                                         lowercaseName, uppercaseName)       \
  void Clear##uppercaseName() noexcept                                                                       \
  {                                                                                                          \
    Data()._##lowercaseName = ownerType::Initial##uppercaseName();                                           \
    Data()._##lowercaseName##State = static_cast<uint8>(CoordinatedValueListPropertyState::Unset);           \
  }                                                                                                          \
  void Set##uppercaseName(type &&lowercaseName) noexcept                                                     \
  {                                                                                                          \
    Data()._##lowercaseName = krys::move(lowercaseName);                                                     \
    Data()._##lowercaseName##State = static_cast<uint8>(CoordinatedValueListPropertyState::Set);             \
  }                                                                                                          \
  void Fill##uppercaseName(type &&lowercaseName) noexcept                                                    \
  {                                                                                                          \
    Data()._##lowercaseName = krys::move(lowercaseName);                                                     \
    Data()._##lowercaseName##State = static_cast<uint8>(CoordinatedValueListPropertyState::Filled);          \
  }                                                                                                          \
  KRYS_INTERNAL_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_COMMON(ownerType, property, type,          \
                                                                         lowercaseName, uppercaseName)

#define KRYS_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_VALUE(ownerType, property, type,              \
                                                                     lowercaseName, uppercaseName)           \
  void Clear##uppercaseName() noexcept                                                                       \
  {                                                                                                          \
    Data()._##lowercaseName = ownerType::Initial##uppercaseName();                                           \
    Data()._##lowercaseName##State = static_cast<uint8>(CoordinatedValueListPropertyState::Unset);           \
  }                                                                                                          \
  void Set##uppercaseName(type lowercaseName) noexcept                                                       \
  {                                                                                                          \
    Data()._##lowercaseName = lowercaseName;                                                                 \
    Data()._##lowercaseName##State = static_cast<uint8>(CoordinatedValueListPropertyState::Set);             \
  }                                                                                                          \
  void Fill##uppercaseName(type lowercaseName) noexcept                                                      \
  {                                                                                                          \
    Data()._##lowercaseName = lowercaseName;                                                                 \
    Data()._##lowercaseName##State = static_cast<uint8>(CoordinatedValueListPropertyState::Filled);          \
  }                                                                                                          \
  KRYS_INTERNAL_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_COMMON(ownerType, property, type,          \
                                                                         lowercaseName, uppercaseName)

#define KRYS_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_ENUM(ownerType, property, type,               \
                                                                    lowercaseName, uppercaseName)            \
  void Clear##uppercaseName() noexcept                                                                       \
  {                                                                                                          \
    Data()._##lowercaseName = static_cast<uint8>(ownerType::Initial##uppercaseName());                       \
    Data()._##lowercaseName##State = static_cast<uint8>(CoordinatedValueListPropertyState::Unset);           \
  }                                                                                                          \
  void Set##uppercaseName(type lowercaseName) noexcept                                                       \
  {                                                                                                          \
    Data()._##lowercaseName = static_cast<uint8>(lowercaseName);                                             \
    Data()._##lowercaseName##State = static_cast<uint8>(CoordinatedValueListPropertyState::Set);             \
  }                                                                                                          \
  void Fill##uppercaseName(type lowercaseName) noexcept                                                      \
  {                                                                                                          \
    Data()._##lowercaseName = static_cast<uint8>(lowercaseName);                                             \
    Data()._##lowercaseName##State = static_cast<uint8>(CoordinatedValueListPropertyState::Filled);          \
  }                                                                                                          \
  KRYS_INTERNAL_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_COMMON(ownerType, property, type,          \
                                                                         lowercaseName, uppercaseName)

namespace krys::boo::style
{
  // Applies the provided functor to each property of the CoordinatedValueList value, invoking the functor
  // with each properties `css::PropertyId` as the first template argument, and then and'ing the results.
  template <CoordinatedValueListValue T, typename F>
  constexpr bool AllOfCoordinatedValueListProperties(KRYS_NOESCAPE F &&f) noexcept
  {
    return std::apply([&]<typename... Ts>(const Ts &...)
                      { return (f.template operator()<Ts::value>() && ...); }, T::Properties);
  }

  // Applies the provided functor to each property of the CoordinatedValueList value, invoking the functor
  // with each properties `css::PropertyId` as the first template argument, and then or'ing the results.
  template <CoordinatedValueListValue T, typename F>
  constexpr bool AnyOfCoordinatedValueListProperties(KRYS_NOESCAPE F &&f) noexcept
  {
    return std::apply([&]<typename... Ts>(const Ts &...)
                      { return (f.template operator()<Ts::value>() || ...); }, T::Properties);
  }

  // Applies the provided functor to each property of the CoordinatedValueList value, invoking the functor
  // with each properties `css::PropertyId` as the first template argument.
  template <CoordinatedValueListValue T, typename F>
  constexpr void EachCoordinatedValueListProperties(KRYS_NOESCAPE F &&f) noexcept
  {
    std::apply([&]<typename... Ts>(const Ts &...) { (f.template operator()<Ts::value>(), ...); },
               T::Properties);
  }

  template <CoordinatedValueListValue T>
  KRYS_NODISCARD bool AllNonBasePropertiesAreUnsetOrFilled(const T &value) noexcept
  {
    return AllOfCoordinatedValueListProperties<T>(
      [&value]<css::PropertyId propertyId>()
      {
        using PropertyAccessor = CoordinatedValueListPropertyConstAccessor<propertyId>;

        if constexpr (propertyId == T::BaseProperty)
        {
          return true;
        }
        else
        {
          return !PropertyAccessor {value}.IsSet();
        }
      });
  }

  template <CoordinatedValueListValue T>
  KRYS_NODISCARD bool AllPropertiesAreUnsetOrFilled(const T &value) noexcept
  {
    return AllOfCoordinatedValueListProperties<T>(
      [&value]<css::PropertyId propertyId>()
      {
        using PropertyAccessor = CoordinatedValueListPropertyConstAccessor<propertyId>;
        return !PropertyAccessor {value}.IsSet();
      });
  }
}
