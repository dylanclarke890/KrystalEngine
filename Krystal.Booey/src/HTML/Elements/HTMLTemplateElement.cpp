#include "Krystal.Booey/HTML/Elements/HTMLTemplateElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#attr-template-shadowrootmode
  template <>
  struct EnumeratedAttributeTraits<"shadowrootmode", HTMLTemplateElement>
  {
    using State = ShadowRootModeAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"open", State::Open},
      {u8"closed", State::Closed},
    };

    constexpr static State MissingValueDefault = State::None;
    constexpr static State InvalidValueDefault = State::None;
  };

  /// @see https://html.spec.whatwg.org/#attr-template-shadowrootslotassignment
  template <>
  struct EnumeratedAttributeTraits<"shadowrootslotassignment", HTMLTemplateElement>
  {
    using State = ShadowRootSlotAssignmentAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"named", State::Named},
      {u8"manual", State::Manual},
    };

    constexpr static State MissingValueDefault = State::Named;
    constexpr static State InvalidValueDefault = State::Named;
  };

  // FIXME: Implement the HTMLTemplateElement constructor in a spec compliant way.

  // https://html.spec.whatwg.org/#template-contents
  HTMLTemplateElement::HTMLTemplateElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Template),
        // 1. Let document be the template element's node document's appropriate template contents owner
        //    document.
        // (Skipped: there's no inert template contents owner document.)
        // 2. Create a DocumentFragment object whose node document is document and host is the template
        // element.
        // 3. Set the template element's template contents to the newly created DocumentFragment object.
        _content(CreateRef<dom::DocumentFragment>(document))
  {
    _content->SetHost(*this);
  }

#pragma region HTMLTemplateElement

  // https://html.spec.whatwg.org/#dom-template-content
  RefPtr<dom::DocumentFragment> HTMLTemplateElement::Content() const noexcept
  {
    // 1. Assert: this's template contents is not a ShadowRoot node.
    krys_assert(!Is<dom::ShadowRoot>(_content.get()));

    // 2. Return this's template contents.
    return _content;
  }

  dom::DOMString HTMLTemplateElement::HtmlFor() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"for").value_or(u8"");
  }

  void HTMLTemplateElement::HtmlFor(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"for", krys::move(value));
  }

  void HTMLTemplateElement::SetTemplateContents(dom::ShadowRoot &shadow) noexcept
  {
    _content = ShareRef<dom::DocumentFragment>(shadow);
  }

  namespace
  {
    // https://html.spec.whatwg.org/#find-markers
    KRYS_NODISCARD Pair<dom::ProcessingInstruction *, dom::ProcessingInstruction *>
      FindMarkers(dom::ContainerNode &scope, dom::DOMStringView name) noexcept
    {
      // 1. For each descendant of scope's descendants, in tree order:
      for (auto &descendant : dom::DescendantRange(scope))
      {
        // 1a. If descendant is not a ProcessingInstruction node, then continue.
        auto *instruction = DynamicDowncast<dom::ProcessingInstruction>(descendant);
        if (!instruction)
        {
          continue;
        }

        // 1b. If the result of getting a processing instruction attribute given descendant and "name" is not
        //     name, then continue.
        auto instructionName = instruction->GetAttribute(u8"name");
        if (!instructionName || *instructionName != name)
        {
          continue;
        }

        // 1c. If descendant's target is "marker", then return (descendant, descendant).
        if (instruction->Target() == u8"marker")
        {
          return {instruction, instruction};
        }

        // 1d. If descendant's target is "start":
        if (instruction->Target() == u8"start")
        {
          // 1d.i. Let start be descendant.
          // 1d.ii. Let nestingLevel be 0.
          uint32 nestingLevel = 0;

          // 1d.iii. Let sibling be start's next sibling.
          // 1d.iv. While sibling is not null: [...] 1d.iv.B. Set sibling to sibling's next sibling.
          for (auto *sibling = instruction->NextSibling(); sibling; sibling = sibling->NextSibling())
          {
            // 1d.iv.A. If sibling is a ProcessingInstruction node:
            auto *siblingInstruction = DynamicDowncast<dom::ProcessingInstruction>(*sibling);
            if (!siblingInstruction)
            {
              continue;
            }

            // 1d.iv.A.1. If sibling's target is "start", then increment nestingLevel by one.
            if (siblingInstruction->Target() == u8"start")
            {
              ++nestingLevel;
            }

            // 1d.iv.A.2. If sibling's target is "end":
            if (siblingInstruction->Target() == u8"end")
            {
              // 1d.iv.A.2a. If nestingLevel is 0, then return (start, sibling).
              if (nestingLevel == 0)
              {
                return {instruction, siblingInstruction};
              }

              // 1d.iv.A.2b. Decrement nestingLevel by one.
              --nestingLevel;
            }
          }

          // 1d.v. Return (start, null).
          return {instruction, nullptr};
        }
      }

      // 2. Return (null, null).
      return {nullptr, nullptr};
    }
  }

  void HTMLTemplateElement::ClearInsertionTarget() noexcept
  {
    _insertionTarget = nullptr;
    _insertionStartMarker = nullptr;
    _insertionEndMarker = nullptr;
  }

  // https://html.spec.whatwg.org/#prepare-content-patching
  bool HTMLTemplateElement::PrepareContentPatching(dom::ContainerNode &scope) noexcept
  {
    // 1. Assert: template's for attribute is specified.
    // 2. Let markerName be template's for attribute's value.
    auto markerName = GetAttribute(u8"for");
    krys_debug_assert(markerName);

    // 3. If markerName is the empty string, then return false.
    if (markerName->empty())
    {
      return false;
    }

    // 4. Let (start, end) be the result of find markers given scope and markerName.
    auto [start, end] = FindMarkers(scope, *markerName);

    // 5. If start is null, then return false.
    if (!start)
    {
      return false;
    }

    // 6. Set template's insertion target to start's parent.
    // 7. Set template's insertion start marker to start.
    // 8. Set template's insertion end marker to end.
    _insertionTarget = ShareRefPtr(start->ParentNode());
    _insertionStartMarker = ShareRefPtr(start);
    _insertionEndMarker = ShareRefPtr(end);

    // 9. If start is end, then return true.
    if (start == end)
    {
      return true;
    }

    // 10. Let removedNodes be an empty list.
    // 11. Let currentNode be start's next sibling.
    // 12. While currentNode is not null: if currentNode is end, then break; append currentNode to
    //     removedNodes; and set currentNode to currentNode's next sibling.
    List<Ref<dom::Node>> removedNodes;
    for (auto *currentNode = start->NextSibling(); currentNode && currentNode != end;
         currentNode = currentNode->NextSibling())
    {
      removedNodes.push_back(ShareRef(*currentNode));
    }

    // 13. For each node of removedNodes: if node's parent is not null, then remove node.
    for (auto &node : removedNodes)
    {
      if (node->ParentNode())
      {
        (void)dom::MutationAlgorithms::Remove(*node);
      }
    }

    // 14. Return true.
    return true;
  }

  // https://html.spec.whatwg.org/#the-template-element:concept-node-adopt-ext
  void HTMLTemplateElement::OnAdopt(dom::Document &oldDocument) noexcept
  {
    HTMLElement::OnAdopt(oldDocument);

    // 1. Let document be node's node document's appropriate template contents owner document.
    // (The template contents are in the template's own document.)
    // 2. Adopt node's template contents (a DocumentFragment object) into document.
    // (Not a declarative shadow root, which is a host's.)
    if (!Is<dom::ShadowRoot>(*_content))
    {
      (void)dom::DocumentAlgorithms::AdoptNode(*_content, NodeDocument());
    }
  }

  // https://html.spec.whatwg.org/#the-template-element:concept-node-clone-ext
  void HTMLTemplateElement::OnClone(dom::Node &copy, bool subtree) noexcept
  {
    HTMLElement::OnClone(copy, subtree);

    // 1. If subtree is false, then return.
    if (!subtree)
    {
      return;
    }

    // 2. For each child of node's template contents's children, in tree order: clone a node given child with
    //    document set to copy's template contents's node document, subtree set to true, and parent set to
    //    copy's template contents.
    auto &copyContents = Downcast<HTMLTemplateElement>(copy).TemplateContents();
    for (auto &child : dom::ChildNodeRange(TemplateContents()))
    {
      (void)dom::NodeAlgorithms::CloneNode(child, &copyContents.NodeDocument(), true, &copyContents);
    }
  }

  dom::DOMString HTMLTemplateElement::ShadowRootMode() const noexcept
  {
    return ReflectEnumerated<"shadowrootmode", HTMLTemplateElement>(*this);
  }

  void HTMLTemplateElement::ShadowRootMode(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"shadowrootmode", krys::move(value));
  }

  bool HTMLTemplateElement::ShadowRootDelegatesFocus() const noexcept
  {
    return Reflect<bool>(*this, u8"shadowrootdelegatesfocus");
  }

  void HTMLTemplateElement::ShadowRootDelegatesFocus(bool value) noexcept
  {
    Reflect<bool>(*this, u8"shadowrootdelegatesfocus", krys::move(value));
  }

  dom::DOMString HTMLTemplateElement::ShadowRootSlotAssignment() const noexcept
  {
    return ReflectEnumerated<"shadowrootslotassignment", HTMLTemplateElement>(*this);
  }

  void HTMLTemplateElement::ShadowRootSlotAssignment(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"shadowrootslotassignment", krys::move(value));
  }

  bool HTMLTemplateElement::ShadowRootClonable() const noexcept
  {
    return Reflect<bool>(*this, u8"shadowrootclonable");
  }

  void HTMLTemplateElement::ShadowRootClonable(bool value) noexcept
  {
    Reflect<bool>(*this, u8"shadowrootclonable", krys::move(value));
  }

  bool HTMLTemplateElement::ShadowRootSerialisable() const noexcept
  {
    return Reflect<bool>(*this, u8"shadowrootserializable");
  }

  void HTMLTemplateElement::ShadowRootSerialisable(bool value) noexcept
  {
    Reflect<bool>(*this, u8"shadowrootserializable", krys::move(value));
  }

  dom::DOMString HTMLTemplateElement::ShadowRootCustomElementRegistry() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"shadowrootcustomelementregistry").value_or(u8"");
  }

  void HTMLTemplateElement::ShadowRootCustomElementRegistry(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"shadowrootcustomelementregistry", krys::move(value));
  }

#pragma endregion
}