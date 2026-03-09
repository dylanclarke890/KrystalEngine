#include "Krystal.HTML/NodeList/StaticNodeList.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.Lib/Core/Move.hpp"

namespace Krys::HTML
{
  StaticNodeList::StaticNodeList(List<Ref<Node>> &&nodes) noexcept
      : NodeList(NodeListType::Static, NodeListFlag::ContainsOnlyElements), _nodes(Krys::Move(nodes))
  {
  }

  RawPtr<Node> StaticNodeList::Item(size_t index) const noexcept
  {
    if (index < _nodes.size())
    {
      return _nodes.at(index).get();
    }

    return nullptr;
  }

  size_t StaticNodeList::Length() const noexcept
  {
    return _nodes.size();
  }

  StaticElementList::StaticElementList(List<Ref<Element>> &&elements) noexcept
      : NodeList(NodeListType::Static, NodeListFlag::ContainsOnlyElements), _elements(Krys::Move(elements))
  {
  }

  RawPtr<Element> StaticElementList::Item(size_t index) const noexcept
  {
    if (index < _elements.size())
    {
      return _elements.at(index).get();
    }

    return nullptr;
  }

  size_t StaticElementList::Length() const noexcept
  {
    return _elements.size();
  }
}
