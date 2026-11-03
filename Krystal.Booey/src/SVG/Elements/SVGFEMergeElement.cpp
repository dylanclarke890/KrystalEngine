#include "Krystal.Booey/SVG/Elements/SVGFEMergeElement.hpp"
#include "Krystal.Booey/Graphics/Filters/FEMerge.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEMergeNodeElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo::svg
{
  SVGFEMergeElement::SVGFEMergeElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEMerge)
  {
  }

  void SVGFEMergeElement::OnChildrenChanged() noexcept
  {
    SVGFilterPrimitiveStandardAttributes::OnChildrenChanged();
    InstanceInvalidationGuard guard(*this);
    MarkFilterEffectForRebuild();
  }

  List<dom::DOMStringAtom> SVGFEMergeElement::FilterEffectInputsNames() const noexcept
  {
    List<dom::DOMStringAtom> inputsNames;
    for (auto *child = FirstChild(); child; child = child->NextSibling())
    {
      if (auto *mergeNode = DynamicDowncast<SVGFEMergeNodeElement>(*child))
      {
        inputsNames.push_back(dom::DOMStringAtom(mergeNode->In1()));
      }
    }
    return inputsNames;
  }

  RefPtr<FilterEffect> SVGFEMergeElement::CreateFilterEffect(const FilterEffectVector &inputs,
                                                             const GraphicsContext &) const noexcept
  {
    return FEMerge::Create(static_cast<uint32>(inputs.size()));
  }
}
