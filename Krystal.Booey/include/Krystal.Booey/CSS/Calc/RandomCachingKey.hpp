#pragma once

#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include "Krystal.Core/Utils/Tags.hpp"

namespace krys::boo::css::calc
{
  struct RandomCachingKey
  {
    using IdentifierVariant = Variant<Random::SharingOptions::Auto, CSSOMStringAtom,
                                      krys::HashTableDeletedValueTag, krys::HashTableEmptyValueTag>;

    IdentifierVariant Identifier;

    RandomCachingKey(Variant<Random::SharingOptions::Auto, CSSOMStringAtom> &&identifier) noexcept
        : Identifier {krys::SwitchOn(krys::move(identifier), [](auto &&alternative)
                                     { return IdentifierVariant {krys::move(alternative)}; })}
    {
    }

    RandomCachingKey(const Variant<Random::SharingOptions::Auto, CSSOMStringAtom> &identifier) noexcept
        : Identifier {krys::SwitchOn(identifier,
                                     [](const auto &alternative) { return IdentifierVariant {alternative}; })}
    {
    }

    explicit RandomCachingKey(krys::HashTableDeletedValueTag) noexcept
        : Identifier {krys::HashTableDeletedValueTag {}}
    {
    }

    explicit RandomCachingKey(krys::HashTableEmptyValueTag) noexcept
        : Identifier {krys::HashTableEmptyValueTag {}}
    {
    }

    KRYS_NODISCARD bool IsHashTableDeletedValue() const noexcept
    {
      return std::holds_alternative<krys::HashTableDeletedValueTag>(Identifier);
    }

    KRYS_NODISCARD bool IsHashTableEmptyValue() const noexcept
    {
      return std::holds_alternative<krys::HashTableEmptyValueTag>(Identifier);
    }

    KRYS_NODISCARD bool operator==(const RandomCachingKey &other) const noexcept
    {
      if (Identifier.index() != other.Identifier.index())
      {
        return false;
      }

      return krys::SwitchOn(
        Identifier, [&](const Random::SharingOptions::Auto &autoValue)
        { return autoValue == std::get<Random::SharingOptions::Auto>(other.Identifier); },
        [&](const CSSOMStringAtom &string) { return string == std::get<CSSOMStringAtom>(other.Identifier); },
        [](const krys::HashTableDeletedValueTag &) { return true; },
        [](const krys::HashTableEmptyValueTag &) { return true; });
    }
  };

  struct RandomCachingKeyHash
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = true;

    KRYS_NODISCARD static size_t Hash(const RandomCachingKey &key) noexcept
    {
      return krys::SwitchOn(
        key.Identifier,
        [&](const Random::SharingOptions::Auto &autoValue) -> size_t
        {
          return krys::Hash::Combine(key.Identifier.index(), ToUnderlying(autoValue.Property),
                                     autoValue.Index);
        },
        [&](const CSSOMStringAtom &string) -> size_t
        { return krys::Hash::Combine(key.Identifier.index(), string); },
        [](const krys::HashTableDeletedValueTag &) -> size_t { krys_unreachable(); },
        [](const krys::HashTableEmptyValueTag &) -> size_t { krys_unreachable(); });
    }

    KRYS_NODISCARD static bool Equal(const RandomCachingKey &a, const RandomCachingKey &b) noexcept
    {
      return a == b;
    }

    // Lets `HashMap` use this as its hasher.
    KRYS_NODISCARD size_t operator()(const RandomCachingKey &key) const noexcept
    {
      return Hash(key);
    }
  };
}

namespace krys
{
  template <>
  struct HashTraits<krys::boo::css::calc::RandomCachingKey>
      : public GenericHashTraits<krys::boo::css::calc::RandomCachingKey>
  {
    constexpr static bool HasIsEmptyValueFunction = true;
    constexpr static bool EmptyValueIsZero = false;

    KRYS_NODISCARD static krys::boo::css::calc::RandomCachingKey EmptyValue() noexcept
    {
      return krys::boo::css::calc::RandomCachingKey(HashTableEmptyValueTag {});
    }

    KRYS_NODISCARD static bool IsEmptyValue(const krys::boo::css::calc::RandomCachingKey &value) noexcept
    {
      return value.IsHashTableEmptyValue();
    }

    static void ConstructDeletedValue(krys::boo::css::calc::RandomCachingKey &slot) noexcept
    {
      new (NotNullTag {}, &slot) krys::boo::css::calc::RandomCachingKey(HashTableDeletedValueTag {});
    }

    KRYS_NODISCARD static bool IsDeletedValue(const krys::boo::css::calc::RandomCachingKey &slot) noexcept
    {
      return slot.IsHashTableDeletedValue();
    }
  };

  template <>
  struct DefaultHash<krys::boo::css::calc::RandomCachingKey>
      : public krys::boo::css::calc::RandomCachingKeyHash
  {
  };
}
