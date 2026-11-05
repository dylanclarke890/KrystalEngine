#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Utils/Tags.hpp"
#include <memory>

namespace krys::boo
{
  /// @brief An OpenType four-character tag, e.g. `liga` or `wght`.
  using FontTag = Array<char, 4>;

  /// @brief Creates a tag from a null-terminated four-character string literal, e.g.
  /// `FontFeatureTag("liga")`.
  KRYS_NODISCARD inline FontTag FontFeatureTag(Span<const char, 5> nullTerminatedString) noexcept
  {
    krys_debug_assert(nullTerminatedString[4] == '\0');
    return {nullTerminatedString[0], nullTerminatedString[1], nullTerminatedString[2],
            nullTerminatedString[3]};
  }

  /// @brief Hashes a `FontTag` as the big-endian integer its four characters spell.
  struct FourCharacterTagHash
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = true;

    KRYS_NODISCARD static size_t Hash(const FontTag &tag) noexcept
    {
      uint32 integer = (static_cast<uint32>(static_cast<uint8>(tag[0])) << 24)
                       | (static_cast<uint32>(static_cast<uint8>(tag[1])) << 16)
                       | (static_cast<uint32>(static_cast<uint8>(tag[2])) << 8)
                       | static_cast<uint32>(static_cast<uint8>(tag[3]));
      return DefaultHash<uint32>::Hash(integer);
    }

    KRYS_NODISCARD static bool Equal(const FontTag &a, const FontTag &b) noexcept
    {
      return a == b;
    }

    // Lets `HashMap` hash `FontTag` keys.
    KRYS_NODISCARD size_t operator()(const FontTag &tag) const noexcept
    {
      return Hash(tag);
    }
  };

  /// @brief Hash table traits for `FontTag` keys: all zeros is empty, all `0xFF` is deleted.
  struct FourCharacterTagHashTraits : public HashTraits<FontTag>
  {
  private:
    constexpr static char ff = static_cast<char>(0xFF);

  public:
    constexpr static bool EmptyValueIsZero = true;

    static void ConstructDeletedValue(FontTag &slot) noexcept
    {
      new (NotNullTag {}, std::addressof(slot)) FontTag({{ff, ff, ff, ff}});
    }

    KRYS_NODISCARD static bool IsDeletedValue(const FontTag &value) noexcept
    {
      return value == FontTag({{ff, ff, ff, ff}});
    }
  };

  /// @brief A single tagged setting, e.g. one entry of `font-feature-settings` or `font-variation-settings`.
  template <typename T>
  class FontTaggedSetting
  {
  private:
    FontTag _tag;
    T _value;

  public:
    FontTaggedSetting() = delete;
    FontTaggedSetting(FontTag tag, T value) noexcept : _tag(tag), _value(value)
    {
    }

    KRYS_NODISCARD bool operator==(const FontTaggedSetting &) const noexcept = default;

    KRYS_NODISCARD FontTag Tag() const noexcept
    {
      return _tag;
    }

    KRYS_NODISCARD T Value() const noexcept
    {
      return _value;
    }

    KRYS_NODISCARD bool Enabled() const noexcept
    {
      return static_cast<bool>(Value());
    }
  };

  /// @brief A list of tagged settings, kept sorted by tag with at most one setting per tag.
  template <typename T>
  class FontTaggedSettings
  {
  public:
    using Setting = FontTaggedSetting<T>;

  private:
    List<FontTaggedSetting<T>> _list;

  public:
    /// @brief Inserts `feature`, replacing any setting that has the same tag.
    void insert(FontTaggedSetting<T> &&feature)
    {
      // This vector will almost always have 0 or 1 items in it. Don't bother with the overhead of a binary
      // search or a hash set. We keep the vector sorted alphabetically and replace any pre-existing value for
      // a given tag.
      size_t i;
      for (i = 0; i < _list.size(); ++i)
      {
        if (_list[i].Tag() < feature.Tag())
        {
          continue;
        }
        if (_list[i].Tag() == feature.Tag())
        {
          _list.erase(_list.begin() + static_cast<ptrdiff_t>(i));
        }
        break;
      }
      _list.insert(_list.begin() + static_cast<ptrdiff_t>(i), krys::move(feature));
    }

    KRYS_NODISCARD bool operator==(const FontTaggedSettings &) const noexcept = default;

    KRYS_NODISCARD bool isEmpty() const noexcept
    {
      return !size();
    }

    KRYS_NODISCARD size_t size() const noexcept
    {
      return _list.size();
    }

    KRYS_NODISCARD const FontTaggedSetting<T> &operator[](size_t index) const noexcept KRYS_LIFETIME_BOUND
    {
      return _list[index];
    }

    KRYS_NODISCARD const FontTaggedSetting<T> &at(size_t index) const KRYS_LIFETIME_BOUND
    {
      return _list.at(index);
    }

    KRYS_NODISCARD auto begin() const noexcept KRYS_LIFETIME_BOUND
    {
      return _list.begin();
    }

    KRYS_NODISCARD auto end() const noexcept KRYS_LIFETIME_BOUND
    {
      return _list.end();
    }
  };

  using FontFeature = FontTaggedSetting<int>;
  using FontFeatureSettings = FontTaggedSettings<int>;
  using FontVariationSettings = FontTaggedSettings<float>;
}

namespace krys
{
  template <typename T>
  struct DefaultHash<boo::FontTaggedSetting<T>>
  {
    KRYS_NODISCARD static size_t Hash(const boo::FontTaggedSetting<T> &setting) noexcept
    {
      return krys::Hash::Combine(boo::FourCharacterTagHash::Hash(setting.Tag()), setting.Value());
    }

    KRYS_NODISCARD static bool Equal(const boo::FontTaggedSetting<T> &a,
                                     const boo::FontTaggedSetting<T> &b) noexcept
    {
      return a == b;
    }
  };
}
