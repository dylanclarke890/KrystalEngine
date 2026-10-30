#pragma once

#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo::dom
{
  class DOMTokenList;
}

namespace krys::boo::svg
{
  /// @brief The link types of an `<a>`'s `rel` that change how it's followed.
  enum class SVGALinkRelation : uint8
  {
    NoReferrer,
    NoOpener,
    Opener
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::svg::SVGALinkRelation, 3uz);

namespace krys::boo::svg
{
  /// @brief The `<a>` element: a hyperlink around graphics.
  /// @see https://svgwg.org/svg2-draft/linking.html#InterfaceSVGAElement
  class SVGAElement final : public SVGGraphicsElement, public SVGURIReference
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGAElement);

  private:
    Ref<SVGAnimatedString> _target;

    EnumSet<SVGALinkRelation> _linkRelations;

    UniquePtr<dom::DOMTokenList> _relList;

  public:
    explicit SVGAElement(dom::Document &document) noexcept;
    ~SVGAElement() noexcept override;

    /// @see https://svgwg.org/svg2-draft/linking.html#__svg__SVGAElement__target
    KRYS_NODISCARD dom::DOMStringAtom Target() const noexcept
    {
      return dom::DOMStringAtom {_target->CurrentValue()};
    }
    KRYS_NODISCARD Ref<SVGAnimatedString> &TargetAnimated() noexcept
    {
      return _target;
    }

    /// @see https://svgwg.org/svg2-draft/linking.html#__svg__SVGAElement__relList
    KRYS_NODISCARD dom::DOMTokenList &RelList() noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGAElement, SVGGraphicsElement, SVGURIReference>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
    KRYS_NODISCARD bool ChildShouldCreateRenderer(const dom::Node &) const noexcept final;

    KRYS_NODISCARD bool IsValid() const noexcept final
    {
      return SVGTests::IsValid();
    }
    void ActivationBehavior(dom::Event &event) noexcept final;
    void DefaultEventHandler(dom::Event &event) noexcept final;

    KRYS_NODISCARD bool HasRel(SVGALinkRelation) const noexcept;

    KRYS_NODISCARD bool CanStartSelection() const noexcept final;

    void FollowTheHyperlink(dom::Event &event) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGAElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGAElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
