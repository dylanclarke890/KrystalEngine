#pragma once

#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/NodeList/NodeList.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
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

    KRYS_NODISCARD RawPtr<Node> Item(size_t index) const noexcept override;

    KRYS_NODISCARD size_t Length() const noexcept override;
  };

  class StaticElementList final : public NodeList
  {
  private:
    List<Ref<Element>> _elements;

  public:
    StaticElementList(List<Ref<Element>> &&elements = {}) noexcept;

    KRYS_NODISCARD RawPtr<Element> Item(size_t index) const noexcept override;

    KRYS_NODISCARD size_t Length() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::StaticNodeList)
  static bool IsType(const Krys::HTML::NodeList &target) noexcept
  {
    return target.IsStaticNodeList();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::StaticElementList)
  static bool IsType(const Krys::HTML::NodeList &target) noexcept
  {
    return target.IsStaticElementList();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();