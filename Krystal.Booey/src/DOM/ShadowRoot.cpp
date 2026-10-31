#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/StyleSheetList.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/Editing/Markup.hpp"
#include "Krystal.Booey/HTML/Algorithms/HTMLInsertionAlgorithms.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::dom
{
  ShadowRoot::ShadowRoot(Document &document, RefPtr<html::CustomElementRegistry> &&registry,
                         NodeFlags flags) noexcept
      : DocumentFragment(document, flags | NodeFlags::IsShadowRoot),
        DocumentOrShadowRoot(*this, krys::move(registry))
  {
    SetNodeFlag(NodeFlags::IsInShadowTree);
    _styleScope = CreateUnique<style::Scope>(*this);
  }

  ShadowRoot::~ShadowRoot() noexcept
  {
    if (_styleSheetList)
    {
      _styleSheetList->Detach();
    }
  }

#pragma region DocumentOrShadowRoot Mixin (CSSOM)

  css::StyleSheetList &ShadowRoot::StyleSheets() noexcept
  {
    if (!_styleSheetList)
    {
      _styleSheetList = css::StyleSheetList::Create(*this);
    }
    return *_styleSheetList;
  }

  List<RefPtr<WebAnimation>> ShadowRoot::GetAnimations() noexcept
  {
    return NodeDocument().MatchingAnimations([&](Element &target) -> bool
                                             { return TreeQueries::ContainingShadowRoot(target) == this; });
  }

#pragma endregion

  Element *ShadowRoot::Host() const noexcept
  {
    if (auto host = _host.lock())
    {
      return host.get();
    }

    return nullptr;
  }

#pragma region ShadowRoot (DOM Parsing)

  ExceptionOr<void> ShadowRoot::SetHTML(DOMStringView html, html::SetHTMLOptions &&options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // SPEC: Filter and set HTML given this, html, options, and Safe.
    return html::HTMLInsertionAlgorithms::FilterAndSetHTML(*this, html, &options.Sanitizer,
                                                           html::FragmentParserMode::Safe);
  }

  ExceptionOr<void> ShadowRoot::AppendHTML(DOMStringView html, html::SetHTMLOptions &&options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // SPEC: Filter and pre-insert HTML given this, null, html, options, and Safe.
    return html::HTMLInsertionAlgorithms::FilterAndPreInsertHTML(*this, nullptr, html, &options.Sanitizer,
                                                                 html::FragmentParserMode::Safe);
  }

  ExceptionOr<void> ShadowRoot::PrependHTML(DOMStringView html, html::SetHTMLOptions &&options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // SPEC: Filter and pre-insert HTML given this, this's first child, html, options, and Safe.
    return html::HTMLInsertionAlgorithms::FilterAndPreInsertHTML(
      *this, FirstChild(), html, &options.Sanitizer, html::FragmentParserMode::Safe);
  }

  ExceptionOr<void> ShadowRoot::SetHTMLUnsafe(DOMStringView html,
                                              html::SetHTMLUnsafeOptions &&options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // SPEC: Filter and set HTML given this, html, options, and Unsafe.
    return html::HTMLInsertionAlgorithms::FilterAndSetHTML(
      *this, html, options.Sanitizer ? &*options.Sanitizer : nullptr, html::FragmentParserMode::Unsafe);
  }

  ExceptionOr<void> ShadowRoot::AppendHTMLUnsafe(DOMStringView html,
                                                 html::SetHTMLUnsafeOptions &&options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // SPEC: Filter and pre-insert HTML given this, null, html, options, and Unsafe.
    return html::HTMLInsertionAlgorithms::FilterAndPreInsertHTML(
      *this, nullptr, html, options.Sanitizer ? &*options.Sanitizer : nullptr,
      html::FragmentParserMode::Unsafe);
  }

  ExceptionOr<void> ShadowRoot::PrependHTMLUnsafe(DOMStringView html,
                                                  html::SetHTMLUnsafeOptions &&options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // SPEC: Filter and pre-insert HTML given this, this's first child, html, options, and Unsafe.
    return html::HTMLInsertionAlgorithms::FilterAndPreInsertHTML(
      *this, FirstChild(), html, options.Sanitizer ? &*options.Sanitizer : nullptr,
      html::FragmentParserMode::Unsafe);
  }

  DOMString ShadowRoot::GetHTML(GetHTMLOptions &&options) const noexcept
  {
    return SerializeFragment(*this, SerializedNodes::SubtreesOfChildren, nullptr,
                             ResolveURLs::NoExcludingURLsForPrivacy, SerializationSyntax::HTML,
                             options.SerializableShadowRoots ? SerializeShadowRoots::Serializable
                                                             : SerializeShadowRoots::Explicit,
                             krys::move(options.ShadowRoots));
  }

  DOMString ShadowRoot::InnerHTML() const noexcept
  {
    return SerializeFragment(*this, SerializedNodes::SubtreesOfChildren, nullptr,
                             ResolveURLs::NoExcludingURLsForPrivacy);
  }

  ExceptionOr<void> ShadowRoot::InnerHTML(DOMStringView markup) noexcept
  {
    html::CEReactionsScope ceReactions;
    return ReplaceChildrenWithMarkup(markup, {});
  }

#pragma endregion

  ExceptionOr<void>
    ShadowRoot::ReplaceChildrenWithMarkup(DOMStringView markup,
                                          EnumSet<ParserContentPolicy> parserContentPolicy) noexcept
  {
    if (markup.empty())
    {
      return MutationAlgorithms::ReplaceAll(nullptr, *this);
    }

    RefPtr<Element> host = ShareRefPtr(Host());
    krys_assert(host);
    auto fragment = CreateFragmentForInnerOuterHTML(*host, markup, parserContentPolicy);
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }
    return ReplaceChildrenWithFragment(*this, fragment.ReleaseValue());
  }

#pragma region Extension Hooks

  void ShadowRoot::OnInsert() noexcept
  {
    DocumentFragment::OnInsert();
    if (IsConnected())
    {
      NodeDocument().DidInsertInDocumentShadowRoot(*this);
    }
    if (!GetAdoptedStyleSheets().empty())
    {
      StyleScope().DidChangeActiveStyleSheetCandidates();
    }
  }

  void ShadowRoot::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    DocumentFragment::OnRemove(isSubtreeRoot, oldAncestor);
    NodeDocument().DidRemoveInDocumentShadowRoot(*this);
  }

  void ShadowRoot::OnAdopt(Document &oldDocument) noexcept
  {
    DocumentFragment::OnAdopt(oldDocument);

    // Adopted sheets were constructed for the old document.
    (void)SetAdoptedStyleSheets({});

    // Style scopes are document specific.
    _styleScope = CreateUnique<style::Scope>(*this);
  }

#pragma endregion

  EventTarget *ShadowRoot::GetParent(Event &event) const noexcept
  {
    if (!event.Composed())
    {
      if (event._path.size() >= 1)
      {
        auto &firstItem = event._path[0];
        if (auto *invocationTargetNode = DynamicDowncast<Node>(firstItem.InvocationTarget()))
        {
          if (&TreeQueries::Root(*invocationTargetNode) == this)
          {
            return nullptr;
          }
        }
      }
    }

    if (auto host = _host.lock())
    {
      return host.get();
    }

    return nullptr;
  }

  List<Ref<ShadowRoot>> AssignedShadowRootsIfSlotted(const Node &node) noexcept
  {
    List<Ref<ShadowRoot>> result;
    for (auto *slot = SlotAlgorithms::GetAssignedSlot(node); slot;
         slot = SlotAlgorithms::GetAssignedSlot(*slot))
    {
      auto *shadowRoot = TreeQueries::ContainingShadowRoot(*slot);
      krys_debug_assert(shadowRoot);
      result.push_back(ShareRef(*shadowRoot));
    }
    return result;
  }
}
