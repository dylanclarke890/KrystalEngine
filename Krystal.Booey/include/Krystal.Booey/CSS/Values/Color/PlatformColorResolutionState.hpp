#pragma once

#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Style/ForVisitedLink.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorOptions.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  enum class LightDarkColorAppearance : bool;

  // Used to resolve a css::Color to a boo::Color (platform color) without going through style::Color / style
  // building.

  class PlatformColorResolutionDelegate
  {
  public:
    virtual ~PlatformColorResolutionDelegate() noexcept;

    // Colors to use that usually get resolved dynamically using Document & RenderStyle.

    /// @brief For `ValueId::Currentcolor`.
    KRYS_NODISCARD virtual boo::Color CurrentColor() const noexcept;

    /// @brief For `ValueId::InternalDocumentTextColor`.
    KRYS_NODISCARD virtual boo::Color InternalDocumentTextColor() const noexcept;

    /// @brief For `ValueId::WebkitLink` [`style::ForVisitedLink::No`].
    KRYS_NODISCARD virtual boo::Color WebkitLink() const noexcept;

    /// @brief For `ValueId::WebkitLink` [`style::ForVisitedLink::Yes`].
    KRYS_NODISCARD virtual boo::Color WebkitLinkVisited() const noexcept;

    /// @brief For `ValueId::WebkitActivelink`.
    KRYS_NODISCARD virtual boo::Color WebkitActiveLink() const noexcept;

    /// @brief For `ValueId::WebkitFocusRingColor`.
    KRYS_NODISCARD virtual boo::Color WebkitFocusRingColor() const noexcept;
  };

  struct PlatformColorResolutionState
  {
    // Delegate for lazily computing color values.
    PlatformColorResolutionDelegate *Delegate {nullptr};

    // Level of nesting inside other colors the resolution currently is.
    uint32 NestingLevel {0};

    // Conversion data needed to evaluate `calc()` expressions with relative length units.
    // If unset, colors that require conversion data will return the invalid Color.
    Maybe<ToLengthConversionData> ConversionData {};

    // Whether links should be resolved to the visited style.
    style::ForVisitedLink ForVisitedLink {style::ForVisitedLink::No};

    // Options to pass when resolving any other keyword with css::ColorFromKeyword()
    style::ColorOptions KeywordOptions {style::ColorOptions::None};

    // Appearance used to select from a light-dark() color function.
    // If unset, light-dark() colors will return the invalid Color.
    Maybe<LightDarkColorAppearance> Appearance {};

    // Colors are resolved:
    //   1. Checking if the color is set below, and if it is, returning it.
    //   2. If a delegate has been set, calling the associated delegate function,
    //      storing the result below, and returning that color.
    //   3. Returning the invalid `Color` value.
    mutable Maybe<boo::Color> ResolvedCurrentColor {};
    mutable Maybe<boo::Color> ResolvedInternalDocumentTextColor {};
    mutable Maybe<boo::Color> ResolvedWebkitLink {};
    mutable Maybe<boo::Color> ResolvedWebkitLinkVisited {};
    mutable Maybe<boo::Color> ResolvedWebkitActiveLink {};
    mutable Maybe<boo::Color> ResolvedWebkitFocusRingColor {};

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
                                             const noexcept) const noexcept
    {
      if (existing)
      {
        return *existing;
      }

      if (Delegate)
      {
        auto resolved = (Delegate->*resolver)();
        existing = resolved;
        return resolved;
      }

      return {};
    }
  };

  /// @brief RAII helper to increment/decrement nesting level.
  struct PlatformColorResolutionStateNester
  {
    KRYS_NON_COPY_MOVABLE(PlatformColorResolutionStateNester);

    PlatformColorResolutionState &State;

    explicit PlatformColorResolutionStateNester(PlatformColorResolutionState &state) noexcept : State {state}
    {
      State.NestingLevel++;
    }

    ~PlatformColorResolutionStateNester() noexcept
    {
      State.NestingLevel--;
    }
  };
}
