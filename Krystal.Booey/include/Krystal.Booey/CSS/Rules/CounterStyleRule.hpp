#pragma once

#include "Krystal.Booey/CSS/Counter/CounterStyleDescriptors.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"

namespace krys::boo::css
{
  /// @brief `@counter-style`.
  class CounterStyleRule final : public Rule
  {
  private:
    CSSOMStringAtom _name;
    CounterStyleDescriptors _descriptors;

    CounterStyleRule(const CSSOMStringAtom &name, CounterStyleDescriptors &&descriptors) noexcept;
    CounterStyleRule(const CounterStyleRule &other) noexcept = default;

  public:
    ~CounterStyleRule() noexcept;

    KRYS_NODISCARD static Ref<CounterStyleRule> Create(const CSSOMStringAtom &name,
                                                       CounterStyleDescriptors &&descriptors) noexcept;

    KRYS_NODISCARD Ref<CounterStyleRule> Copy() const noexcept;

    KRYS_NODISCARD const CounterStyleDescriptors &Descriptors() const noexcept
    {
      return _descriptors;
    }

    KRYS_NODISCARD CounterStyleDescriptors &MutableDescriptors() noexcept
    {
      return _descriptors;
    }

    KRYS_NODISCARD const CSSOMStringAtom &Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD CSSOMString System() const noexcept
    {
      return _descriptors.SystemCSSText();
    }

    KRYS_NODISCARD CSSOMString Negative() const noexcept
    {
      return _descriptors.NegativeCSSText();
    }

    KRYS_NODISCARD CSSOMString Prefix() const noexcept
    {
      return _descriptors.PrefixCSSText();
    }

    KRYS_NODISCARD CSSOMString Suffix() const noexcept
    {
      return _descriptors.SuffixCSSText();
    }

    KRYS_NODISCARD CSSOMString Range() const noexcept
    {
      return _descriptors.RangesCSSText();
    }

    KRYS_NODISCARD CSSOMString Pad() const noexcept
    {
      return _descriptors.PadCSSText();
    }

    KRYS_NODISCARD CSSOMString Fallback() const noexcept
    {
      return _descriptors.FallbackCSSText();
    }

    KRYS_NODISCARD CSSOMString Symbols() const noexcept
    {
      return _descriptors.SymbolsCSSText();
    }

    KRYS_NODISCARD CSSOMString AdditiveSymbols() const noexcept
    {
      return _descriptors.AdditiveSymbolsCSSText();
    }

    KRYS_NODISCARD CSSOMString SpeakAs() const noexcept
    {
      return {};
    }

    void SetName(const CSSOMStringAtom &name) noexcept
    {
      _name = name;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CounterStyleRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsCounterStyleRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
