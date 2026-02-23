#pragma once

#include "Krystal.HTML/Element/Element.hpp"
#include "Krystal.HTML/NodeList/NodeList.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class StaticNodeList final : public NodeList
  {
  private:
    List<Ref<Node>> _nodes;

  public:
    StaticNodeList(List<Ref<Node>> &&nodes = {}) noexcept;

    KRYS_NODISCARD size_t Length() const noexcept override;

    KRYS_NODISCARD RawPtr<Node> Item(size_t index) const noexcept override;
  };

  class StaticElementList final : public NodeList
  {
  private:
    List<Ref<Element>> _elements;

  public:
    StaticElementList(List<Ref<Element>> &&elements = {}) noexcept;

    KRYS_NODISCARD size_t Length() const noexcept override;

    KRYS_NODISCARD RawPtr<Element> Item(size_t index) const noexcept override;
  };
}