#include "Krystal.Booey/Rendering/ReferencedSVGResources.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/DOM/Mixins/NonElementParentNode.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceClipper.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGResourceElementClient.hpp"
#include "Krystal.Core/Types/HashSet.hpp"

namespace krys::boo
{
  class CSSSVGResourceElementClient final : public svg::SVGResourceElementClient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSSVGResourceElementClient);

  private:
    RenderElement &_clientRenderer;

  public:
    explicit CSSSVGResourceElementClient(RenderElement &clientRenderer) noexcept
        : _clientRenderer(clientRenderer)
    {
    }

    void ResourceChanged(svg::SVGElement &) noexcept final;

    KRYS_NODISCARD const RenderElement &Renderer() const noexcept final
    {
      return _clientRenderer;
    }
  };

  void CSSSVGResourceElementClient::ResourceChanged(svg::SVGElement &) noexcept
  {
    if (_clientRenderer.RenderTreeBeingDestroyed())
    {
      return;
    }

    _clientRenderer.Repaint();
  }

  ReferencedSVGResources::ReferencedSVGResources(RenderElement &renderer) noexcept : _renderer(renderer)
  {
  }

  ReferencedSVGResources::~ReferencedSVGResources() noexcept
  {
    auto &treeScope = _renderer.TreeScopeForSVGReferences();
    List<dom::DOMStringAtom> targetIDs;
    for (const auto &[targetID, client] : _elementClients)
    {
      targetIDs.push_back(targetID);
    }
    for (const auto &targetID : targetIDs)
    {
      RemoveClientForTarget(treeScope, targetID);
    }
  }

  void ReferencedSVGResources::AddClientForTarget(svg::SVGElement &targetElement,
                                                  const dom::DOMStringAtom &targetID) noexcept
  {
    auto &client = _elementClients[targetID];
    if (!client)
    {
      client = CreateUnique<CSSSVGResourceElementClient>(_renderer);
      targetElement.AddReferencingCSSClient(*client);
    }
  }

  void ReferencedSVGResources::RemoveClientForTarget(dom::DocumentOrShadowRoot &treeScope,
                                                     const dom::DOMStringAtom &targetID) noexcept
  {
    auto it = _elementClients.find(targetID);
    if (it == _elementClients.end())
    {
      return;
    }
    auto client = krys::move(it->second);
    _elementClients.erase(it);

    auto targetElement =
      dom::mixins::NonElementParentNode::GetElementById(treeScope.RootNode(), targetID.View());
    if (auto *svgElement = DynamicDowncast<svg::SVGElement>(targetElement.get()))
    {
      svgElement->RemoveReferencingCSSClient(*client);
    }
  }

  ReferencedSVGResources::SVGElementIdentifierAndTagPairs
    ReferencedSVGResources::ReferencedSVGResourceIDs(const RenderStyle &style, const dom::Document &) noexcept
  {
    SVGElementIdentifierAndTagPairs referencedResources;
    if (auto clipPath = style.GetClipPath().TryReference())
    {
      if (!clipPath->Fragment().View().empty())
      {
        referencedResources.push_back({clipPath->Fragment(), {html::TagName::clipPath}});
      }
    }

    // FIXME: The reference filters, with filters (B11h).

    return referencedResources;
  }

  void ReferencedSVGResources::UpdateReferencedResources(
    dom::DocumentOrShadowRoot &treeScope, const SVGElementIdentifierAndTagPairs &referencedResources) noexcept
  {
    HashSet<dom::DOMStringAtom> oldKeys;
    for (const auto &[key, client] : _elementClients)
    {
      oldKeys.insert(key);
    }

    for (const auto &[targetID, tagNames] : referencedResources)
    {
      auto element = ElementForResourceIDs(treeScope, targetID, tagNames);
      if (!element)
      {
        continue;
      }

      AddClientForTarget(*element, targetID);
      oldKeys.erase(targetID);
    }

    for (const auto &targetID : oldKeys)
    {
      RemoveClientForTarget(treeScope, targetID);
    }
  }

  // SVG code uses getRenderSVGResourceById<>, but that works in terms of renderers. We need to find resources
  // before the render tree is fully constructed, so this works on Elements.
  RefPtr<svg::SVGElement> ReferencedSVGResources::ElementForResourceID(dom::DocumentOrShadowRoot &treeScope,
                                                                       const dom::DOMStringAtom &resourceID,
                                                                       html::TagName tagName) noexcept
  {
    auto element = dom::mixins::NonElementParentNode::GetElementById(treeScope.RootNode(), resourceID.View());
    auto *svgElement = DynamicDowncast<svg::SVGElement>(element.get());
    if (!svgElement || svgElement->TagQualifiedName().TagName() != tagName)
    {
      return nullptr;
    }

    return ShareRefPtr(svgElement);
  }

  RefPtr<svg::SVGElement>
    ReferencedSVGResources::ElementForResourceIDs(dom::DocumentOrShadowRoot &treeScope,
                                                  const dom::DOMStringAtom &resourceID,
                                                  const SVGQualifiedNames &tagNames) noexcept
  {
    auto element = dom::mixins::NonElementParentNode::GetElementById(treeScope.RootNode(), resourceID.View());
    auto *svgElement = DynamicDowncast<svg::SVGElement>(element.get());
    if (!svgElement)
    {
      return nullptr;
    }

    for (auto tagName : tagNames)
    {
      if (svgElement->TagQualifiedName().TagName() == tagName)
      {
        return ShareRefPtr(svgElement);
      }
    }

    return nullptr;
  }

  LegacyRenderSVGResourceClipper *
    ReferencedSVGResources::ReferencedClipperRenderer(dom::DocumentOrShadowRoot &treeScope,
                                                      const style::ReferencePath &clipPath) noexcept
  {
    if (clipPath.Fragment().View().empty())
    {
      return nullptr;
    }
    // For some reason, SVG stores a cache of id -> renderer, rather than just using getElementById() and
    // renderer().
    return GetRenderSVGResourceById<LegacyRenderSVGResourceClipper>(treeScope, clipPath.Fragment());
  }

  LegacyRenderSVGResourceContainer *
    ReferencedSVGResources::ReferencedRenderResource(dom::DocumentOrShadowRoot &treeScope,
                                                     const dom::DOMStringAtom &fragment) noexcept
  {
    if (fragment.View().empty())
    {
      return nullptr;
    }
    return GetRenderSVGResourceContainerById(treeScope, fragment);
  }
}
