#pragma once

#include "Krystal.Booey/CSS/DOMMatrix2DInit.hpp"
#include "Krystal.Booey/SVG/Properties/SVGValuePropertyList.hpp"
#include "Krystal.Booey/SVG/SVGTransform.hpp"

namespace krys::boo::svg
{
  /// @see https://svgwg.org/svg2-draft/coords.html#InterfaceSVGTransformList
  class SVGTransformList final : public SVGValuePropertyList<SVGTransform>
  {
    friend class SVGViewSpec;
    using Base = SVGValuePropertyList<SVGTransform>;
    using Base::Base;

    enum class ListReplacement : bool
    {
      Append,
      Replace
    };

  public:
    KRYS_NODISCARD static Ref<SVGTransformList> Create() noexcept
    {
      return AdoptRef(*new SVGTransformList());
    }

    KRYS_NODISCARD static Ref<SVGTransformList> Create(SVGPropertyOwner *owner,
                                                       SVGPropertyAccess access) noexcept
    {
      return AdoptRef(*new SVGTransformList(owner, access));
    }

    KRYS_NODISCARD static Ref<SVGTransformList> Create(const SVGTransformList &other,
                                                       SVGPropertyAccess access) noexcept
    {
      return AdoptRef(*new SVGTransformList(other, access));
    }

    KRYS_NODISCARD dom::ExceptionOr<Ref<SVGTransform>>
      CreateSVGTransformFromMatrix(css::DOMMatrix2DInit &&matrixInit) noexcept
    {
      Ref svgTransform = SVGTransform::Create();
      (void)svgTransform->SetMatrix(krys::move(matrixInit));
      return svgTransform;
    }

    dom::ExceptionOr<RefPtr<SVGTransform>> Consolidate() noexcept;
    KRYS_NODISCARD AffineTransform Concatenate() const noexcept;

    void Parse(dom::DOMStringView) noexcept;
    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override;

  private:
    bool ParseGeneric(dom::DOMStringView &buffer, ListReplacement = ListReplacement::Append) noexcept;
  };
}
