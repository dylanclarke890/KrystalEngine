#include "Krystal.Web/Dom/Nodes/CharacterData.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Web/Dom/Mixins/ChildNode.hpp"
#include "Krystal.Web/Dom/Mixins/NonDocumentTypeChildNode.hpp"
#include "Krystal.Web/Dom/MutationObserver/MutationObserverAlgorithms.hpp"
#include "Krystal.Web/Dom/Nodes/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Web/Dom/Ranges/LiveRangeUpdater.hpp"
#include "Krystal.Web/Dom/Trees/ExtensibilityHooks.hpp"
#include "Krystal.Web/Html/Algorithms/HTMLInsertionAlgorithms.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Web/Html/Sanitizer/SetHTMLOptions.hpp"
#include <algorithm>

namespace krys::web
{
  CharacterData::CharacterData(Document &document, DOMString data, dom::NodeType type,
                               dom::NodeFlags flags) noexcept
      : Node(document, type, flags | dom::NodeFlag::IsCharacterData), _data {krys::move(data)}
  {
  }

#pragma region CharacterData

  ExceptionOr<void> CharacterData::Data(DOMString data) noexcept
  {
    return dom::TextAlgorithms::Replace(*this, 0uz, Length(), krys::move(data));
  }

  ExceptionOr<DOMString> CharacterData::SubstringData(size_t offset, size_t count) const noexcept
  {
    return dom::TextAlgorithms::Substring(*this, offset, count);
  }

  ExceptionOr<void> CharacterData::AppendData(DOMString data) noexcept
  {
    return dom::TextAlgorithms::Replace(*this, _data.size(), 0uz, krys::move(data));
  }

  ExceptionOr<void> CharacterData::InsertData(size_t offset, DOMString data) noexcept
  {
    return dom::TextAlgorithms::Replace(*this, offset, 0uz, krys::move(data));
  }

  ExceptionOr<void> CharacterData::DeleteData(size_t offset, size_t count) noexcept
  {
    return dom::TextAlgorithms::Replace(*this, offset, count, u8"");
  }

  ExceptionOr<void> CharacterData::ReplaceData(size_t offset, size_t count, DOMString data) noexcept
  {
    return dom::TextAlgorithms::Replace(*this, offset, count, krys::move(data));
  }

#pragma endregion

#pragma region ChildNode

  ExceptionOr<void> CharacterData::Before(const List<dom::NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ChildNode::Before(*this, nodes);
  }

  ExceptionOr<void> CharacterData::After(const List<dom::NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ChildNode::After(*this, nodes);
  }

  ExceptionOr<void> CharacterData::ReplaceWith(const List<dom::NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ChildNode::ReplaceWith(*this, nodes);
  }

  ExceptionOr<void> CharacterData::Remove() noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ChildNode::Remove(*this);
  }

#pragma endregion

#pragma region NonDocumentTypeChildNode

  RefPtr<const Element> CharacterData::PreviousElementSibling() const noexcept
  {
    return dom::mixins::NonDocumentTypeChildNode::PreviousElementSibling(*this);
  }

  RefPtr<Element> CharacterData::PreviousElementSibling() noexcept
  {
    return dom::mixins::NonDocumentTypeChildNode::PreviousElementSibling(*this);
  }

  RefPtr<const Element> CharacterData::NextElementSibling() const noexcept
  {
    return dom::mixins::NonDocumentTypeChildNode::NextElementSibling(*this);
  }

  RefPtr<Element> CharacterData::NextElementSibling() noexcept
  {
    return dom::mixins::NonDocumentTypeChildNode::NextElementSibling(*this);
  }

#pragma endregion

#pragma region NonDocumentTypeChildNode (HTML extensions)

  ExceptionOr<void> CharacterData::BeforeHTML(DOMStringView html,
                                              const html::SetHTMLOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    return html::HTMLInsertionAlgorithms::BeforeHTML(*this, html, &options.Sanitizer,
                                                     html::FragmentParserMode::Safe);
  }

  ExceptionOr<void> CharacterData::BeforeHTML(DOMStringView html) noexcept
  {
    return BeforeHTML(html, {});
  }

  ExceptionOr<void> CharacterData::AfterHTML(DOMStringView html, const html::SetHTMLOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    return html::HTMLInsertionAlgorithms::AfterHTML(*this, html, &options.Sanitizer,
                                                    html::FragmentParserMode::Safe);
  }

  ExceptionOr<void> CharacterData::AfterHTML(DOMStringView html) noexcept
  {
    return AfterHTML(html, {});
  }

  ExceptionOr<void> CharacterData::ReplaceWithHTML(DOMStringView html,
                                                   const html::SetHTMLOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    return html::HTMLInsertionAlgorithms::ReplaceWithHTML(*this, html, &options.Sanitizer,
                                                          html::FragmentParserMode::Safe);
  }

  ExceptionOr<void> CharacterData::ReplaceWithHTML(DOMStringView html) noexcept
  {
    return ReplaceWithHTML(html, {});
  }

  ExceptionOr<void> CharacterData::BeforeHTMLUnsafe(DOMStringView html,
                                                    const html::SetHTMLUnsafeOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    return html::HTMLInsertionAlgorithms::BeforeHTML(
      *this, html, options.Sanitizer ? &*options.Sanitizer : nullptr, html::FragmentParserMode::Unsafe);
  }

  ExceptionOr<void> CharacterData::BeforeHTMLUnsafe(DOMStringView html) noexcept
  {
    return BeforeHTMLUnsafe(html, {});
  }

  ExceptionOr<void> CharacterData::AfterHTMLUnsafe(DOMStringView html,
                                                   const html::SetHTMLUnsafeOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    return html::HTMLInsertionAlgorithms::AfterHTML(
      *this, html, options.Sanitizer ? &*options.Sanitizer : nullptr, html::FragmentParserMode::Unsafe);
  }

  ExceptionOr<void> CharacterData::AfterHTMLUnsafe(DOMStringView html) noexcept
  {
    return AfterHTMLUnsafe(html, {});
  }

  ExceptionOr<void> CharacterData::ReplaceWithHTMLUnsafe(DOMStringView html,
                                                         const html::SetHTMLUnsafeOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    return html::HTMLInsertionAlgorithms::ReplaceWithHTML(
      *this, html, options.Sanitizer ? &*options.Sanitizer : nullptr, html::FragmentParserMode::Unsafe);
  }

  ExceptionOr<void> CharacterData::ReplaceWithHTMLUnsafe(DOMStringView html) noexcept
  {
    return ReplaceWithHTMLUnsafe(html, {});
  }

#pragma endregion

  bool CharacterData::ContainsOnlyASCIIWhitespace() const noexcept
  {
    return std::ranges::all_of(_data, [](char8_t c) { return text::IsASCIIWhitespace(c); });
  }
}
