#pragma once

#include "Krystal.Booey/CSS/Rules/Rule.hpp"

namespace krys::boo::css
{
  /// @brief `@position-try`.
  class PositionTryRule final : public Rule
  {
  private:
    CSSOMStringAtom _name;
    Ref<StyleProperties> _properties;

    PositionTryRule(CSSOMStringAtom &&name, Ref<StyleProperties> &&properties) noexcept;
    PositionTryRule(const PositionTryRule &other) noexcept;

  public:
    ~PositionTryRule() noexcept;

    KRYS_NODISCARD static Ref<PositionTryRule> Create(CSSOMStringAtom &&name,
                                                      Ref<StyleProperties> &&properties) noexcept;

    KRYS_NODISCARD Ref<PositionTryRule> Copy() const noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD StyleProperties &Properties() const noexcept
    {
      return *_properties;
    }

    KRYS_NODISCARD Ref<StyleProperties> ProtectedProperties() const noexcept;

    KRYS_NODISCARD MutableStyleProperties &MutableProperties() noexcept;

    KRYS_NODISCARD Ref<MutableStyleProperties> ProtectedMutableProperties() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PositionTryRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsPositionTryRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
