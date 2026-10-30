#pragma once

#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGTests.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::svg::tests
{
  /// @brief The name of a test attribute in the null namespace.
  KRYS_NODISCARD inline dom::QualifiedName TestAttributeName(dom::DOMStringView localName) noexcept
  {
    return dom::QualifiedName {dom::DOMStringAtom::Null(), dom::DOMStringAtom::Null(),
                               dom::DOMStringAtom(localName)};
  }

  /// @brief An SVG element with a property of every kind the registry reaches, standing in for the elements
  /// that aren't ported yet. It reflects some of them from attributes, and records the attributes it's told
  /// about.
  class TestSVGElement final : public SVGElement, public SVGTests, public SVGURIReference
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestSVGElement);

  public:
    using PropertyRegistry = SVGPropertyOwnerRegistry<TestSVGElement, SVGElement, SVGTests, SVGURIReference>;

    Ref<SVGAnimatedLength> X;
    Ref<SVGAnimatedLength> Dx;
    Ref<SVGAnimatedBoolean> Boolean;
    Ref<SVGAnimatedInteger> Integer;
    Ref<SVGAnimatedInteger> OrderX;
    Ref<SVGAnimatedInteger> OrderY;
    Ref<SVGAnimatedNumber> Number;
    Ref<SVGAnimatedNumber> StdDeviationX;
    Ref<SVGAnimatedNumber> StdDeviationY;
    Ref<SVGAnimatedEnumeration> Units;
    Ref<SVGAnimatedAngle> OrientAngle;
    Ref<SVGAnimatedOrientType> OrientType;
    Ref<SVGAnimatedLengthList> Lengths;
    Ref<SVGAnimatedNumberList> Numbers;
    Ref<SVGAnimatedPointList> Points;
    Ref<SVGAnimatedPreserveAspectRatio> PreserveAspectRatio;
    Ref<SVGAnimatedRect> ViewBox;
    Ref<SVGAnimatedString> String;
    Ref<SVGAnimatedTransformList> Transform;
    Ref<SVGTransformList> ViewTransform;

    List<dom::QualifiedName> ChangedAttributes;
    bool SelfRelative {false};
    uint32 ClearedTargets {0};

    explicit TestSVGElement(dom::Document &document, bool selfRelative = false) noexcept
        : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::None),
          SVGTests(this), SVGURIReference(this), X(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
          Dx(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
          Boolean(SVGAnimatedBoolean::Create(this)), Integer(SVGAnimatedInteger::Create(this, 7)),
          OrderX(SVGAnimatedInteger::Create(this, 1)), OrderY(SVGAnimatedInteger::Create(this, 1)),
          Number(SVGAnimatedNumber::Create(this, 1.5f)), StdDeviationX(SVGAnimatedNumber::Create(this)),
          StdDeviationY(SVGAnimatedNumber::Create(this)),
          Units(SVGAnimatedEnumeration::Create(this, SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE)),
          OrientAngle(SVGAnimatedAngle::Create(this)),
          OrientType(SVGAnimatedOrientType::Create(this, SVGMarkerOrientAngle)),
          Lengths(SVGAnimatedLengthList::Create(this, SVGLengthMode::Height)),
          Numbers(SVGAnimatedNumberList::Create(this)), Points(SVGAnimatedPointList::Create(this)),
          PreserveAspectRatio(SVGAnimatedPreserveAspectRatio::Create(this)),
          ViewBox(SVGAnimatedRect::Create(this)), String(SVGAnimatedString::Create(this)),
          Transform(SVGAnimatedTransformList::Create(this)),
          ViewTransform(SVGTransformList::Create(this, SVGPropertyAccess::ReadWrite)),
          SelfRelative(selfRelative)
    {
      static bool didRegistration = false;
      if (!didRegistration)
      {
        didRegistration = true;
        PropertyRegistry::RegisterProperty<&TestSVGElement::X>(SVGNames::XAttr());
        PropertyRegistry::RegisterProperty<&TestSVGElement::Dx>(TestAttributeName(u8"dx"));
        PropertyRegistry::RegisterProperty<&TestSVGElement::Boolean>(TestAttributeName(u8"preserveAlpha"));
        PropertyRegistry::RegisterProperty<&TestSVGElement::Integer>(TestAttributeName(u8"numOctaves"));
        PropertyRegistry::RegisterProperty<&TestSVGElement::OrderX, &TestSVGElement::OrderY>(
          TestAttributeName(u8"order"));
        PropertyRegistry::RegisterProperty<&TestSVGElement::Number>(TestAttributeName(u8"k1"));
        PropertyRegistry::RegisterProperty<&TestSVGElement::StdDeviationX, &TestSVGElement::StdDeviationY>(
          TestAttributeName(u8"stdDeviation"));
        PropertyRegistry::RegisterProperty<SVGUnitTypes::SVGUnitType, &TestSVGElement::Units>(
          TestAttributeName(u8"filterUnits"));
        PropertyRegistry::RegisterProperty<&TestSVGElement::OrientAngle, &TestSVGElement::OrientType>(
          TestAttributeName(u8"orient"));
        PropertyRegistry::RegisterProperty<&TestSVGElement::Lengths>(TestAttributeName(u8"lengths"));
        PropertyRegistry::RegisterProperty<&TestSVGElement::Numbers>(TestAttributeName(u8"values"));
        PropertyRegistry::RegisterProperty<&TestSVGElement::Points>(TestAttributeName(u8"points"));
        PropertyRegistry::RegisterProperty<&TestSVGElement::PreserveAspectRatio>(
          TestAttributeName(u8"preserveAspectRatio"));
        PropertyRegistry::RegisterProperty<&TestSVGElement::ViewBox>(TestAttributeName(u8"viewBox"));
        PropertyRegistry::RegisterProperty<&TestSVGElement::String>(TestAttributeName(u8"result"));
        PropertyRegistry::RegisterProperty<&TestSVGElement::Transform>(TestAttributeName(u8"transform"));
        PropertyRegistry::RegisterProperty<&TestSVGElement::ViewTransform>(
          TestAttributeName(u8"viewTransform"));
      }
    }

    KRYS_NODISCARD bool IsValid() const noexcept override
    {
      return SVGTests::IsValid();
    }

    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept override
    {
      ChangedAttributes.push_back(attributeName);
      SVGTests::SVGAttributeChanged(attributeName);
      SVGElement::SVGAttributeChanged(attributeName);
    }

    using SVGElement::DetachAllProperties;
    using SVGElement::UpdateRelativeLengthsInformation;
    using SVGURIReference::Href;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override
    {
      auto string = value.value_or(dom::DOMStringView {});
      if (name == SVGNames::XAttr())
      {
        X->SetBaseValInternal(SVGLengthValue(SVGLengthMode::Width, string));
      }
      else if (name == TestAttributeName(u8"dx"))
      {
        Dx->SetBaseValInternal(SVGLengthValue(SVGLengthMode::Width, string));
      }
      else if (name == TestAttributeName(u8"result"))
      {
        String->SetBaseValInternal(dom::DOMString {string});
      }
      else if (name == TestAttributeName(u8"transform"))
      {
        Transform->BaseVal()->Parse(string);
      }

      SVGTests::ParseAttribute(name, value);
      SVGURIReference::ParseAttribute(name, value);
      SVGElement::AttributeChanged(name, oldValue, value);
    }

    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept override
    {
      return SelfRelative;
    }

    void ClearTarget() noexcept override
    {
      ++ClearedTargets;
    }
  };

  /// @brief A test element, created in @p document.
  KRYS_NODISCARD inline Ref<TestSVGElement> CreateTestSVGElement(dom::Document &document,
                                                                 bool selfRelative = false) noexcept
  {
    return AdoptRef(*new TestSVGElement(document, selfRelative));
  }
}
