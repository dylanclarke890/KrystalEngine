#pragma once

#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/AnchorSizeDimension.hpp"
#include "Krystal.Booey/Style/PropertyCascade.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Booey/Style/ScopeOrdinal.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionTryFallbackTactic.hpp"
#include "Krystal.Booey/Style/Values/Box/MarginTrim.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorOptions.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/AppleColorFilter.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"
#include "Krystal.Booey/Style/Values/Math/MathDepth.hpp"
#include "Krystal.Booey/Style/Values/Sizing/AspectRatio.hpp"
#include "Krystal.Booey/Style/Values/Text/TextAlign.hpp"
#include "Krystal.Booey/Style/Values/Text/TextAlignLast.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include <bitset>

// FIXME: Placeholders for the rendering, font and style-resolver types that length resolution, calc
// evaluation, colour resolution, the computed value conversions and the extractor need. Each declares only
// the members those callers use, under its upstream name. None can be constructed. Each type moves to its own
// header, with its real implementation, when its slice is ported:
//   - `style::AnchorPositionEvaluator`: anchor positioning (`Style/AnchorPositionEvaluator.hpp`, unreviewed).
//   - the `Document*` colour functions: `dom::Document` (upstream members) once it has a page and settings.
//   - the `Document*` security function: `dom::Document` (upstream member) once it has a security origin.
// A few members collapse an upstream call chain whose types aren't ported yet; they're marked below.

namespace krys::boo::dom
{
  class Document;
  class Element;
  class Node;
}

namespace krys::boo::css
{
  class CSSStyleSheet;
  class StyleProperties;
}

namespace krys::boo::css::calc
{
  struct RandomCachingKey;
}

namespace krys::boo::style
{
  struct Color;
}

namespace krys::boo
{
  class GraphicsContext;
  class RenderLayerModelObject;
  class StyleImage;

  class RenderStyle;

  class RenderView;
  class RenderBlock;
  class RenderBox;
  class RenderBoxModelObject;
  class RenderElement;
  class RenderGrid;
  class RenderInline;

  /// @brief Collapses `adjustForAbsoluteZoom(value, renderView)`.
  KRYS_NODISCARD int32 AdjustForAbsoluteZoom(int32, const RenderView &) noexcept;

  /// @brief Collapses `document.renderView()`: the document's render view, if it's shown in a frame with a
  /// view.
  KRYS_NODISCARD const RenderView *RenderViewOf(const dom::Document &) noexcept;

  /// @brief Collapses `document.useDarkAppearance(style)` for a document with no page and default settings:
  /// dark only when the style's colour scheme is dark only.
  KRYS_NODISCARD bool DocumentUseDarkAppearance(const dom::Document &, const RenderStyle *) noexcept;

  /// @brief Collapses `document.styleColorOptions(style)` for a document with no page and default settings.
  KRYS_NODISCARD style::ColorOptions DocumentStyleColorOptions(const dom::Document &,
                                                               const RenderStyle *) noexcept;

  /// @brief Collapses `document.protectedSecurityOrigin()->canRequest(url, ...)`. There's no security origin
  /// or loader yet, so every URL can be requested.
  KRYS_NODISCARD bool DocumentCanRequest(const dom::Document &, const url::URL &) noexcept;
}

namespace krys::boo::style
{
  class BuilderState;
  struct BuilderPositionTryFallback;

  class AnchorPositionEvaluator
  {
  public:
    using Side = Variant<css::ValueId, double>;

    AnchorPositionEvaluator() = delete;

    KRYS_NODISCARD static Maybe<double> Evaluate(BuilderState &, Maybe<ScopedName> elementName,
                                                 Side) noexcept;
    KRYS_NODISCARD static Maybe<double> EvaluateSize(BuilderState &, Maybe<ScopedName> elementName,
                                                     Maybe<AnchorSizeDimension>) noexcept;

    /// @brief Whether @p property takes `anchor()`: the inset properties.
    KRYS_NODISCARD static bool PropertyAllowsAnchorFunction(css::PropertyId property) noexcept
    {
      return css::Property::IsInsetProperty(property);
    }

    /// @brief Whether @p property takes `anchor-size()`: the sizing, inset and margin properties.
    KRYS_NODISCARD static bool PropertyAllowsAnchorSizeFunction(css::PropertyId property) noexcept
    {
      return css::Property::IsSizingProperty(property) || css::Property::IsInsetProperty(property)
             || css::Property::IsMarginProperty(property);
    }

    /// @brief Swaps a self-alignment position (`align-self`/`justify-self`) according to a position-try
    /// fallback's tactics.
    KRYS_NODISCARD static css::ValueId
      ResolvePositionTryFallbackValueForSelfPosition(css::PropertyId, css::ValueId position, WritingMode,
                                                     const BuilderPositionTryFallback &) noexcept;
  };
}
