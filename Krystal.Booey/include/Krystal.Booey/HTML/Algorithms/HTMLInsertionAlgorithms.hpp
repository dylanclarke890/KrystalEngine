#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/Dicts/SetHTMLOptions.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class DocumentFragment;
  class Element;
  class Node;
}

namespace krys::boo::html
{
  /// @brief How the HTML insertion methods parse.
  /// @note The specification's Legacy mode is `innerHTML`'s, `outerHTML`'s and `insertAdjacentHTML()`'s,
  /// which parse through `CreateFragmentForInnerOuterHTML`.
  /// @see https://html.spec.whatwg.org/#fragment-parser-mode
  enum class FragmentParserMode : uint8
  {
    /// @brief Unsafe elements and attributes are removed, and declarative shadow roots are allowed.
    Safe,
    /// @brief Unsafe elements and attributes are kept unless a sanitizer is given, and declarative shadow
    /// roots are allowed.
    Unsafe,
  };

  /// @brief The algorithms of the HTML insertion methods (`setHTML()`, `appendHTML()`, `prependHTML()`,
  /// `beforeHTML()`, `afterHTML()`, `replaceWithHTML()` and their Unsafe counterparts).
  /// @see https://html.spec.whatwg.org/#html-parsing-and-insertion-methods
  class HTMLInsertionAlgorithms
  {
  public:
    /// @brief @p element, or a template's template contents.
    /// @see https://html.spec.whatwg.org/#target-for-html-insertion
    KRYS_NODISCARD static dom::ContainerNode &TargetForHTMLInsertion(dom::Element &element) noexcept;

    /// @brief @p node's parent, null if it has none, or a `HierarchyRequestError` if it can't give the parser
    /// a context (it's a template, a document, or a fragment without a host).
    /// @see https://html.spec.whatwg.org/#parent-for-html-insertion
    KRYS_NODISCARD static dom::ExceptionOr<dom::ContainerNode *>
      ParentForHTMLInsertion(dom::Node &node) noexcept;

    /// @brief Replaces @p target's children with @p html parsed in its context.
    /// @param sanitizer The options' sanitizer member, or null if it's missing.
    /// @see https://html.spec.whatwg.org/#filter-and-set-html
    static dom::ExceptionOr<void> FilterAndSetHTML(dom::ContainerNode &target, dom::DOMStringView html,
                                                   const SanitizerInput *sanitizer,
                                                   FragmentParserMode mode) noexcept;

    /// @brief Inserts @p html, parsed in @p parent's context, into @p parent before @p child.
    /// @see https://html.spec.whatwg.org/#filter-and-pre-insert-html
    static dom::ExceptionOr<void> FilterAndPreInsertHTML(dom::ContainerNode &parent, dom::Node *child,
                                                         dom::DOMStringView html,
                                                         const SanitizerInput *sanitizer,
                                                         FragmentParserMode mode) noexcept;

    /// @brief Replaces @p node with @p html, parsed in @p parent's context.
    /// @see https://html.spec.whatwg.org/#filter-and-replace-with-html
    static dom::ExceptionOr<void> FilterAndReplaceWithHTML(dom::ContainerNode &parent, dom::Node &node,
                                                           dom::DOMStringView html,
                                                           const SanitizerInput *sanitizer,
                                                           FragmentParserMode mode) noexcept;

    /// @brief @p html parsed as @p target's children; null when Safe parsing into a script element.
    /// @see https://html.spec.whatwg.org/#parse-html-for-insertion
    KRYS_NODISCARD static dom::ExceptionOr<RefPtr<dom::DocumentFragment>>
      ParseHTMLForInsertion(dom::ContainerNode &target, dom::DOMStringView html,
                            const SanitizerInput *sanitizer, FragmentParserMode mode) noexcept;

    /// @brief The steps of `beforeHTML()` and `beforeHTMLUnsafe()`, shared by elements and character data.
    /// @see https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-beforehtml
    static dom::ExceptionOr<void> BeforeHTML(dom::Node &node, dom::DOMStringView html,
                                             const SanitizerInput *sanitizer,
                                             FragmentParserMode mode) noexcept;

    /// @brief The steps of `afterHTML()` and `afterHTMLUnsafe()`.
    /// @see https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-afterhtml
    static dom::ExceptionOr<void> AfterHTML(dom::Node &node, dom::DOMStringView html,
                                            const SanitizerInput *sanitizer,
                                            FragmentParserMode mode) noexcept;

    /// @brief The steps of `replaceWithHTML()` and `replaceWithHTMLUnsafe()`.
    /// @see https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-replacewithhtml
    static dom::ExceptionOr<void> ReplaceWithHTML(dom::Node &node, dom::DOMStringView html,
                                                  const SanitizerInput *sanitizer,
                                                  FragmentParserMode mode) noexcept;
  };
}
