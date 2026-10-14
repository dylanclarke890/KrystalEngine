#pragma once

#include "Krystal.Booey/CSS/Rules/Rule.hpp"

namespace krys::boo::css
{
  /// @brief The `navigation` descriptor of `@view-transition`.
  enum class ViewTransitionNavigation : bool
  {
    Auto,
    None,
  };

  /// @brief `@view-transition`.
  class ViewTransitionRule final : public Rule
  {
  private:
    Maybe<ViewTransitionNavigation> _navigation;
    List<CSSOMStringAtom> _types;

    explicit ViewTransitionRule(Ref<StyleProperties> &&properties) noexcept;
    ViewTransitionRule(const ViewTransitionRule &other) noexcept = default;

  public:
    ~ViewTransitionRule() noexcept;

    KRYS_NODISCARD static Ref<ViewTransitionRule> Create(Ref<StyleProperties> &&properties) noexcept;

    KRYS_NODISCARD Ref<ViewTransitionRule> Copy() const noexcept;

    KRYS_NODISCARD Maybe<ViewTransitionNavigation> Navigation() const noexcept
    {
      return _navigation;
    }

    KRYS_NODISCARD ViewTransitionNavigation ComputedNavigation() const noexcept
    {
      return Navigation().value_or(ViewTransitionNavigation::None);
    }

    KRYS_NODISCARD List<CSSOMStringAtom> Types() const noexcept
    {
      return _types;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ViewTransitionRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsViewTransitionRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
