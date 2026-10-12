#pragma once

#include "Krystal.Booey\Style\Values\Types.hpp"

/// @summary Utilities and macros for implementing coordinating list property groups.
/// @see https://drafts.csswg.org/css-values-4/#coordinating-list-property
namespace krys::boo::style
{

  template <typename T>
  concept CoordinatedValueListValue = requires(T value) {
    { T::ComputedValueUsesUsedValues } -> SameType<const bool &>;
    { T::BaseProperty.Value } -> SameType<const css::PropertyId &>;
    { value.Clone(std::declval<const T &>()) } -> SameType<T>;
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

#define KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY(ownerType, property, type, lowercaseName,               \
                                                     uppercaseName)                                          \
  css::PropertyNameConstant<css::PropertyId::##property> {},

#define KRYS_INTERNAL_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_COMMON(ownerType, property, type,     \
                                                                              name)                          \
  static type Initial()                                                                                      \
  {                                                                                                          \
    return ownerType::Initial##name();                                                                       \
  }                                                                                                          \
  bool IsUnset() const noexcept                                                                              \
  {                                                                                                          \
    return value.Is##name##Unset();                                                                          \
  }                                                                                                          \
  bool IsSet() const noexcept                                                                                \
  {                                                                                                          \
    return value.Is##name##Set();                                                                            \
  }                                                                                                          \
  bool IsFilled() const noexcept                                                                             \
  {                                                                                                          \
    return value.Is##name##Filled();                                                                         \
  }                                                                                                          \
  CoordinatedValueListPropertyState State() const noexcept                                                   \
  {                                                                                                          \
    return value.name##State();                                                                              \
  }

#define KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_REFERENCE(ownerType, property, type,           \
                                                                        lowercaseName, uppercaseName)        \
  template <>                                                                                                \
  struct CoordinatedValueListPropertyAccessor<css::PropertyId::##property>                                   \
  {                                                                                                          \
    using Type = type;                                                                                       \
                                                                                                             \
    ownerType &value;                                                                                        \
                                                                                                             \
    const type &Get() const noexcept                                                                         \
    {                                                                                                        \
      return value.lowercaseName();                                                                          \
    }                                                                                                        \
    void Set(type &&lowercaseName) noexcept                                                                  \
    {                                                                                                        \
      value.Set##uppercaseName(WTF::move(lowercaseName));                                                    \
    }                                                                                                        \
    void Fill(type &&lowercaseName) noexcept                                                                 \
    {                                                                                                        \
      value.Fill##uppercaseName(WTF::move(lowercaseName));                                                   \
    }                                                                                                        \
    void Clear() noexcept                                                                                    \
    {                                                                                                        \
      value.Clear##uppercaseName();                                                                          \
    }                                                                                                        \
                                                                                                             \
    KRYS_INTERNAL_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_COMMON(ownerType, property, type,         \
                                                                          uppercaseName)                     \
                                                                                                             \
    bool operator==(                                                                                         \
      const CoordinatedValueListPropertyAccessor<css::PropertyId::##property> &other) const noexcept         \
    {                                                                                                        \
      return Get() == other.Get() && State() == other.State();                                               \
    }                                                                                                        \
  };                                                                                                         \
                                                                                                             \
  template <>                                                                                                \
  struct CoordinatedValueListPropertyConstAccessor<css::PropertyId::##property>                              \
  {                                                                                                          \
    using Type = type;                                                                                       \
                                                                                                             \
    const ownerType &value;                                                                                  \
                                                                                                             \
    const type &Get() const noexcept                                                                         \
    {                                                                                                        \
      return value.lowercaseName();                                                                          \
    }                                                                                                        \
                                                                                                             \
    KRYS_INTERNAL_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_COMMON(ownerType, property, type,         \
                                                                          uppercaseName)                     \
                                                                                                             \
    bool operator==(                                                                                         \
      const CoordinatedValueListPropertyConstAccessor<css::PropertyId::##property> &other) const noexcept    \
    {                                                                                                        \
      return Get() == other.Get() && State() == other.State();                                               \
    }                                                                                                        \
  };

#define DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_VALUE(ownerType, property, type, lowercaseName,     \
                                                               uppercaseName)                                \
  template <>                                                                                                \
  struct CoordinatedValueListPropertyAccessor<css::PropertyId::##property>                                   \
  {                                                                                                          \
    using Type = type;                                                                                       \
                                                                                                             \
    ownerType &value;                                                                                        \
                                                                                                             \
    type get() const                                                                                         \
    {                                                                                                        \
      return value.lowercaseName();                                                                          \
    }                                                                                                        \
    void set(type lowercaseName)                                                                             \
    {                                                                                                        \
      value.set##uppercaseName(lowercaseName);                                                               \
    }                                                                                                        \
    void fill(type lowercaseName)                                                                            \
    {                                                                                                        \
      value.fill##uppercaseName(lowercaseName);                                                              \
    }                                                                                                        \
    void clear()                                                                                             \
    {                                                                                                        \
      value.clear##uppercaseName();                                                                          \
    }                                                                                                        \
                                                                                                             \
    INTERNAL_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_COMMON(ownerType, property, type,              \
                                                                     lowercaseName, uppercaseName)           \
                                                                                                             \
    bool operator==(const CoordinatedValueListPropertyAccessor<css::PropertyId::##property> &other) const    \
    {                                                                                                        \
      return get() == other.get() && state() == other.state();                                               \
    }                                                                                                        \
  };                                                                                                         \
                                                                                                             \
  template <>                                                                                                \
  struct CoordinatedValueListPropertyConstAccessor<css::PropertyId::##property>                              \
  {                                                                                                          \
    using Type = type;                                                                                       \
                                                                                                             \
    const ownerType &value;                                                                                  \
                                                                                                             \
    type get() const                                                                                         \
    {                                                                                                        \
      return value.lowercaseName();                                                                          \
    }                                                                                                        \
                                                                                                             \
    INTERNAL_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_COMMON(ownerType, property, type,              \
                                                                     lowercaseName, uppercaseName)           \
                                                                                                             \
    bool                                                                                                     \
      operator==(const CoordinatedValueListPropertyConstAccessor<css::PropertyId::##property> &other) const  \
    {                                                                                                        \
      return get() == other.get() && state() == other.state();                                               \
    }                                                                                                        \
  };

#define DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_ENUM(ownerType, property, type, lowercaseName,      \
                                                              uppercaseName)                                 \
  DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_VALUE(ownerType, property, type, lowercaseName,           \
                                                         uppercaseName)

#define DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_SHORTHAND(ownerType, property, type, lowercaseName, \
                                                                   uppercaseName)                            \
  template <>                                                                                                \
  struct CoordinatedValueListPropertyAccessor<css::PropertyId::##property>                                   \
  {                                                                                                          \
    using Type = type;                                                                                       \
                                                                                                             \
    ownerType &value;                                                                                        \
                                                                                                             \
    type get() const                                                                                         \
    {                                                                                                        \
      return value.lowercaseName();                                                                          \
    }                                                                                                        \
    void set(type &&lowercaseName)                                                                           \
    {                                                                                                        \
      value.set##uppercaseName(WTF::move(lowercaseName));                                                    \
    }                                                                                                        \
    void fill(type &&lowercaseName)                                                                          \
    {                                                                                                        \
      value.fill##uppercaseName(WTF::move(lowercaseName));                                                   \
    }                                                                                                        \
    void clear()                                                                                             \
    {                                                                                                        \
      value.clear##uppercaseName();                                                                          \
    }                                                                                                        \
                                                                                                             \
    static type initial()                                                                                    \
    {                                                                                                        \
      return ownerType::initial##uppercaseName();                                                            \
    }                                                                                                        \
    bool isUnset() const                                                                                     \
    {                                                                                                        \
      return value.is##uppercaseName##Unset();                                                               \
    }                                                                                                        \
    bool isSet() const                                                                                       \
    {                                                                                                        \
      return value.is##uppercaseName##Set();                                                                 \
    }                                                                                                        \
    bool isFilled() const                                                                                    \
    {                                                                                                        \
      return value.is##uppercaseName##Filled();                                                              \
    }                                                                                                        \
  };                                                                                                         \
                                                                                                             \
  template <>                                                                                                \
  struct CoordinatedValueListPropertyConstAccessor<css::PropertyId::##property>                              \
  {                                                                                                          \
    using Type = type;                                                                                       \
                                                                                                             \
    const ownerType &value;                                                                                  \
                                                                                                             \
    type get() const                                                                                         \
    {                                                                                                        \
      return value.lowercaseName();                                                                          \
    }                                                                                                        \
                                                                                                             \
    static type initial()                                                                                    \
    {                                                                                                        \
      return ownerType::initial##uppercaseName();                                                            \
    }                                                                                                        \
    bool isUnset() const                                                                                     \
    {                                                                                                        \
      return value.is##uppercaseName##Unset();                                                               \
    }                                                                                                        \
    bool isSet() const                                                                                       \
    {                                                                                                        \
      return value.is##uppercaseName##Set();                                                                 \
    }                                                                                                        \
    bool isFilled() const                                                                                    \
    {                                                                                                        \
      return value.is##uppercaseName##Filled();                                                              \
    }                                                                                                        \
  };

#define DECLARE_COORDINATED_VALUE_LIST_IS_SET_AND_IS_FILLED_MEMBERS(ownerType, property, type,               \
                                                                    lowercaseName, uppercaseName)            \
  PREFERRED_TYPE(CoordinatedValueListPropertyState)                                                          \
  unsigned m_##lowercaseName##State : 2 {static_cast<unsigned>(CoordinatedValueListPropertyState::Unset)};

#define INTERNAL_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_COMMON(ownerType, property, type,         \
                                                                          lowercaseName, uppercaseName)      \
  CoordinatedValueListPropertyState lowercaseName##State() const                                             \
  {                                                                                                          \
    return static_cast<CoordinatedValueListPropertyState>(data().m_##lowercaseName##State);                  \
  }                                                                                                          \
  bool is##uppercaseName##Unset() const                                                                      \
  {                                                                                                          \
    return lowercaseName##State() == CoordinatedValueListPropertyState::Unset;                               \
  }                                                                                                          \
  bool is##uppercaseName##Set() const                                                                        \
  {                                                                                                          \
    return lowercaseName##State() == CoordinatedValueListPropertyState::Set;                                 \
  }                                                                                                          \
  bool is##uppercaseName##Filled() const                                                                     \
  {                                                                                                          \
    return lowercaseName##State() == CoordinatedValueListPropertyState::Filled;                              \
  }

#define DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_REFERENCE(ownerType, property, type,               \
                                                                    lowercaseName, uppercaseName)            \
  void clear##uppercaseName()                                                                                \
  {                                                                                                          \
    data().m_##lowercaseName = ownerType::initial##uppercaseName();                                          \
    data().m_##lowercaseName##State = static_cast<unsigned>(CoordinatedValueListPropertyState::Unset);       \
  }                                                                                                          \
  void set##uppercaseName(type &&lowercaseName)                                                              \
  {                                                                                                          \
    data().m_##lowercaseName = WTF::move(lowercaseName);                                                     \
    data().m_##lowercaseName##State = static_cast<unsigned>(CoordinatedValueListPropertyState::Set);         \
  }                                                                                                          \
  void fill##uppercaseName(type &&lowercaseName)                                                             \
  {                                                                                                          \
    data().m_##lowercaseName = WTF::move(lowercaseName);                                                     \
    data().m_##lowercaseName##State = static_cast<unsigned>(CoordinatedValueListPropertyState::Filled);      \
  }                                                                                                          \
  INTERNAL_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_COMMON(ownerType, property, type,               \
                                                                    lowercaseName, uppercaseName)

#define DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_VALUE(ownerType, property, type, lowercaseName,    \
                                                                uppercaseName)                               \
  void clear##uppercaseName()                                                                                \
  {                                                                                                          \
    data().m_##lowercaseName = ownerType::initial##uppercaseName();                                          \
    data().m_##lowercaseName##State = static_cast<unsigned>(CoordinatedValueListPropertyState::Unset);       \
  }                                                                                                          \
                                                                                                             \
  void set##uppercaseName(type lowercaseName)                                                                \
  {                                                                                                          \
    data().m_##lowercaseName = lowercaseName;                                                                \
    data().m_##lowercaseName##State = static_cast<unsigned>(CoordinatedValueListPropertyState::Set);         \
  }                                                                                                          \
  void fill##uppercaseName(type lowercaseName)                                                               \
  {                                                                                                          \
    data().m_##lowercaseName = lowercaseName;                                                                \
    data().m_##lowercaseName##State = static_cast<unsigned>(CoordinatedValueListPropertyState::Filled);      \
  }                                                                                                          \
  INTERNAL_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_COMMON(ownerType, property, type,               \
                                                                    lowercaseName, uppercaseName)

#define DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_ENUM(ownerType, property, type, lowercaseName,     \
                                                               uppercaseName)                                \
  void clear##uppercaseName()                                                                                \
  {                                                                                                          \
    data().m_##lowercaseName = static_cast<unsigned>(ownerType::initial##uppercaseName());                   \
    data().m_##lowercaseName##State = static_cast<unsigned>(CoordinatedValueListPropertyState::Unset);       \
  }                                                                                                          \
                                                                                                             \
  void set##uppercaseName(type lowercaseName)                                                                \
  {                                                                                                          \
    data().m_##lowercaseName = static_cast<unsigned>(lowercaseName);                                         \
    data().m_##lowercaseName##State = static_cast<unsigned>(CoordinatedValueListPropertyState::Set);         \
  }                                                                                                          \
  void fill##uppercaseName(type lowercaseName)                                                               \
  {                                                                                                          \
    data().m_##lowercaseName = static_cast<unsigned>(lowercaseName);                                         \
    data().m_##lowercaseName##State = static_cast<unsigned>(CoordinatedValueListPropertyState::Filled);      \
  }                                                                                                          \
  INTERNAL_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_COMMON(ownerType, property, type,               \
                                                                    lowercaseName, uppercaseName)

  // Applies the provided functor to each property of the CoordinatedValueList value, invoking the functor
  // with each properties css::PropertyId as the first template argument, and then and'ing the results.
  template <CoordinatedValueListValue T, typename F>
  constexpr bool allOfCoordinatedValueListProperties(NOESCAPE F &&f)
  {
    return WTF::apply([&]<typename... Ts>(const Ts &...)
                      { return (f.template operator()<Ts::value>() && ...); }, T::properties);
  }

  // Applies the provided functor to each property of the CoordinatedValueList value, invoking the functor
  // with each properties css::PropertyId as the first template argument, and then or'ing the results.
  template <CoordinatedValueListValue T, typename F>
  constexpr bool anyOfCoordinatedValueListProperties(NOESCAPE F &&f)
  {
    return WTF::apply([&]<typename... Ts>(const Ts &...)
                      { return (f.template operator()<Ts::value>() || ...); }, T::properties);
  }

  // Applies the provided functor to each property of the CoordinatedValueList value, invoking the functor
  // with each properties css::PropertyId as the first template argument.
  template <CoordinatedValueListValue T, typename F>
  constexpr void eachCoordinatedValueListProperties(NOESCAPE F &&f)
  {
    WTF::apply([&]<typename... Ts>(const Ts &...) { (f.template operator()<Ts::value>(), ...); },
               T::properties);
  }

  template <CoordinatedValueListValue T>
  bool allNonBasePropertiesAreUnsetOrFilled(const T &value)
  {
    return allOfCoordinatedValueListProperties<T>(
      [&value]<css::PropertyId propertyID>()
      {
        using PropertyAccessor = CoordinatedValueListPropertyConstAccessor<propertyID>;

        if constexpr (propertyID == T::baseProperty)
        {
          return true;
        }
        else
        {
          return !PropertyAccessor {value}.isSet();
        }
      });
  }

  template <CoordinatedValueListValue T>
  bool allPropertiesAreUnsetOrFilled(const T &value)
  {
    return allOfCoordinatedValueListProperties<T>(
      [&value]<css::PropertyId propertyID>()
      {
        using PropertyAccessor = CoordinatedValueListPropertyConstAccessor<propertyID>;
        return !PropertyAccessor {value}.isSet();
      });
  }
}