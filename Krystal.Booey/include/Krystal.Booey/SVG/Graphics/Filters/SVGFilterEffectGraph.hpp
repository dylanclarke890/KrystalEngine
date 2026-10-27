#pragma once

#include "Krystal.Booey/SVG/Graphics/Filters/SVGFilterGraph.hpp"

namespace krys::boo
{
  class FilterEffect;

  /// @brief The graph of a filter's effects, with the sources every filter has.
  class SVGFilterEffectGraph final : public SVGFilterGraph<FilterEffect>
  {
  private:
    HashMap<dom::DOMStringAtom, Ref<FilterEffect>> _sourceNodes;

  public:
    SVGFilterEffectGraph(Ref<FilterEffect> &&sourceGraphic, Ref<FilterEffect> &&sourceAlpha) noexcept
    {
      _sourceNodes.insert_or_assign(dom::DOMStringAtom(SourceGraphic::EffectName()),
                                    krys::move(sourceGraphic));
      _sourceNodes.insert_or_assign(dom::DOMStringAtom(SourceAlpha::EffectName()), krys::move(sourceAlpha));

      SetNodeInputs(*SourceGraphicNode(), NodeVector {});
      SetNodeInputs(*SourceAlphaNode(), NodeVector {ShareRef(*SourceGraphicNode())});
    }

    void AddNamedNode(const dom::DOMStringAtom &name, Ref<FilterEffect> &&node) noexcept override
    {
      if (name.View().empty())
      {
        _lastNode = krys::move(node);
        return;
      }

      if (_sourceNodes.contains(name))
      {
        return;
      }

      _lastNode = krys::move(node);
      _namedNodes.insert_or_assign(name, Ref<FilterEffect>(_lastNode));
    }

  private:
    KRYS_NODISCARD FilterEffect *SourceGraphicNode() const noexcept
    {
      return SourceNode(FilterEffect::SourceGraphicName());
    }

    KRYS_NODISCARD FilterEffect *SourceAlphaNode() const noexcept
    {
      return SourceNode(FilterEffect::SourceAlphaName());
    }

    KRYS_NODISCARD FilterEffect *SourceNode(utf8_stringview name) const noexcept
    {
      auto it = _sourceNodes.find(dom::DOMStringAtom(name));
      return it != _sourceNodes.end() ? it->second.get() : nullptr;
    }

    KRYS_NODISCARD RefPtr<FilterEffect> GetNamedNode(const dom::DOMStringAtom &name) const noexcept override
    {
      if (!name.View().empty())
      {
        if (auto it = _sourceNodes.find(name); it != _sourceNodes.end())
        {
          return it->second;
        }

        if (auto it = _namedNodes.find(name); it != _namedNodes.end())
        {
          return it->second;
        }
      }

      if (_lastNode)
      {
        return _lastNode;
      }

      // Fallback to the 'sourceGraphic' input.
      return ShareRefPtr(SourceGraphicNode());
    }
  };
}
