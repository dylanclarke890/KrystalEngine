#pragma once

#include "Krystal.Booey/Core/Color/Color.hpp"
#include "Krystal.Booey/CSS/CSSToLengthConversionData.hpp"

namespace krys::boo::css
{
  enum class LightDarkColorAppearance : bool;

  // Used to resolve a CSS::Color to a krys::Color (platform color) without going through Style::Color / style
  // building.

  class PlatformColorResolutionDelegate
  {
  public:
    virtual ~PlatformColorResolutionDelegate() = default;

    // Colors to use that usually get resolved dynamically using Document & RenderStyle.

    /// @brief For ValueId::Currentcolor
    KRYS_NODISCARD virtual boo::Color CurrentColor() const noexcept
    {
      return {};
    }

    /// @brief For ValueId::InternalDocumentTextColor
    KRYS_NODISCARD virtual boo::Color InternalDocumentTextColor() const noexcept
    {
      return {};
    }

    /// @brief For ValueId::WebkitLink [Style::ForVisitedLink::No]
    KRYS_NODISCARD virtual boo::Color WebkitLink() const noexcept
    {
      return {};
    }

    /// @brief For ValueId::WebkitLink [Style::ForVisitedLink::Yes]
    KRYS_NODISCARD virtual boo::Color WebkitLinkVisited() const noexcept
    {
      return {};
    }

    /// @brief For ValueId::WebkitActiveLink
    KRYS_NODISCARD virtual boo::Color WebkitActiveLink() const noexcept
    {
      return {};
    }

    /// @brief For ValueId::WebkitFocusRingColor
    KRYS_NODISCARD virtual boo::Color WebkitFocusRingColor() const noexcept
    {
      return {};
    }
  };

  struct PlatformColorResolutionState
  {
    // Delegate for lazily computing color values.
    PlatformColorResolutionDelegate *delegate = nullptr;

    // Level of nesting inside other colors the resolution currently is.
    size_t nestingLevel = 0uz;

    // Conversion data needed to evaluate `calc()` expressions with relative length units.
    // If unset, colors that require conversion data will return the invalid Color.
    Maybe<CSSToLengthConversionData> conversionData = null;

    // Whether links should be resolved to the visited style.
    Style::ForVisitedLink forVisitedLink = Style::ForVisitedLink::No;

    // Options to pass when resolving any other keyword with CSS::colorFromKeyword()
    StyleColorOptions keywordOptions = {};

    // Appearance used to select from a light-dark() color function.
    // If unset, light-dark() colors will return the invalid Color.
    Maybe<LightDarkColorAppearance> appearance = std::nullopt;

    // Colors are resolved:
    //   1. Checking if the color is set below, and if it is, returning it.
    //   2. If a delegate has been set, calling the associated delegate function,
    //      storing the result below, and returning that color.
    //   3. Returning the invalid `Color` value.
    mutable Maybe<boo::Color> ResolvedCurrentColor = null;
    mutable Maybe<boo::Color> ResolvedInternalDocumentTextColor = null;
    mutable Maybe<boo::Color> ResolvedWebkitLink = null;
    mutable Maybe<boo::Color> ResolvedWebkitLinkVisited = null;
    mutable Maybe<boo::Color> ResolvedWebkitActiveLink = null;
    mutable Maybe<boo::Color> ResolvedWebkitFocusRingColor = null;

    KRYS_NODISCARD boo::Color CurrentColor() const noexcept
    {
      return ResolveColor(ResolvedCurrentColor, &PlatformColorResolutionDelegate::CurrentColor);
    }

    KRYS_NODISCARD boo::Color InternalDocumentTextColor() const noexcept
    {
      return ResolveColor(ResolvedInternalDocumentTextColor,
                          &PlatformColorResolutionDelegate::InternalDocumentTextColor);
    }

    KRYS_NODISCARD boo::Color WebkitLink() const noexcept
    {
      return ResolveColor(ResolvedWebkitLink, &PlatformColorResolutionDelegate::WebkitLink);
    }

    KRYS_NODISCARD boo::Color WebkitLinkVisited() const noexcept
    {
      return ResolveColor(ResolvedWebkitLinkVisited, &PlatformColorResolutionDelegate::WebkitLinkVisited);
    }

    KRYS_NODISCARD boo::Color WebkitActiveLink() const noexcept
    {
      return ResolveColor(ResolvedWebkitActiveLink, &PlatformColorResolutionDelegate::WebkitActiveLink);
    }

    KRYS_NODISCARD boo::Color WebkitFocusRingColor() const noexcept
    {
      return ResolveColor(ResolvedWebkitFocusRingColor,
                          &PlatformColorResolutionDelegate::WebkitFocusRingColor);
    }

  private:
    KRYS_NODISCARD boo::Color ResolveColor(Maybe<boo::Color> &existing,
                                           boo::Color (PlatformColorResolutionDelegate::*resolver)()
                                             const) const noexcept
    {
      if (existing)
      {
        return *existing;
      }

      if (delegate)
      {
        auto resolved = ((*delegate).*resolver)();
        existing = resolved;
        return resolved;
      }

      return {};
    }
  };

  // RAII helper to increment/decrement nesting level.
  struct PlatformColorResolutionStateNester
  {
    PlatformColorResolutionStateNester(PlatformColorResolutionState &state) noexcept : state {state}
    {
      state.nestingLevel++;
    }

    ~PlatformColorResolutionStateNester() noexcept
    {
      state.nestingLevel--;
    }

    PlatformColorResolutionState &state;
  };
}