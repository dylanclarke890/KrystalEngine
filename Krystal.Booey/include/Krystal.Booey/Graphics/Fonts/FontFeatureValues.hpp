#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief A feature value name and its values, e.g. `nice-style: 12` in `@styleset`.
  using FontFeatureValuesTag = Pair<css::CSSOMString, List<uint32>>;

  /// @brief The feature value blocks of `@font-feature-values`.
  /// @see https://drafts.csswg.org/css-fonts-4/#font-feature-values
  enum class FontFeatureValuesType : uint8
  {
    Styleset,
    Stylistic,
    CharacterVariant,
    Swash,
    Ornaments,
    Annotation
  };

  /// @brief The named feature values defined by `@font-feature-values` for a font family.
  class FontFeatureValues : public RefCounted<FontFeatureValues>
  {
  public:
    using Tags = HashMap<css::CSSOMString, List<uint32>>;

  private:
    Tags _styleset;
    Tags _stylistic;
    Tags _characterVariant;
    Tags _swash;
    Tags _ornaments;
    Tags _annotation;

    FontFeatureValues() noexcept = default;

  public:
    KRYS_NODISCARD static Ref<FontFeatureValues> Create() noexcept
    {
      return AdoptRef(*new FontFeatureValues());
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _styleset.empty() && _stylistic.empty() && _characterVariant.empty() && _swash.empty()
             && _ornaments.empty() && _annotation.empty();
    }

    KRYS_NODISCARD bool operator==(const FontFeatureValues &other) const noexcept
    {
      return _styleset == other._styleset && _stylistic == other._stylistic
             && _characterVariant == other._characterVariant && _swash == other._swash
             && _ornaments == other._ornaments && _annotation == other._annotation;
    }

    KRYS_NODISCARD const Tags &Styleset() const noexcept
    {
      return _styleset;
    }

    KRYS_NODISCARD Tags &Styleset() noexcept
    {
      return _styleset;
    }

    KRYS_NODISCARD const Tags &Stylistic() const noexcept
    {
      return _stylistic;
    }

    KRYS_NODISCARD Tags &Stylistic() noexcept
    {
      return _stylistic;
    }

    KRYS_NODISCARD const Tags &CharacterVariant() const noexcept
    {
      return _characterVariant;
    }

    KRYS_NODISCARD Tags &CharacterVariant() noexcept
    {
      return _characterVariant;
    }

    KRYS_NODISCARD const Tags &Swash() const noexcept
    {
      return _swash;
    }

    KRYS_NODISCARD Tags &Swash() noexcept
    {
      return _swash;
    }

    KRYS_NODISCARD const Tags &Ornaments() const noexcept
    {
      return _ornaments;
    }

    KRYS_NODISCARD Tags &Ornaments() noexcept
    {
      return _ornaments;
    }

    KRYS_NODISCARD const Tags &Annotation() const noexcept
    {
      return _annotation;
    }

    KRYS_NODISCARD Tags &Annotation() noexcept
    {
      return _annotation;
    }

    /// @brief Adds every tag of @p other, replacing the values of tags that already exist.
    void UpdateOrInsert(const FontFeatureValues &other) noexcept;

    /// @brief Adds @p tags to the block for @p type, replacing the values of tags that already exist.
    void UpdateOrInsertForType(FontFeatureValuesType type, const List<FontFeatureValuesTag> &tags) noexcept;

    KRYS_NODISCARD size_t Hash() const noexcept;
  };
}

namespace krys
{
  template <>
  struct DefaultHash<boo::FontFeatureValues>
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = false;

    KRYS_NODISCARD static size_t Hash(const boo::FontFeatureValues &values) noexcept
    {
      return values.Hash();
    }

    KRYS_NODISCARD static bool Equal(const boo::FontFeatureValues &a,
                                     const boo::FontFeatureValues &b) noexcept
    {
      return a == b;
    }
  };
}
