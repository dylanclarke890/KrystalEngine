#pragma once

#include "Krystal.Booey/SVG/Properties/SVGPrimitivePropertyAnimatorImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGValuePropertyAnimatorImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGValuePropertyListAnimatorImpl.hpp"
#include "Krystal.Booey/SVG/SVGLength.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::svg
{
  /// @brief Makes the animators of an element's presentation attributes, which animate its animated style
  /// properties; the animators of one attribute share the value they animate.
  class SVGPropertyAnimatorFactory
  {
    using PropertyCreator = Ref<SVGProperty> (*)() noexcept;
    using AnimatorCreator = Ref<SVGAttributeAnimator> (*)(const dom::QualifiedName &, Ref<SVGProperty> &&,
                                                          AnimationMode, CalcMode, bool, bool) noexcept;

    // This list maps an attribute name to a pair of static methods. The first one creates a shared
    // Ref<SVGProperty> for the value type of this attribute. The second creates the animator given the
    // attribute name and the shared Ref<SVGProperty>.
    struct AttributeAnimatorCreator
    {
      const dom::QualifiedName *Name;
      PropertyCreator CreateProperty;
      AnimatorCreator CreateAnimator;
    };

  private:
    List<Pair<dom::QualifiedName, Ref<SVGProperty>>> _attributeProperty;

  public:
    SVGPropertyAnimatorFactory() noexcept = default;

    KRYS_NODISCARD static bool IsKnownAttribute(const dom::QualifiedName &attributeName) noexcept
    {
      return FindCreator(attributeName);
    }

    KRYS_NODISCARD RefPtr<SVGAttributeAnimator> CreateAnimator(const dom::QualifiedName &attributeName,
                                                               AnimationMode animationMode, CalcMode calcMode,
                                                               bool isAccumulated, bool isAdditive) noexcept
    {
      const auto *creator = FindCreator(attributeName);
      if (!creator)
      {
        return nullptr;
      }

      auto iterator = std::ranges::find_if(_attributeProperty,
                                           [&](const auto &entry) { return entry.first == attributeName; });
      if (iterator == _attributeProperty.end())
      {
        _attributeProperty.emplace_back(attributeName, creator->CreateProperty());
        iterator = _attributeProperty.end() - 1;
      }

      return creator->CreateAnimator(attributeName, Ref {iterator->second}, animationMode, calcMode,
                                     isAccumulated, isAdditive);
    }

    void AnimatorWillBeDeleted(const dom::QualifiedName &attributeName) noexcept
    {
      auto iterator = std::ranges::find_if(_attributeProperty,
                                           [&](const auto &entry) { return entry.first == attributeName; });
      if (iterator == _attributeProperty.end())
      {
        return;
      }

      // If refCount = 1 (in the animator) + 1 (in _attributeProperty) = 2, the entry can be deleted.
      if (iterator->second->GetRefCount() == 2)
      {
        _attributeProperty.erase(iterator);
      }
    }

  private:
    template <typename TProperty>
    KRYS_NODISCARD static Ref<SVGProperty> CreateValueProperty() noexcept
    {
      return SVGValueProperty<TProperty>::Create();
    }

    KRYS_NODISCARD static Ref<SVGProperty> CreateLength() noexcept
    {
      return SVGLength::Create();
    }

    KRYS_NODISCARD static Ref<SVGProperty> CreateLengthList() noexcept
    {
      return SVGLengthList::Create();
    }

    template <typename TAnimator>
    KRYS_NODISCARD static Ref<SVGAttributeAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGProperty> &&property,
             AnimationMode animationMode, CalcMode calcMode, bool isAccumulated, bool isAdditive) noexcept
    {
      return TAnimator::Create(attributeName, krys::move(property), animationMode, calcMode, isAccumulated,
                               isAdditive);
    }

    KRYS_NODISCARD static const AttributeAnimatorCreator *
      FindCreator(const dom::QualifiedName &attributeName) noexcept
    {
      for (const auto &creator : AttributeAnimatorCreators())
      {
        if (*creator.Name == attributeName)
        {
          return &creator;
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD static const List<AttributeAnimatorCreator> &AttributeAnimatorCreators() noexcept
    {
      constexpr auto color = &CreateValueProperty<Color>;
      constexpr auto number = &CreateValueProperty<float>;
      constexpr auto string = &CreateValueProperty<dom::DOMString>;
      constexpr auto colorAnimator = &Create<SVGColorAnimator>;
      constexpr auto lengthAnimator = &Create<SVGLengthAnimator>;
      constexpr auto lengthListAnimator = &Create<SVGLengthListAnimator>;
      constexpr auto numberAnimator = &Create<SVGNumberAnimator>;
      constexpr auto stringAnimator = &Create<SVGStringAnimator>;

      static const List<AttributeAnimatorCreator> creators {
        {&SVGNames::ColorAttr(), color, colorAnimator},
        {&SVGNames::FillAttr(), color, colorAnimator},
        {&SVGNames::FloodColorAttr(), color, colorAnimator},
        {&SVGNames::LightingColorAttr(), color, colorAnimator},
        {&SVGNames::StopColorAttr(), color, colorAnimator},
        {&SVGNames::StrokeAttr(), color, colorAnimator},

        {&SVGNames::FontSizeAttr(), &CreateLength, lengthAnimator},
        {&SVGNames::LetterSpacingAttr(), &CreateLength, lengthAnimator},
        {&SVGNames::StrokeDashoffsetAttr(), &CreateLength, lengthAnimator},
        {&SVGNames::StrokeWidthAttr(), &CreateLength, lengthAnimator},
        {&SVGNames::WordSpacingAttr(), &CreateLength, lengthAnimator},

        {&SVGNames::StrokeDasharrayAttr(), &CreateLengthList, lengthListAnimator},

        {&SVGNames::FillOpacityAttr(), number, numberAnimator},
        {&SVGNames::FloodOpacityAttr(), number, numberAnimator},
        {&SVGNames::OpacityAttr(), number, numberAnimator},
        {&SVGNames::StopOpacityAttr(), number, numberAnimator},
        {&SVGNames::StrokeMiterlimitAttr(), number, numberAnimator},
        {&SVGNames::StrokeOpacityAttr(), number, numberAnimator},

        {&SVGNames::AlignmentBaselineAttr(), string, stringAnimator},
        {&SVGNames::BaselineShiftAttr(), string, stringAnimator},
        {&SVGNames::BufferedRenderingAttr(), string, stringAnimator},
        {&SVGNames::ClipPathAttr(), string, stringAnimator},
        {&SVGNames::ClipRuleAttr(), string, stringAnimator},
        {&SVGNames::ColorInterpolationAttr(), string, stringAnimator},
        {&SVGNames::ColorInterpolationFiltersAttr(), string, stringAnimator},
        {&SVGNames::CursorAttr(), string, stringAnimator},
        {&SVGNames::DisplayAttr(), string, stringAnimator},
        {&SVGNames::DominantBaselineAttr(), string, stringAnimator},
        {&SVGNames::FillRuleAttr(), string, stringAnimator},
        {&SVGNames::FilterAttr(), string, stringAnimator},
        {&SVGNames::FontFamilyAttr(), string, stringAnimator},
        {&SVGNames::FontStretchAttr(), string, stringAnimator},
        {&SVGNames::FontStyleAttr(), string, stringAnimator},
        {&SVGNames::FontVariantAttr(), string, stringAnimator},
        {&SVGNames::FontWeightAttr(), string, stringAnimator},
        {&SVGNames::ImageRenderingAttr(), string, stringAnimator},
        {&SVGNames::MarkerEndAttr(), string, stringAnimator},
        {&SVGNames::MarkerMidAttr(), string, stringAnimator},
        {&SVGNames::MarkerStartAttr(), string, stringAnimator},
        {&SVGNames::MaskAttr(), string, stringAnimator},
        {&SVGNames::MaskTypeAttr(), string, stringAnimator},
        {&SVGNames::OverflowAttr(), string, stringAnimator},
        {&SVGNames::PaintOrderAttr(), string, stringAnimator},
        {&SVGNames::PointerEventsAttr(), string, stringAnimator},
        {&SVGNames::ShapeRenderingAttr(), string, stringAnimator},
        {&SVGNames::StrokeLinecapAttr(), string, stringAnimator},
        {&SVGNames::StrokeLinejoinAttr(), string, stringAnimator},
        {&SVGNames::TextAnchorAttr(), string, stringAnimator},
        {&SVGNames::TextDecorationAttr(), string, stringAnimator},
        {&SVGNames::TextRenderingAttr(), string, stringAnimator},
        {&SVGNames::VectorEffectAttr(), string, stringAnimator},
        {&SVGNames::VisibilityAttr(), string, stringAnimator},
      };
      return creators;
    }
  };
}
