#pragma once

#include "Krystal.Booey/SVG/Graphics/Filters/SVGFilterGraph.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo
{
  /// @brief The graph of a filter's primitive elements, before they create their effects.
  class SVGFilterPrimitiveGraph final : public SVGFilterGraph<svg::SVGFilterPrimitiveStandardAttributes>
  {
  public:
    SVGFilterPrimitiveGraph() noexcept = default;

    void AddNamedNode(const dom::DOMStringAtom &name,
                      Ref<svg::SVGFilterPrimitiveStandardAttributes> &&node) noexcept override
    {
      if (name.View().empty())
      {
        _lastNode = krys::move(node);
        return;
      }

      _lastNode = krys::move(node);
      _namedNodes.insert_or_assign(name, Ref<svg::SVGFilterPrimitiveStandardAttributes>(_lastNode));
    }

  private:
    KRYS_NODISCARD RefPtr<svg::SVGFilterPrimitiveStandardAttributes>
      GetNamedNode(const dom::DOMStringAtom &name) const noexcept override
    {
      if (name.View().empty())
      {
        return _lastNode;
      }

      auto it = _namedNodes.find(name);
      return it != _namedNodes.end() ? RefPtr<svg::SVGFilterPrimitiveStandardAttributes>(it->second)
                                     : nullptr;
    }
  };
}
