#pragma once

#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontFamilyName.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <iterator>

namespace krys::boo::style
{
  enum class FontFamilyKind : bool
  {
    Specified,
    Generic
  };

  // <single-font-family> = [ <family-name> | <generic-family> ]
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-family
  struct SingleFontFamily
  {
    css::CSSOMStringAtom Value;

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      using namespace css::WebKitFontFamilyNames;

      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      // <generic-family>
      if (Value == CursiveFamily())
      {
        return visitor(css::keywords::Cursive {});
      }
      if (Value == FantasyFamily())
      {
        return visitor(css::keywords::Fantasy {});
      }
      if (Value == MonospaceFamily())
      {
        return visitor(css::keywords::Monospace {});
      }
      if (Value == MathFamily())
      {
        return visitor(css::keywords::Math {});
      }
      if (Value == PictographFamily())
      {
        return visitor(css::keywords::WebkitPictograph {});
      }
      if (Value == SansSerifFamily())
      {
        return visitor(css::keywords::SansSerif {});
      }
      if (Value == SerifFamily())
      {
        return visitor(css::keywords::Serif {});
      }
      if (Value == SystemUiFamily())
      {
        return visitor(css::keywords::SystemUi {});
      }
      // <family-name>
      return visitor(FontFamilyName {Value});
    }

    KRYS_NODISCARD bool operator==(const SingleFontFamily &) const noexcept = default;
  };

  // <'font-family'> = [ <family-name> | <generic-family> ]#
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-family
  struct FontFamilies
  {
  private:
    List<css::CSSOMStringAtom> _families;
    FontFamilyKind _firstFontKind {FontFamilyKind::Generic};

  public:
    FontFamilies(List<css::CSSOMStringAtom> &&families, FontFamilyKind firstFontKind) noexcept
        : _families {krys::move(families)}, _firstFontKind {firstFontKind}
    {
    }

    FontFamilies(List<css::CSSOMStringAtom> &&families, bool isSpecifiedFont) noexcept
        : FontFamilies {krys::move(families),
                        isSpecifiedFont ? FontFamilyKind::Specified : FontFamilyKind::Generic}
    {
    }

    FontFamilies(css::CSSOMStringAtom family, FontFamilyKind firstFontKind) noexcept
        : FontFamilies {List<css::CSSOMStringAtom> {krys::move(family)}, firstFontKind}
    {
    }

    class const_iterator
    {
    public:
      using inner_iterator = List<css::CSSOMStringAtom>::const_iterator;
      using iterator_category = std::forward_iterator_tag;
      using value_type = SingleFontFamily;
      using difference_type = std::ptrdiff_t;

    private:
      inner_iterator _it;

    public:
      const_iterator() noexcept = default;
      const_iterator(inner_iterator it) noexcept : _it {it}
      {
      }

      KRYS_NODISCARD value_type operator*() const noexcept
      {
        return SingleFontFamily {*_it};
      }

      const_iterator &operator++() noexcept
      {
        ++_it;
        return *this;
      }
      const_iterator operator++(int) noexcept
      {
        auto result = *this;
        ++(*this);
        return result;
      }

      KRYS_NODISCARD bool operator==(const const_iterator &) const noexcept = default;
    };

    KRYS_NODISCARD const_iterator begin() const noexcept KRYS_LIFETIME_BOUND
    {
      return const_iterator(_families.begin());
    }
    KRYS_NODISCARD const_iterator end() const noexcept KRYS_LIFETIME_BOUND
    {
      return const_iterator(_families.end());
    }

    KRYS_NODISCARD uint32 size() const noexcept
    {
      return static_cast<uint32>(_families.size());
    }

    KRYS_NODISCARD SingleFontFamily First() const noexcept
    {
      return SingleFontFamily {_families.front()};
    }
    KRYS_NODISCARD SingleFontFamily Last() const noexcept
    {
      return SingleFontFamily {_families.back()};
    }

    KRYS_NODISCARD List<css::CSSOMStringAtom> &ToPlatform() noexcept KRYS_LIFETIME_BOUND
    {
      return _families;
    }
    KRYS_NODISCARD const List<css::CSSOMStringAtom> &ToPlatform() const noexcept KRYS_LIFETIME_BOUND
    {
      return _families;
    }
    KRYS_NODISCARD List<css::CSSOMStringAtom> TakePlatform() noexcept
    {
      return krys::move(_families);
    }

    KRYS_NODISCARD FontFamilyKind FirstFontKind() const noexcept
    {
      return _firstFontKind;
    }
    KRYS_NODISCARD bool IsSpecifiedFont() const noexcept
    {
      return _firstFontKind == FontFamilyKind::Specified;
    }

    KRYS_NODISCARD bool operator==(const FontFamilies &) const noexcept = default;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontFamilies>
  {
    KRYS_NODISCARD FontFamilies operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_COMMA_SEPARATED_RANGE_LIKE_CONFORMANCE(krys::boo::style::FontFamilies)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SingleFontFamily)
