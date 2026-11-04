#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/BloomFilter.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class Selector;
}

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::style
{
  enum class MatchElement : uint8;

  /// @brief A bloom filter of the identifiers of an element's children or descendants, for fast rejection of
  /// `:has()` arguments.
  class HasSelectorFilter
  {
  public:
    enum class Type : uint8
    {
      Children,
      Descendants
    };

    using Key = uint32;

  private:
    const Type _type;
    BloomFilter<12> _filter;

  public:
    HasSelectorFilter(const dom::Element &element, Type type) noexcept;

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD static Maybe<Type> TypeForMatchElement(MatchElement matchElement) noexcept;

    KRYS_NODISCARD static Key MakeKey(const css::Selector &hasSelector) noexcept;

    KRYS_NODISCARD bool Reject(const css::Selector &hasSelector) const noexcept
    {
      return Reject(MakeKey(hasSelector));
    }

    KRYS_NODISCARD bool Reject(Key key) const noexcept
    {
      return key && !_filter.MayContain(key);
    }

  private:
    void Add(const dom::Element &element) noexcept;
  };
}
