#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/BuilderStateInlines.hpp"

#include "Krystal.Booey/CSS/Calc/RandomCachingKey.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Values/CanvasValue.hpp"
#include "Krystal.Booey/CSS/Values/CrossFadeValue.hpp"
#include "Krystal.Booey/CSS/Values/CursorImageValue.hpp"
#include "Krystal.Booey/CSS/Values/FilterImageValue.hpp"
#include "Krystal.Booey/CSS/Values/GradientValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageSetValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/CSS/Values/NamedImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PaintImageValue.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCursorImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Style/FontSizeFunctions.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::style
{
  BuilderState::BuilderState(RenderStyle &style) noexcept : _style(style)
  {
  }

  BuilderState::BuilderState(RenderStyle &style, BuilderContext &&context) noexcept
      : _style(style), _context(krys::move(context)), _cssToLengthConversionData(style, *this)
  {
  }

  BuilderState::~BuilderState() noexcept = default;

  float BuilderState::ZoomWithTextZoomFactor() noexcept
  {
    // FIXME: Documents have no frame, so there's no text zoom factor (`-webkit-text-zoom` isn't supported).
    return CssToLengthConversionData().Zoom();
  }

  // SVG handles zooming in a different way compared to CSS. The whole document is scaled instead
  // of each individual length value in the render style / tree. CSSPrimitiveValue::resolveAsLength*()
  // multiplies each resolved length with the zoom multiplier - so for SVG we need to disable that.
  // Though all CSS values that can be applied to outermost <svg> elements (width/height/border/padding...)
  // need to respect the scaling. RenderBox (the parent class of LegacyRenderSVGRoot) grabs values like
  // width/height/border/padding/... from the RenderStyle -> for SVG these values would never scale,
  // if we'd pass a 1.0 zoom factor everywhere. So we only pass a zoom factor of 1.0 for specific
  // properties that are NOT allowed to scale within a zoomed SVG document (letter/word-spacing/font-size).
  bool BuilderState::UseSVGZoomRules() const noexcept
  {
    return Is<svg::SVGElement>(GetElement());
  }

  bool BuilderState::UseSVGZoomRulesForLength() const noexcept
  {
    auto *svgElement = DynamicDowncast<svg::SVGElement>(GetElement());
    return svgElement && !(Is<svg::SVGSvgElement>(*svgElement) && svgElement->ParentNode());
  }

  RefPtr<StyleImage> BuilderState::CreateStyleImage(const css::Value &value) const noexcept
  {
    if (const auto *imageValue = DynamicDowncast<css::ImageValue>(value))
    {
      return imageValue->CreateStyleImage(*this);
    }
    if (const auto *imageSetValue = DynamicDowncast<css::ImageSetValue>(value))
    {
      return imageSetValue->CreateStyleImage(*this);
    }
    if (const auto *imageValue = DynamicDowncast<css::CursorImageValue>(value))
    {
      return imageValue->CreateStyleImage(*this);
    }
    if (const auto *imageValue = DynamicDowncast<css::NamedImageValue>(value))
    {
      return imageValue->CreateStyleImage(*this);
    }
    if (const auto *cssCanvasValue = DynamicDowncast<css::CanvasValue>(value))
    {
      return cssCanvasValue->CreateStyleImage(*this);
    }
    if (const auto *crossfadeValue = DynamicDowncast<css::CrossFadeValue>(value))
    {
      return crossfadeValue->CreateStyleImage(*this);
    }
    if (const auto *filterImageValue = DynamicDowncast<css::FilterImageValue>(value))
    {
      return filterImageValue->CreateStyleImage(*this);
    }
    if (const auto *gradientValue = DynamicDowncast<css::GradientValue>(value))
    {
      return gradientValue->CreateStyleImage(*this);
    }
    if (const auto *paintImageValue = DynamicDowncast<css::PaintImageValue>(value))
    {
      return paintImageValue->CreateStyleImage(*this);
    }
    return nullptr;
  }

  void BuilderState::RegisterContentAttribute(const css::CSSOMStringAtom &attributeLocalName) noexcept
  {
    if (Style().GetPseudoElementType() == PseudoElementType::Before
        || Style().GetPseudoElementType() == PseudoElementType::After)
    {
      _registeredContentAttributes.push_back(attributeLocalName);
    }
  }

  void BuilderState::AdjustStyleForInterCharacterRuby() noexcept
  {
    if (!_style.IsInterCharacterRubyPosition() || !GetElement() || !Is<html::HTMLElement>(*GetElement())
        || GetElement()->TagQualifiedName().TagName() != html::TagName::rt)
    {
      return;
    }

    _style.SetTextAlign(TextAlign::Center);
    if (!_style.GetWritingMode().IsVerticalTypographic())
    {
      std::ignore = _style.SetWritingMode(StyleWritingMode::VerticalLr);
    }
  }

  void BuilderState::UpdateFont() noexcept
  {
    // FIXME: The font needs an update when it's dirty or the cascade has no fonts, and cascades have no fonts
    // until there's a font backend, so it always needs one.

    UpdateFontForGenericFamilyChange();
    UpdateFontForZoomChange();
    UpdateFontForOrientationChange();
    UpdateFontForSizeChange();

    // FIXME: Update the font cascade's fonts from the document's font selector once there is one.

    _fontDirty = false;
  }

  void BuilderState::UpdateFontForZoomChange() noexcept
  {
    // FIXME: Compare the text zoom too once there's a frame (`-webkit-text-zoom` isn't supported).
    if (_style.UsedZoom() == ParentStyle().UsedZoom())
    {
      return;
    }

    SetFontDescriptionFontSize(_style.GetFontDescription().SpecifiedSize());
  }

  void BuilderState::UpdateFontForGenericFamilyChange() noexcept
  {
    const auto &childFont = _style.GetFontDescription();

    if (childFont.IsAbsoluteSize())
    {
      return;
    }

    const auto &parentFont = ParentStyle().GetFontDescription();
    if (childFont.UseFixedDefaultSize() == parentFont.UseFixedDefaultSize())
    {
      return;
    }

    // We know the parent is monospace or the child is monospace, and that font
    // size was unspecified. We want to scale our font size as appropriate.
    // If the font uses a keyword size, then we refetch from the table rather than
    // multiplying by our scale factor.
    float size = [&]
    {
      if (css::ValueId sizeIdentifier = childFont.KeywordSizeAsIdentifier();
          sizeIdentifier != css::ValueId::Invalid)
      {
        return style::FontSizeForKeyword(static_cast<uint32>(sizeIdentifier), childFont.UseFixedDefaultSize(),
                                         GetDocument());
      }

      const auto &settings = DocumentSettingsValues(GetDocument());
      auto fixedSize = settings.DefaultFixedFontSize;
      auto defaultSize = settings.DefaultFontSize;
      float fixedScaleFactor =
        (fixedSize && defaultSize) ? static_cast<float>(fixedSize) / static_cast<float>(defaultSize) : 1;
      return parentFont.UseFixedDefaultSize() ? childFont.SpecifiedSize() / fixedScaleFactor
                                              : childFont.SpecifiedSize() * fixedScaleFactor;
    }();

    auto newFontDescription = childFont;
    SetFontSize(newFontDescription, size);
    std::ignore = _style.SetFontDescriptionWithoutUpdate(krys::move(newFontDescription));
  }

  void BuilderState::UpdateFontForOrientationChange() noexcept
  {
    auto [fontOrientation, glyphOrientation] = _style.FontAndGlyphOrientation();

    const auto &fontDescription = _style.GetFontDescription();
    if (fontDescription.Orientation() == fontOrientation
        && fontDescription.GetNonCJKGlyphOrientation() == glyphOrientation)
    {
      return;
    }

    auto newFontDescription = fontDescription;
    newFontDescription.SetNonCJKGlyphOrientation(glyphOrientation);
    newFontDescription.SetOrientation(fontOrientation);
    std::ignore = _style.SetFontDescriptionWithoutUpdate(krys::move(newFontDescription));
  }

  void BuilderState::UpdateFontForSizeChange() noexcept
  {
    _style.SynchronizeLetterSpacingWithFontCascadeWithoutUpdate();
    _style.SynchronizeWordSpacingWithFontCascadeWithoutUpdate();
  }

  void BuilderState::SetFontSize(FontCascadeDescription &fontDescription, float size) noexcept
  {
    fontDescription.SetSpecifiedSize(size);
    auto computedFontSize = style::ComputedFontSizeFromSpecifiedSize(
      size, fontDescription.IsAbsoluteSize(), UseSVGZoomRules(), &Style(), GetDocument());
    fontDescription.SetComputedSize(computedFontSize.Size, computedFontSize.UsedZoomFactor);
  }

  css::PropertyId BuilderState::CssPropertyId() const noexcept
  {
    return _currentProperty ? _currentProperty->Id : css::PropertyId::Invalid;
  }

  bool BuilderState::IsCurrentPropertyInvalidAtComputedValueTime() const noexcept
  {
    return _invalidAtComputedValueTimeProperties.test(static_cast<size_t>(CssPropertyId()));
  }

  void BuilderState::SetCurrentPropertyInvalidAtComputedValueTime() noexcept
  {
    _invalidAtComputedValueTimeProperties.set(static_cast<size_t>(CssPropertyId()));
  }

  void BuilderState::SetUsesViewportUnits() noexcept
  {
    _style.SetUsesViewportUnits();
  }

  void BuilderState::SetUsesContainerUnits() noexcept
  {
    _style.SetUsesContainerUnits();
  }

  double BuilderState::LookupCSSRandomBaseValue(const css::calc::RandomCachingKey &,
                                                Maybe<css::keywords::ElementShared>) const noexcept
  {
    // FIXME: The element and document `random()` caches come with the resolver.
    krys_unreachable();
  }

  // MARK: - Tree Counting Functions

  uint32 BuilderState::SiblingCount() noexcept
  {
    // https://drafts.csswg.org/css-values-5/#funcdef-sibling-count

    krys_debug_assert(GetElement());

    RefPtr parent = ShareRefPtr(GetElement()->ParentElement());
    if (!parent)
    {
      return 1;
    }

    _style.SetUsesTreeCountingFunctions();
    // FIXME: Mark the parent's children as affected by forward and backward positional rules once elements
    // keep their style relations (invalidation).

    uint32 count = 1;
    for (auto sibling = GetElement()->PreviousElementSibling(); sibling;
         sibling = sibling->PreviousElementSibling())
    {
      ++count;
    }
    for (auto sibling = GetElement()->NextElementSibling(); sibling; sibling = sibling->NextElementSibling())
    {
      ++count;
    }
    return count;
  }

  uint32 BuilderState::SiblingIndex() noexcept
  {
    // https://drafts.csswg.org/css-values-5/#funcdef-sibling-index

    krys_debug_assert(GetElement());

    RefPtr parent = ShareRefPtr(GetElement()->ParentElement());
    if (!parent)
    {
      return 1;
    }

    _style.SetUsesTreeCountingFunctions();
    // FIXME: Mark the parent's children as affected by forward and backward positional rules once elements
    // keep their style relations (invalidation).

    uint32 count = 1;
    for (auto sibling = GetElement()->PreviousElementSibling(); sibling;
         sibling = sibling->PreviousElementSibling())
    {
      ++count;
    }
    return count;
  }

  void BuilderState::DisableNativeAppearanceIfNeeded(css::PropertyId propertyID,
                                                     PropertyCascade::Origin origin) noexcept
  {
    auto shouldDisable = [&]
    {
      if (origin != PropertyCascade::Origin::Author)
      {
        return false;
      }
      if (!css::Property::DisablesNativeAppearance(propertyID))
      {
        return false;
      }
      if (!ApplyPropertyToRegularStyle())
      {
        return false;
      }
      // FIXME: `Element::isDevolvableWidget()` comes with the form controls; the devolvable widgets are the
      // elements the user agent style gives an appearance.
      return RenderTheme::HasAppearanceForElementTypeFromUAStyle(*GetElement());
    };

    if (shouldDisable())
    {
      _style.SetNativeAppearanceDisabled(true);
    }
  }

  void BuilderState::SetCurrentProperty(const PropertyCascade::Property *property) noexcept
  {
    _currentProperty = property;
  }
}
