#pragma once

#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css::calc
{
  struct RandomCachingKey
  {
    using IdentifierVariant = Variant<Random::SharingOptions::Auto, CSSOMStringAtom,
                                      krys::HashTableDeletedValueTag, krys::HashTableEmptyValueTag>;

    IdentifierVariant Identifier;

    RandomCachingKey(Variant<Random::SharingOptions::Auto, CSSOMStringAtom> &&Identifier) noexcept
        : Identifier {krys::SwitchOn(krys::move(Identifier), [](auto &&alternative)
                                     { return IdentifierVariant {krys::move(alternative)}; })}
    {
    }

    RandomCachingKey(const Variant<Random::SharingOptions::Auto, CSSOMStringAtom> &Identifier) noexcept
        : Identifier {krys::SwitchOn(Identifier,
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

    bool operator==(const RandomCachingKey &) const noexcept = default;
  };

  struct RandomCachingKeyHash
  {
    constexpr static bool SafeToCompareToEmptyOrDeleted = true;

    KRYS_NODISCARD static size_t Hash(const krys::boo::css::calc::RandomCachingKey &key) noexcept
    {
      return krys::SwitchOn(
        key.Identifier, [&](const krys::boo::css::calc::Random::SharingOptions::Auto &autoValue)
        { return krys::Hash::Combine(key.Identifier.index(), autoValue.Property, autoValue.Index); },
        [&](const krys::boo::css::CSSOMStringAtom &string)
        { return krys::Hash::Combine(key.Identifier.index(), string); }, [](const HashTableDeletedValueTag &)
        { krys_unreachable(); }, [](const HashTableEmptyValueTag &) { krys_unreachable(); });
    }

    KRYS_NODISCARD static bool Equal(const krys::boo::css::calc::RandomCachingKey &a,
                                     const krys::boo::css::calc::RandomCachingKey &b) noexcept
    {
      return a == b;
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