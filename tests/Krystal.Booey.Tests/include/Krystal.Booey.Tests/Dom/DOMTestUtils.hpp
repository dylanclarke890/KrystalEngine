#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Dom/Events/EventListener.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/Dom/Traversal/NodeFilter.hpp"
#include "Krystal.Booey/Editing/Position.hpp"
#include "Krystal.Booey/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Html/Parser/DocumentParser.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo::tests
{
  /// @brief The element of @p document with the id @p id, which must exist.
  KRYS_NODISCARD inline Element &ById(Document &document, DOMStringView id)
  {
    auto element = document.GetElementById(id);
    REQUIRE(element);
    return *element;
  }

  /// @brief A document parsed from @p markup.
  KRYS_NODISCARD inline Ref<HTMLDocument> ParseDocument(utf8_string markup) noexcept
  {
    auto document = CreateRef<HTMLDocument>();
    auto parser = CreateRef<html::DocumentParser>(*document);
    parser->InputStream().Append(krys::move(markup), IsEOF::Yes);
    parser->PumpTokenizer();
    return document;
  }

  /// @brief A new element of @p document named @p localName.
  KRYS_NODISCARD inline Ref<Element> CreateElement(Document &document, DOMStringAtom localName)
  {
    auto element = document.CreateElement(krys::move(localName));
    REQUIRE_FALSE(element.HasException());
    return element.ReleaseValue();
  }

  /// @brief Runs the event loop's queued tasks.
  inline void RunTasks() noexcept
  {
    html::EventLoop::Current().RunUntilIdle();
  }

  /// @brief The position @p offset into @p node.
  KRYS_NODISCARD inline Position At(Node &node, uint32 offset) noexcept
  {
    return CreateContainerOffsetPosition(ShareRefPtr(&node), offset);
  }

  /// @brief An event listener that calls a function.
  struct FunctionListener final : EventListenerCallback
  {
    Func<void(Event &)> Function;

    explicit FunctionListener(Func<void(Event &)> function) noexcept : Function(krys::move(function))
    {
    }

    void HandleEvent(Event &event) noexcept override
    {
      Function(event);
    }
  };

  /// @brief A node filter that accepts text nodes only.
  class TextNodeFilter : public NodeFilter
  {
  public:
    KRYS_NODISCARD ExceptionOr<FilterResult> AcceptNode(const Node &node) const noexcept override
    {
      return node.NodeType() == dom::NodeType::TEXT_NODE ? FilterResult::FILTER_ACCEPT
                                                         : FilterResult::FILTER_REJECT;
    }
  };
}
