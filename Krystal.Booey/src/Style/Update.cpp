#include "Krystal.Booey/Style/Update.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeAncestorIterator.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"

namespace krys::boo::style
{
  Update::Update(dom::Document &document) noexcept : _document(ShareRef(document))
  {
  }

  Update::~Update() noexcept = default;

  const ElementUpdate *Update::GetElementUpdate(const dom::Element &element) const noexcept
  {
    auto it = _elements.find(ShareRefPtr(&element));
    if (it == _elements.end())
    {
      return nullptr;
    }
    return &it->second;
  }

  ElementUpdate *Update::GetElementUpdate(const dom::Element &element) noexcept
  {
    auto it = _elements.find(ShareRefPtr(&element));
    if (it == _elements.end())
    {
      return nullptr;
    }
    return &it->second;
  }

  const TextUpdate *Update::GetTextUpdate(const dom::Text &text) const noexcept
  {
    auto it = _texts.find(ShareRefPtr(&text));
    if (it == _texts.end())
    {
      return nullptr;
    }
    return &it->second;
  }

  const RenderStyle *Update::ElementStyle(const dom::Element &element) const noexcept
  {
    if (auto *update = GetElementUpdate(element))
    {
      return update->Style.get();
    }
    return element.GetRenderStyle();
  }

  RenderStyle *Update::ElementStyle(const dom::Element &element) noexcept
  {
    if (auto *update = GetElementUpdate(element))
    {
      return update->Style.get();
    }
    // FIXME: The renderer's mutable style; there's no render tree, so it's the style kept on the element.
    return const_cast<RenderStyle *>(element.GetRenderStyle());
  }

  void Update::AddElement(dom::Element &element, dom::Element *parent, ElementUpdate &&elementUpdate) noexcept
  {
    krys_debug_assert(dom::ComposedTreeAncestors(element).First() == parent);
    krys_debug_assert(!_elements.contains(ShareRefPtr<const dom::Element>(&element)));

    _roots.remove(ShareRefPtr<dom::ContainerNode>(&element));
    AddPossibleRoot(parent);

    if (elementUpdate.MayNeedRebuildRoot)
    {
      AddPossibleRebuildRoot(element, parent);
    }

    _elements.emplace(ShareRefPtr<const dom::Element>(&element), krys::move(elementUpdate));
  }

  void Update::AddText(dom::Text &text, dom::Element *parent, TextUpdate &&textUpdate) noexcept
  {
    krys_debug_assert(dom::ComposedTreeAncestors(text).First() == parent);

    AddPossibleRoot(parent);

    auto [it, isNewEntry] = _texts.try_emplace(ShareRefPtr<const dom::Text>(&text), krys::move(textUpdate));

    if (!isNewEntry)
    {
      auto &entry = it->second;
      auto startOffset = std::min(entry.Offset, textUpdate.Offset);
      auto endOffset = std::max(entry.Offset + entry.Length, textUpdate.Offset + textUpdate.Length);
      entry.Offset = startOffset;
      entry.Length = endOffset - startOffset;

      krys_debug_assert(!entry.InheritedDisplayContentsStyle || !textUpdate.InheritedDisplayContentsStyle);
      if (!entry.InheritedDisplayContentsStyle)
      {
        entry.InheritedDisplayContentsStyle = krys::move(textUpdate.InheritedDisplayContentsStyle);
      }
    }
  }

  void Update::AddText(dom::Text &text, TextUpdate &&textUpdate) noexcept
  {
    AddText(text, dom::ComposedTreeAncestors(text).First(), krys::move(textUpdate));
  }

  void Update::AddInitialContainingBlockUpdate(UniquePtr<RenderStyle> style) noexcept
  {
    _initialContainingBlockUpdate = krys::move(style);
  }

  void Update::AddSVGRendererUpdate(svg::SVGElement &element) noexcept
  {
    auto *parent = dom::ComposedTreeAncestors(element).First();
    _roots.remove(ShareRefPtr<dom::ContainerNode>(&element));
    AddPossibleRoot(parent);
    element.SetNeedsSVGRendererUpdate(true);
  }

  void Update::AddPossibleRoot(dom::Element *element) noexcept
  {
    if (!element)
    {
      _roots.add(ShareRefPtr<dom::ContainerNode>(_document.get()));
      return;
    }
    if (element->NeedsSVGRendererUpdate() || _elements.contains(ShareRefPtr<const dom::Element>(element)))
    {
      return;
    }
    _roots.add(ShareRefPtr<dom::ContainerNode>(element));
  }

  void Update::AddPossibleRebuildRoot(dom::Element &element, dom::Element *parent) noexcept
  {
    if (parent && _rebuildRoots.contains(ShareRefPtr(parent)))
    {
      return;
    }

    _rebuildRoots.add(ShareRefPtr(&element));
  }
}
