#include "Krystal.HTML/HTML/Parser/HTMLTreeBuilder.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/DOM/Comment.hpp"
#include "Krystal.HTML/DOM/HTMLDocument.hpp"
#include "Krystal.HTML/DOM/Internals/ElementFactory.hpp"
#include "Krystal.HTML/DOM/Text.hpp"
#include "Krystal.HTML/HTML/Enums/HTMLTokenType.hpp"
#include "Krystal.HTML/HTML/HTMLTableCellElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableRowElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableSectionElement.hpp"
#include "Krystal.HTML/HTML/HTMLTemplateElement.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  HTMLTreeBuilder::HTMLTreeBuilder(Document &document, HTMLTokenizer &tokenizer,
                                   RawPtr<Element> contextElement) noexcept
      : _document(document), _tokenizer(tokenizer), _contextElement(contextElement)
  {
  }

  void HTMLTreeBuilder::ProcessToken(HTMLTokenAtom &&token) noexcept
  {
    // TODO
    // If the stack of open elements is empty
    // If the adjusted current node is an element in the HTML namespace
    // If the adjusted current node is a MathML text integration point and the token is a start tag whose tag
    //   name is neither "mglyph" nor "malignmark"
    // If the adjusted current node is a MathML text integration point and the token is a character token
    // If the adjusted current node is a MathML annotation-xml element and the token is a start tag whose tag
    //   name is "svg"
    // If the adjusted current node is an HTML integration point and the token is a start tag
    // If the adjusted current node is an HTML integration point and the token is a character token
    // If the token is an end-of-file token
    //     Process the token according to the rules given in the section corresponding to the current
    //     insertion mode in HTML content.
    // Otherwise
    //     Process the token according to the rules given in the section for parsing tokens in foreign
    //     content.
  }

  void HTMLTreeBuilder::ResetInsertionModeAppropriately() noexcept
  {
    bool last = false;

    auto *stackElement = &_openElementStack.Bottom();
    auto *node = &stackElement->Node();

    while (true)
    {
      if (node == &_openElementStack.Top().Node())
      {
        last = true;

        if (_contextElement != nullptr)
        {
          node = _contextElement;
        }
      }

      if (auto *htmlElement = DynamicDowncast<HTMLElement>(node))
      {
        switch (stackElement->TagName())
        {
          case TagName::td:
          case TagName::th:
          {
            if (!last)
            {
              _insertionMode = InsertionMode::InCell;
              return;
            }

            break;
          }
          case TagName::tr:
          {
            _insertionMode = InsertionMode::InRow;
            return;
          }
          case TagName::tbody:
          case TagName::thead:
          case TagName::tfoot:
          {
            _insertionMode = InsertionMode::InTableBody;
            return;
          }
          case TagName::caption:
          {
            _insertionMode = InsertionMode::InCaption;
            return;
          }
          case TagName::colgroup:
          {
            _insertionMode = InsertionMode::InColumnGroup;
            return;
          }
          case TagName::table:
          {
            _insertionMode = InsertionMode::InTable;
            return;
          }
          case TagName::template_:
          {
            _insertionMode = CurrentTemplateInsertionMode();
            return;
          }
          case TagName::head:
          {
            if (!last)
            {
              _insertionMode = InsertionMode::InHead;
              return;
            }

            break;
          }
          case TagName::body:
          {
            _insertionMode = InsertionMode::InBody;
            return;
          }
          case TagName::frameset:
          {
            _insertionMode = InsertionMode::InFrameset;
            return;
          }
          case TagName::html:
          {
            if (_head == nullptr)
            {
              _insertionMode = InsertionMode::BeforeHead;
            }
            else
            {
              _insertionMode = InsertionMode::AfterHead;
            }
            return;
          }
          default:
          {
            break;
          }
        }
      }

      if (last)
      {
        _insertionMode = InsertionMode::InBody;
        return;
      }

      stackElement = _openElementStack.EntryBefore(*node);
      assert(stackElement != nullptr);

      node = &stackElement->Node();
    }
  }

  InsertionMode HTMLTreeBuilder::CurrentTemplateInsertionMode() const noexcept
  {
    assert(!_templateInsertionModes.empty());
    return _templateInsertionModes.back();
  }

  ContainerNode &HTMLTreeBuilder::CurrentNode() noexcept
  {
    return _openElementStack.Bottom().Node();
  }

  ContainerNode &HTMLTreeBuilder::AdjustedCurrentNode() noexcept
  {
    if (_contextElement != nullptr && _openElementStack.Size() == 1)
    {
      return *_contextElement;
    }
    else
    {
      return CurrentNode();
    }
  }

  bool HTMLTreeBuilder::HasElementInScope(TagName targetNode) const noexcept
  {
    auto *node = &_openElementStack.Bottom();

    while (true)
    {
      if (node->TagName() == targetNode)
      {
        return true;
      }

      switch (node->TagName())
      {
        case TagName::applet:
        case TagName::caption:
        case TagName::html:
        case TagName::table:
        case TagName::td:
        case TagName::th:
        case TagName::marquee:
        case TagName::object:
        case TagName::select:
        case TagName::template_:
        {
          // TODO(HTMLTREEBUILDER, HTML) also: MathML mi MathML mo MathML mn MathML ms MathML mtext MathML
          // annotation-xml SVG foreignObject SVG desc SVG title
          return false;
        }
        default:
        {
          break;
        }
      }

      node = _openElementStack.EntryBefore(node->Node());
    }

    return false;
  }

  bool HTMLTreeBuilder::HasElementInListItemScope(TagName targetNode) const noexcept
  {
    auto *node = &_openElementStack.Bottom();

    while (true)
    {
      if (node->TagName() == targetNode)
      {
        return true;
      }

      switch (node->TagName())
      {
        case TagName::applet:
        case TagName::caption:
        case TagName::html:
        case TagName::table:
        case TagName::td:
        case TagName::th:
        case TagName::li:
        case TagName::marquee:
        case TagName::object:
        case TagName::select:
        case TagName::template_:
        case TagName::ul:
        {
          // TODO(HTMLTREEBUILDER, HTML) also: MathML mi MathML mo MathML mn MathML ms MathML mtext MathML
          // annotation-xml SVG foreignObject SVG desc SVG title
          return false;
        }
        default:
        {
          break;
        }
      }

      node = _openElementStack.EntryBefore(node->Node());
    }

    return false;
  }

  bool HTMLTreeBuilder::HasElementInButtonScope(TagName targetNode) const noexcept
  {
    auto *node = &_openElementStack.Bottom();

    while (true)
    {
      if (node->TagName() == targetNode)
      {
        return true;
      }

      switch (node->TagName())
      {
        case TagName::applet:
        case TagName::button:
        case TagName::caption:
        case TagName::html:
        case TagName::table:
        case TagName::td:
        case TagName::th:
        case TagName::marquee:
        case TagName::object:
        case TagName::select:
        case TagName::template_:
        {
          // TODO(HTMLTREEBUILDER, HTML) also: MathML mi MathML mo MathML mn MathML ms MathML mtext MathML
          // annotation-xml SVG foreignObject SVG desc SVG title
          return false;
        }
        default:
        {
          break;
        }
      }

      node = _openElementStack.EntryBefore(node->Node());
    }

    return false;
  }

  bool HTMLTreeBuilder::HasElementInTableScope(TagName targetNode) const noexcept
  {
    auto *node = &_openElementStack.Bottom();

    while (true)
    {
      if (node->TagName() == targetNode)
      {
        return true;
      }

      switch (node->TagName())
      {
        case TagName::html:
        case TagName::table:
        case TagName::template_:
        {
          return false;
        }
        default:
        {
          break;
        }
      }

      node = _openElementStack.EntryBefore(node->Node());
    }

    return false;
  }

  AdjustedInsertionLocation
    HTMLTreeBuilder::AppropriateInsertionLocation(RawPtr<ContainerNode> targetOverride) noexcept
  {
    auto &target = targetOverride ? *targetOverride : CurrentNode();

    AdjustedInsertionLocation location {};

    auto *targetHTMLElement = DynamicDowncast<HTMLElement>(target);
    if (_fosterParenting
        && IsOneOf<HTMLTableElement, HTMLTableSectionElement, HTMLTableRowElement>(targetHTMLElement))
    {
      auto [tableElement, templateElement, elementBeforeTable, isTemplateMostRecent] =
        _openElementStack.LastTableAndTemplate();

      if (templateElement != nullptr && (tableElement == nullptr || isTemplateMostRecent))
      {
        location = {.Parent = templateElement->Content().get()};
      }
      else if (tableElement == nullptr)
      {
        location = {.Parent = &_openElementStack.Top().Node()}; // should be the html element
      }
      else if (tableElement->ParentNode() != nullptr)
      {
        location = {.Parent = tableElement->ParentNode(), .BeforeSibling = tableElement};
      }
      else
      {
        location = {.Parent = elementBeforeTable};
      }
    }
    else
    {
      location.Parent = &target;
      location.BeforeSibling = nullptr;
    }

    if (auto *templateElement = DynamicDowncast<HTMLTemplateElement>(location.Parent))
    {
      location.Parent = templateElement->Content().get();
      location.BeforeSibling = nullptr;
    }

    return location;
  }

  Ref<Element> HTMLTreeBuilder::CreateElement(HTMLTokenAtom &token, DOMStringAtom namespaceURI,
                                              ContainerNode &intendedParent) noexcept
  {
    auto &document = intendedParent.NodeDocument();
    auto localName = token.Name();
    auto is = [&]() -> DOMStringAtom
    {
      auto it = std::ranges::find_if(token.Attributes(),
                                     [](const ParsedAttribute &attr) { return attr.NameView() == u8"is"; });
      return it != std::ranges::end(token.Attributes()) ? it->NameView() : DOMStringAtom::Null();
    }();

    // TODO(HTMLTREEBUILDER, CUSTOMELEMENTS, HTML): Let registry be the result of looking up a custom element
    // registry given intendedParent.
    // Let definition be the result of looking up a custom element definition given registry, namespace,
    // localName, and is.
    // Let willExecuteScript be true if definition is non-null and the parser was not created as part of the
    // HTML fragment parsing algorithm; otherwise false.
    // If willExecuteScript is true:
    // Increment document's throw-on-dynamic-markup-insertion counter.
    // If the JavaScript execution context stack is empty, then perform a microtask checkpoint.
    // Push a new element queue onto document's relevant agent's custom element reactions stack.
    RefPtr<CustomElementRegistry> registry = nullptr;
    bool willExecuteScript = false;

    auto element = ElementFactory::Create(document, {namespaceURI, DOMStringAtom::Null(), localName}, is,
                                          willExecuteScript, registry);
    for (auto &attr : token.Attributes())
    {
      ElementAlgorithms::SetAttributeValue(*element, attr.NameView(), DOMString(attr.ValueView()));
    }

    // TODO(HTMLTREEBUILDER, CUSTOMELEMENTS, HTML):
    // If willExecuteScript is true:
    //     Let queue be the result of popping from document's relevant agent's custom element reactions stack.
    //     (This will be the same element queue as was pushed above.)
    //     Invoke custom element reactions in queue.
    //     Decrement document's throw-on-dynamic-markup-insertion counter.
    //
    // TODO(HTMLTREEBUILDER, HTML):
    // If element has an xmlns attribute in the XMLNS namespace whose value is not exactly the same as the
    // element's namespace, that is a parse error. Similarly, if element has an xmlns:xlink attribute in the
    // XMLNS namespace whose value is not the XLink Namespace, that is a parse error.
    //
    // TODO(HTMLTREEBUILDER, HTML):
    // If element is a resettable element and not a form-associated custom element, then invoke its reset
    // algorithm. (This initializes the element's value and checkedness based on the element's attributes.)
    //
    // TODO(HTMLTREEBUILDER, HTML):
    // If element is a form-associated element and not a form-associated custom element, the form element
    // pointer is not null, there is no template element on the stack of open elements, element is either not
    // listed or doesn't have a form attribute, and the intendedParent is in the same tree as the element
    // pointed to by the form element pointer, then associate element with the form element pointed to by the
    // form element pointer and set element's parser inserted flag.

    return element;
  }

  void HTMLTreeBuilder::InsertElementAtAdjustedInsertionLocation(Element &element) noexcept
  {
    auto [parent, beforeSibling] = AppropriateInsertionLocation();

    if (auto result = MutationAlgorithms::EnsurePreInsertValidity(element, *parent, beforeSibling);
        result.HasException())
    {
      return;
    }

    // TODO(HTMLTREEBUILDER, CUSTOMELEMENTS, HTML):
    // If the parser was not created as part of the HTML fragment parsing algorithm, then push a new element
    // queue onto element's relevant agent's custom element reactions stack.

    // NOTE: We purposely ignore the error here if it happens.
    (void)MutationAlgorithms::Insert(element, *parent, beforeSibling);

    // TODO(HTMLTREEBUILDER, CUSTOMELEMENTS, HTML):
    // If the parser was not created as part of the HTML fragment parsing algorithm, then pop the element
    // queue from element's relevant agent's custom element reactions stack, and invoke custom element
    // reactions in that queue.
  }

  Ref<Element> HTMLTreeBuilder::InsertForeignElement(HTMLTokenAtom &&token, DOMStringAtom namespaceURI,
                                                     bool onlyAddToElementStack) noexcept
  {
    auto [parent, beforeSibling] = AppropriateInsertionLocation();
    auto element = CreateElement(token, namespaceURI, *parent);

    if (!onlyAddToElementStack)
    {
      InsertElementAtAdjustedInsertionLocation(*element);
    }

    auto tagName = ParseTagName(token.Name().View());
    auto tagNamespace = ParseNamespace(namespaceURI.View());
    HTMLStackItem stackItem(tagName, tagNamespace, *element, Krys::Move(token.Attributes()));

    _openElementStack.Push(Krys::Move(stackItem));

    return element;
  }

  void HTMLTreeBuilder::InsertHTMLElement(HTMLTokenAtom &&token) noexcept
  {
    InsertForeignElement(Krys::Move(token), Namespaces::HTML, false);
  }

  void HTMLTreeBuilder::AdjustMathMLAttributes(HTMLTokenAtom &token) noexcept
  {
    // TODO(HTMLTREEBUILDER, HTML): Adjust attributes for mathml elements.
  }

  void HTMLTreeBuilder::AdjustSVGAttributes(HTMLTokenAtom &token) noexcept
  {
    // TODO(HTMLTREEBUILDER, HTML): Adjust attributes for svg elements.
  }

  void HTMLTreeBuilder::AdjustForeignAttributes(HTMLTokenAtom &token) noexcept
  {
    // TODO(HTMLTREEBUILDER, HTML): Adjust attributes for foreign elements.
  }

  void HTMLTreeBuilder::InsertCharacter(DOMString &&data) noexcept
  {
    auto [parent, beforeSibling] = AppropriateInsertionLocation();

    if (Is<Document>(parent))
    {
      return;
    }

    auto previousSibling = beforeSibling;
    if (previousSibling == nullptr)
    {
      previousSibling = parent->LastChild();
    }

    if (Is<HTML::Text>(previousSibling))
    {
      auto &textNode = Downcast<HTML::Text>(*previousSibling);
      textNode.AppendData(Krys::Move(data));
    }
    else
    {
      auto textNode = CreateRef<HTML::Text>(parent->NodeDocument(), Krys::Move(data));

      // NOTE: We purposely ignore the error here if it happens.
      (void)MutationAlgorithms::Insert(*textNode, *parent, beforeSibling);
    }
  }

  void HTMLTreeBuilder::InsertComment(DOMString &&data, Maybe<AdjustedInsertionLocation> position) noexcept
  {
    auto [parent, beforeSibling] = position.has_value() ? *position : AppropriateInsertionLocation();
    auto commentNode = CreateRef<Comment>(parent->NodeDocument(), Krys::Move(data));

    // NOTE: We purposely ignore the error here if it happens.
    (void)MutationAlgorithms::Insert(*commentNode, *parent, beforeSibling);
  }

  void HTMLTreeBuilder::ParseGenericRawTextElement(HTMLTokenAtom &&token) noexcept
  {
    InsertHTMLElement(Krys::Move(token));
    _tokenizer.State(TokenizerState::RAWTEXT);
    _originalInsertionMode = _insertionMode;
    _insertionMode = InsertionMode::Text;
  }

  void HTMLTreeBuilder::ParseGenericRCDATATextElement(HTMLTokenAtom &&token) noexcept
  {
    InsertHTMLElement(Krys::Move(token));
    _tokenizer.State(TokenizerState::RCDATA);
    _originalInsertionMode = _insertionMode;
    _insertionMode = InsertionMode::Text;
  }

  void HTMLTreeBuilder::GenerateImpliedEndTags(Maybe<TagName> exception) noexcept
  {
    while (true)
    {
      auto &currentNode = _openElementStack.Bottom();
      switch (currentNode.TagName())
      {
        case TagName::dd:
        case TagName::dt:
        case TagName::li:
        case TagName::optgroup:
        case TagName::option:
        case TagName::p:
        case TagName::rb:
        case TagName::rp:
        case TagName::rt:
        case TagName::rtc:
        {
          if (exception.has_value() && currentNode.TagName() == exception.value())
          {
            return;
          }

          _openElementStack.Pop();
          break;
        }
        default:
        {
          return;
        }
      }
    }
  }

  void HTMLTreeBuilder::GenerateImpliedEndTagsThoroughly() noexcept
  {
    while (true)
    {
      auto &currentNode = _openElementStack.Bottom();
      switch (currentNode.TagName())
      {
        case TagName::caption:
        case TagName::colgroup:
        case TagName::dd:
        case TagName::dt:
        case TagName::li:
        case TagName::optgroup:
        case TagName::option:
        case TagName::p:
        case TagName::rb:
        case TagName::rp:
        case TagName::rt:
        case TagName::rtc:
        case TagName::tbody:
        case TagName::td:
        case TagName::tfoot:
        case TagName::th:
        case TagName::thead:
        case TagName::tr:
        {
          _openElementStack.Pop();
          break;
        }
        default:
        {
          return;
        }
      }
    }
  }

#pragma region InsertionMode Algorithms

  void HTMLTreeBuilder::Initial(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::BeforeHTML(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::BeforeHead(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InHead(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InHeadNoscript(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::AfterHead(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InBody(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::Text(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InTable(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InTableText(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InCaption(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InColumnGroup(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InTableBody(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InRow(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InCell(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InSelect(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InSelectInTable(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InTemplate(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::AfterBody(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InFrameset(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::AfterFrameset(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::AfterAfterBody(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::AfterAfterFrameset(HTMLTokenAtom &&token) noexcept
  {
  }

#pragma endregion
}
