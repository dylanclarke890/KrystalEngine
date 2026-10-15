#include "Krystal.Booey/Style/Computed/ComputedStyleBase.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyleBase+SettersInlines.hpp"

#include "Krystal.Booey/Animation/ScrollAxis.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/PathTraversalState.hpp"
#include "Krystal.Booey/Graphics/Transforms/ScaleTransformOperation.hpp"
#include "Krystal.Booey/Rendering/MotionPath.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/SVGRenderStyle.hpp"

#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"
#include "Krystal.Booey/Rendering/TransformOperationData.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Values/NonStandard/WebKitLocale.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Booey/Style/Values/Text/TextTransform.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationLine.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/ScaleTransformFunction.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <algorithm>

namespace krys::boo::style
{
  static_assert(PublicPseudoIDBits == AllPublicPseudoElementTypes.Size());
  static_assert(!(static_cast<unsigned>(maxTextTransformValue) >> TextTransformBits));

  // Value zero is used to indicate no pseudo-element.
  static_assert(!((ToUnderlying(boo::PseudoElementType::HighestEnumValue) + 1) >> PseudoElementTypeBits));

  ComputedStyleBase::~ComputedStyleBase() noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(!_deletionHasBegun);
    _deletionHasBegun = true;
#endif
  }

  // MARK: - Pseudo element/style

  Maybe<PseudoElementIdentifier> ComputedStyleBase::GetPseudoElementIdentifier() const noexcept
  {
    if (!GetPseudoElementType())
    {
      return {};
    }
    return PseudoElementIdentifier {*GetPseudoElementType(), PseudoElementNameArgument()};
  }

  RenderStyle *ComputedStyleBase::GetCachedPseudoStyle(
    const PseudoElementIdentifier &pseudoElementIdentifier) const noexcept
  {
    if (!_cachedPseudoStyles)
    {
      return nullptr;
    }

    auto it = _cachedPseudoStyles->Styles.find(pseudoElementIdentifier);
    return it == _cachedPseudoStyles->Styles.end() ? nullptr : it->second.get();
  }

  RenderStyle *ComputedStyleBase::AddCachedPseudoStyle(UniquePtr<RenderStyle> pseudo) noexcept
  {
    if (!pseudo)
    {
      return nullptr;
    }

    krys_debug_assert(pseudo->GetPseudoElementType());

    auto *result = pseudo.get();

    if (!_cachedPseudoStyles)
    {
      _cachedPseudoStyles = CreateUnique<PseudoStyleCache>();
    }

    _cachedPseudoStyles->Styles.emplace(*result->GetPseudoElementIdentifier(), krys::move(pseudo));

    return result;
  }

  // MARK: - Custom properties

  const CustomProperty *
    ComputedStyleBase::CustomPropertyValue(const css::CSSOMStringAtom &name) const noexcept
  {
    for (auto *map : {&NonInheritedCustomProperties(), &InheritedCustomProperties()})
    {
      if (const auto *value = map->Get(name))
      {
        return value;
      }
    }
    return nullptr;
  }

  void ComputedStyleBase::SetCustomPropertyValue(Ref<const CustomProperty> &&value, bool isInherited) noexcept
  {
    auto &name = value->GetName();
    if (isInherited)
    {
      if (const auto *existingValue = _rareInheritedData->CustomProperties->Get(name);
          !existingValue || *existingValue != *value)
      {
        _rareInheritedData.Access().CustomProperties.Access().Set(name, krys::move(value));
      }
    }
    else
    {
      if (const auto *existingValue = _nonInheritedData->RareData->CustomProperties->Get(name);
          !existingValue || *existingValue != *value)
      {
        _nonInheritedData.Access().RareData.Access().CustomProperties.Access().Set(name, krys::move(value));
      }
    }
  }

  bool ComputedStyleBase::CustomPropertyValueEqual(const ComputedStyleBase &other,
                                                   const css::CSSOMStringAtom &name) const noexcept
  {
    if (&NonInheritedCustomProperties() == &other.NonInheritedCustomProperties()
        && &InheritedCustomProperties() == &other.InheritedCustomProperties())
    {
      return true;
    }

    auto *value = CustomPropertyValue(name);
    auto *otherValue = other.CustomPropertyValue(name);
    if (value == otherValue)
    {
      return true;
    }
    if (!value || !otherValue)
    {
      return false;
    }
    return *value == *otherValue;
  }

  bool ComputedStyleBase::CustomPropertiesEqual(const ComputedStyleBase &other) const noexcept
  {
    return _nonInheritedData->RareData->CustomProperties
             == other._nonInheritedData->RareData->CustomProperties
           && _rareInheritedData->CustomProperties == other._rareInheritedData->CustomProperties;
  }

  void ComputedStyleBase::DeduplicateCustomProperties(const ComputedStyleBase &other) noexcept
  {
    auto deduplicate = [&]<typename T>(const DataRef<T> &data, const DataRef<T> &otherData)
    {
      auto &properties = const_cast<DataRef<CustomPropertyData> &>(data->CustomProperties);
      auto &otherProperties = otherData->CustomProperties;
      if (properties.Ptr() == otherProperties.Ptr() || *properties != *otherProperties)
      {
        return;
      }
      properties = otherProperties;
    };

    deduplicate(_rareInheritedData, other._rareInheritedData);
    deduplicate(_nonInheritedData->RareData, other._nonInheritedData->RareData);
  }

  // MARK: - Custom paint

  void ComputedStyleBase::AddCustomPaintWatchProperty(const css::CSSOMStringAtom &name) noexcept
  {
    auto &data = _nonInheritedData.Access().RareData.Access();
    data.CustomPaintWatchedProperties.insert(name);
  }

  // MARK: - FontCascade support.

  FontCascade &ComputedStyleBase::MutableFontCascadeWithoutUpdate() noexcept
  {
    return _inheritedData.Access().FontData.Access().FontCascade;
  }

  void ComputedStyleBase::SetFontCascade(FontCascade &&fontCascade) noexcept
  {
    if (fontCascade == this->GetFontCascade())
    {
      return;
    }

    _inheritedData.Access().FontData.Access().FontCascade = fontCascade;
  }

  // MARK: - FontCascadeDescription support.

  const FontCascadeDescription &ComputedStyleBase::GetFontDescription() const noexcept
  {
    return _inheritedData->FontData->FontCascade.GetFontDescription();
  }

  FontCascadeDescription &ComputedStyleBase::MutableFontDescriptionWithoutUpdate() noexcept
  {
    auto &cascade = _inheritedData.Access().FontData.Access().FontCascade;
    return cascade.MutableFontDescription();
  }

  void ComputedStyleBase::SetFontDescription(FontCascadeDescription &&description) noexcept
  {
    if (GetFontDescription() == description)
    {
      return;
    }

    // FIXME: Update the new font cascade with the existing cascade's font selector once fonts load.
    SetFontCascade(FontCascade {krys::move(description), GetFontCascade()});
  }

  bool ComputedStyleBase::SetFontDescriptionWithoutUpdate(FontCascadeDescription &&description) noexcept
  {
    if (GetFontDescription() == description)
    {
      return false;
    }

    auto &cascade = _inheritedData.Access().FontData.Access().FontCascade;
    cascade = FontCascade {krys::move(description), cascade};
    return true;
  }

  const FontMetrics &ComputedStyleBase::MetricsOfPrimaryFont() const noexcept
  {
    return _inheritedData->FontData->FontCascade.MetricsOfPrimaryFont();
  }

  std::pair<FontOrientation, NonCJKGlyphOrientation> ComputedStyleBase::FontAndGlyphOrientation() noexcept
  {
    if (!GetWritingMode().IsVerticalTypographic())
    {
      return {FontOrientation::Horizontal, NonCJKGlyphOrientation::Mixed};
    }

    switch (GetWritingMode().ComputedTextOrientation())
    {
      case TextOrientation::Mixed:    return {FontOrientation::Vertical, NonCJKGlyphOrientation::Mixed};
      case TextOrientation::Upright:  return {FontOrientation::Vertical, NonCJKGlyphOrientation::Upright};
      case TextOrientation::Sideways: return {FontOrientation::Horizontal, NonCJKGlyphOrientation::Mixed};
      default:
        krys_debug_assert_msg(false, "Unreachable");
        return {FontOrientation::Horizontal, NonCJKGlyphOrientation::Mixed};
    }
  }

  float ComputedStyleBase::ComputedFontSize() const noexcept
  {
    return GetFontDescription().ComputedSize();
  }

  const LineHeight &ComputedStyleBase::SpecifiedLineHeight() const noexcept
  {
    return _inheritedData->LineHeight;
  }

  void ComputedStyleBase::SetLetterSpacingFromAnimation(LetterSpacing &&value) noexcept
  {
    if (value != _inheritedData->FontData->LetterSpacing)
    {
      _inheritedData.Access().FontData.Access().LetterSpacing = value;

      SynchronizeLetterSpacingWithFontCascade();
    }
  }

  void ComputedStyleBase::SetWordSpacingFromAnimation(WordSpacing &&value) noexcept
  {
    if (value != _inheritedData->FontData->WordSpacing)
    {
      _inheritedData.Access().FontData.Access().WordSpacing = value;

      SynchronizeWordSpacingWithFontCascade();
    }
  }

  void ComputedStyleBase::SynchronizeLetterSpacingWithFontCascade() noexcept
  {
    auto &fontCascade = MutableFontCascadeWithoutUpdate();
    auto fontSize = fontCascade.Size();

    auto newLetterSpacing =
      Evaluate<float>(_inheritedData->FontData->LetterSpacing, fontSize, UsedZoomForLength());

    if (newLetterSpacing != fontCascade.LetterSpacing())
    {
      fontCascade.SetLetterSpacing(newLetterSpacing);

      auto oldFontDescription = GetFontDescription();

      bool oldShouldDisableLigatures = oldFontDescription.ShouldDisableLigaturesForSpacing();
      bool newShouldDisableLigatures = newLetterSpacing != 0;

      // Switching letter-spacing between zero and non-zero requires updating to enable/disable ligatures.
      if (oldShouldDisableLigatures != newShouldDisableLigatures)
      {
        auto newFontDescription = oldFontDescription;
        newFontDescription.SetShouldDisableLigaturesForSpacing(newShouldDisableLigatures);
        SetFontDescription(krys::move(newFontDescription));
      }
    }
  }

  void ComputedStyleBase::SynchronizeLetterSpacingWithFontCascadeWithoutUpdate() noexcept
  {
    auto &fontCascade = MutableFontCascadeWithoutUpdate();
    auto fontSize = fontCascade.Size();

    auto newLetterSpacing =
      Evaluate<float>(_inheritedData->FontData->LetterSpacing, fontSize, UsedZoomForLength());

    if (newLetterSpacing != fontCascade.LetterSpacing())
    {
      fontCascade.SetLetterSpacing(newLetterSpacing);

      auto oldFontDescription = GetFontDescription();

      bool oldShouldDisableLigatures = oldFontDescription.ShouldDisableLigaturesForSpacing();
      bool newShouldDisableLigatures = newLetterSpacing != 0;

      // Switching letter-spacing between zero and non-zero requires updating to enable/disable ligatures.
      if (oldShouldDisableLigatures != newShouldDisableLigatures)
      {
        auto newFontDescription = oldFontDescription;
        newFontDescription.SetShouldDisableLigaturesForSpacing(newShouldDisableLigatures);
        std::ignore = SetFontDescriptionWithoutUpdate(krys::move(newFontDescription));
      }
    }
  }

  void ComputedStyleBase::SynchronizeWordSpacingWithFontCascade() noexcept
  {
    auto &fontCascade = MutableFontCascadeWithoutUpdate();
    auto fontSize = fontCascade.Size();

    auto newWordSpacing =
      Evaluate<float>(_inheritedData->FontData->WordSpacing, fontSize, UsedZoomForLength());

    if (newWordSpacing != fontCascade.WordSpacing())
    {
      fontCascade.SetWordSpacing(newWordSpacing);
    }
  }

  void ComputedStyleBase::SynchronizeWordSpacingWithFontCascadeWithoutUpdate() noexcept
  {
    SynchronizeWordSpacingWithFontCascade();
  }

  // MARK: - Properties/descriptors that are not yet generated

  const CounterDirectiveMap &ComputedStyleBase::GetCounterDirectives() const noexcept
  {
    return _nonInheritedData->RareData->CounterDirectives;
  }

  CounterDirectiveMap &ComputedStyleBase::AccessCounterDirectives() noexcept
  {
    return _nonInheritedData.Access().RareData.Access().CounterDirectives;
  }

}
