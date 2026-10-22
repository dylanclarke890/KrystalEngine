#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Editing/HTMLInterchange.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Document;
  class DocumentFragment;
  class Element;
  class Node;
  class ShadowRoot;
  struct SimpleRange;
  enum class TreeType : uint8;
}

namespace krys::boo
{
  class VisibleSelection;

  enum class ResolveURLs : uint8
  {
    No,
    NoExcludingURLsForPrivacy,
    Yes,
    YesExcludingURLsForPrivacy
  };

  /// @brief Whether nodes, and everything under them, are `user-select: none`, remembered as nodes are asked
  /// about.
  class UserSelectNoneStateCache
  {
  private:
    enum class State : uint8
    {
      NotUserSelectNone,
      Mixed,
      OnlyUserSelectNone
    };

    HashMap<const dom::Node *, State> _cache;
    bool _useComposedTree;

  public:
    explicit UserSelectNoneStateCache(dom::TreeType treeType) noexcept;

    KRYS_NODISCARD bool NodeOnlyContainsUserSelectNone(dom::Node &node) noexcept
    {
      return ComputeState(node) == State::OnlyUserSelectNone;
    }

  private:
    KRYS_NODISCARD dom::ContainerNode *ParentNode(dom::Node &node) noexcept;
    KRYS_NODISCARD dom::Node *FirstChild(dom::Node &node) noexcept;
    KRYS_NODISCARD dom::Node *NextSibling(dom::Node &node) noexcept;

    KRYS_NODISCARD State ComputeState(dom::Node &targetNode) noexcept;
  };

  /// @brief The text, as a fragment of `p`s (or clones of the context's block), `br`s, text nodes and tab
  /// spans, that it would be pasted as at @p context.
  KRYS_NODISCARD Ref<dom::DocumentFragment> CreateFragmentFromText(const dom::SimpleRange &context,
                                                                   utf8_stringview text) noexcept;
  /// @brief The markup, parsed as if it were in a `body`.
  KRYS_NODISCARD Ref<dom::DocumentFragment> CreateFragmentFromMarkup(dom::Document &document,
                                                                     utf8_stringview markup,
                                                                     utf8_stringview baseURL) noexcept;
  /// @see https://w3c.github.io/DOM-Parsing/#dom-range-createcontextualfragment
  KRYS_NODISCARD dom::ExceptionOr<Ref<dom::DocumentFragment>>
    CreateContextualFragment(dom::Element &element, utf8_stringview markup) noexcept;

  KRYS_NODISCARD bool IsPlainTextMarkup(dom::Node *node) noexcept;

  // These methods are used by HTMLElement & ShadowRoot to replace the children with respected fragment/text.
  dom::ExceptionOr<void> ReplaceChildrenWithFragment(dom::ContainerNode &container,
                                                     Ref<dom::DocumentFragment> &&fragment) noexcept;

  enum class ConvertBlocksToInlines : bool
  {
    No,
    Yes
  };

  enum class SerializeComposedTree : bool
  {
    No,
    Yes
  };

  enum class IgnoreUserSelectNone : bool
  {
    No,
    Yes
  };

  enum class PreserveBaseElement : bool
  {
    No,
    Yes
  };

  enum class PreserveDirectionForInlineText : bool
  {
    No,
    Yes
  };

  /// @brief The range's markup, with the styles that make it look as it does inlined, so it looks the same
  /// wherever it's pasted.
  KRYS_NODISCARD utf8_string SerializePreservingVisualAppearance(
    const dom::SimpleRange &range, List<Ref<dom::Node>> *nodes = nullptr,
    AnnotateForInterchange annotate = AnnotateForInterchange::No,
    ConvertBlocksToInlines convertBlocksToInlines = ConvertBlocksToInlines::No,
    ResolveURLs resolveURLs = ResolveURLs::No) noexcept;
  KRYS_NODISCARD utf8_string SerializePreservingVisualAppearance(
    const VisibleSelection &selection, ResolveURLs resolveURLs = ResolveURLs::No,
    SerializeComposedTree serializeComposedTree = SerializeComposedTree::No,
    IgnoreUserSelectNone ignoreUserSelectNone = IgnoreUserSelectNone::Yes,
    PreserveBaseElement preserveBaseElement = PreserveBaseElement::No,
    PreserveDirectionForInlineText preserveDirectionForInlineText = PreserveDirectionForInlineText::No,
    List<Ref<dom::Node>> *nodes = nullptr) noexcept;

  enum class SerializedNodes : uint8
  {
    SubtreeIncludingNode,
    SubtreesOfChildren
  };

  enum class SerializationSyntax : uint8
  {
    HTML,
    XML,
    HTMLLegacyAttributeValue
  };

  enum class SerializeShadowRoots : uint8
  {
    Explicit,
    Serializable,
    AllForInterchange
  };

  /// @see https://html.spec.whatwg.org/#serialising-html-fragments
  KRYS_NODISCARD utf8_string SerializeFragment(
    const dom::Node &node, SerializedNodes root, List<Ref<dom::Node>> *nodes = nullptr,
    ResolveURLs resolveURLs = ResolveURLs::No, Maybe<SerializationSyntax> serializationSyntax = null,
    SerializeShadowRoots serializeShadowRoots = SerializeShadowRoots::Explicit,
    List<Ref<dom::ShadowRoot>> &&explicitShadowRoots = {}) noexcept;

  KRYS_NODISCARD utf8_string URLToMarkup(utf8_stringview url, utf8_stringview title) noexcept;

  KRYS_NODISCARD utf8_string DocumentTypeString(const dom::Document &document) noexcept;
}
