#include "Krystal.HTML/HTML/Parser/HTMLTreeBuilder.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/DOM/Comment.hpp"
#include "Krystal.HTML/DOM/DocumentType.hpp"
#include "Krystal.HTML/DOM/HTMLDocument.hpp"
#include "Krystal.HTML/DOM/Internals/ElementFactory.hpp"
#include "Krystal.HTML/DOM/Text.hpp"
#include "Krystal.HTML/HTML/Enums/HTMLTokenType.hpp"
#include "Krystal.HTML/HTML/HTMLBodyElement.hpp"
#include "Krystal.HTML/HTML/HTMLHtmlElement.hpp"
#include "Krystal.HTML/HTML/HTMLScriptElement.hpp"
#include "Krystal.HTML/HTML/HTMLSelectElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableCellElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableRowElement.hpp"
#include "Krystal.HTML/HTML/HTMLTableSectionElement.hpp"
#include "Krystal.HTML/HTML/HTMLTemplateElement.hpp"
#include "Krystal.HTML/HTML/Internals/HTMLElementFactory.hpp"
#include "Krystal.HTML/Infra/StringAlgorithms.hpp"
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

    switch (_insertionMode)
    {
      case InsertionMode::Initial:            InitialMode(Krys::Move(token)); break;
      case InsertionMode::BeforeHTML:         BeforeHTMLMode(Krys::Move(token)); break;
      case InsertionMode::BeforeHead:         BeforeHeadMode(Krys::Move(token)); break;
      case InsertionMode::InHead:             InHeadMode(Krys::Move(token)); break;
      case InsertionMode::InHeadNoscript:     InHeadNoscriptMode(Krys::Move(token)); break;
      case InsertionMode::AfterHead:          AfterHeadMode(Krys::Move(token)); break;
      case InsertionMode::InBody:             InBodyMode(Krys::Move(token)); break;
      case InsertionMode::Text:               TextMode(Krys::Move(token)); break;
      case InsertionMode::InTable:            InTableMode(Krys::Move(token)); break;
      case InsertionMode::InTableText:        InTableTextMode(Krys::Move(token)); break;
      case InsertionMode::InCaption:          InCaptionMode(Krys::Move(token)); break;
      case InsertionMode::InColumnGroup:      InColumnGroupMode(Krys::Move(token)); break;
      case InsertionMode::InTableBody:        InTableBodyMode(Krys::Move(token)); break;
      case InsertionMode::InRow:              InRowMode(Krys::Move(token)); break;
      case InsertionMode::InCell:             InCellMode(Krys::Move(token)); break;
      case InsertionMode::InSelect:           InSelectMode(Krys::Move(token)); break;
      case InsertionMode::InSelectInTable:    InSelectInTableMode(Krys::Move(token)); break;
      case InsertionMode::InTemplate:         InTemplateMode(Krys::Move(token)); break;
      case InsertionMode::AfterBody:          AfterBodyMode(Krys::Move(token)); break;
      case InsertionMode::InFrameset:         InFramesetMode(Krys::Move(token)); break;
      case InsertionMode::AfterFrameset:      AfterFramesetMode(Krys::Move(token)); break;
      case InsertionMode::AfterAfterBody:     AfterAfterBodyMode(Krys::Move(token)); break;
      case InsertionMode::AfterAfterFrameset: AfterAfterFramesetMode(Krys::Move(token)); break;
    }
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

  Ref<Element> HTMLTreeBuilder::InsertHTMLElement(HTMLTokenAtom &&token) noexcept
  {
    return InsertForeignElement(Krys::Move(token), Namespaces::HTML, false);
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
    // TODO(HTMLTREEBUILDER, HTML): parse error if it contains a null character; skip them.

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

  void HTMLTreeBuilder::InitialMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        if (SkipCharacterTokenWhitespace(token))
        {
          break; // Reprocess remaining characters in the BeforeHTML insertion mode.
        }

        return;
      }
      case HTMLTokenType::Comment:
      {
        InsertComment(DOMString(token.Data()), AdjustedInsertionLocation {&_document, nullptr});
        return;
      }
      case HTMLTokenType::DOCTYPE:
      {
        auto &docTypeData = *token.DOCTYPEData();

        // TODO(HTMLTREEBUILDER, HTML): If the DOCTYPE token's name is not "html", or the token's public
        // identifier is not missing, or the token's system identifier is neither missing nor
        // "about:legacy-compat", then there is a parse error.

        auto name = DOMString(token.Name().View());
        auto publicIdentifier =
          DOMString(docTypeData.PublicIdentifier.begin(), docTypeData.PublicIdentifier.end());
        auto systemIdentifier =
          DOMString(docTypeData.SystemIdentifier.begin(), docTypeData.SystemIdentifier.end());

        auto doctype =
          _document.Implementation().CreateDocumentType(name, publicIdentifier, systemIdentifier);
        if (!doctype.HasException())
        {
          _document.AppendChild(**doctype);
        }

        // TODO(HTMLTREEBUILDER, HTML): Then, if the document is not an iframe srcdoc document &&
        if (!_document._parserCannotChangeTheMode)
        {
          if (IsQuirksModeDOCTYPE(token))
          {
            _document._quirksMode = QuirksMode::Quirks;
          }
          else if (IsLimitedQuirksModeDOCTYPE(token))
          {
            _document._quirksMode = QuirksMode::LimitedQuirks;
          }
        }

        _insertionMode = InsertionMode::BeforeHTML;
        return;
      }
      default: break;
    }

    // TODO(HTMLTREEBUILDER, HTML): If the document is not an iframe srcdoc document, then this is a parse
    // error

    if (!_document._parserCannotChangeTheMode)
    {
      _document._quirksMode = QuirksMode::Quirks;
    }

    _insertionMode = InsertionMode::BeforeHTML;
    BeforeHTMLMode(Krys::Move(token));
  }

  void HTMLTreeBuilder::BeforeHTMLMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::DOCTYPE:
      {
        // TODO(HTMLTREEBUILDER, HTML): parse error
        return; // ignore the token
      }
      case HTMLTokenType::Comment:
      {
        InsertComment(DOMString(token.Data()), AdjustedInsertionLocation {&_document, nullptr});
        return;
      }
      case HTMLTokenType::Character:
      {
        if (SkipCharacterTokenWhitespace(token))
        {
          break; // Reprocess remaining characters in the BeforeHead insertion mode.
        }

        return;
      }
      case HTMLTokenType::StartTag:
      {
        if (token.Name() != u8"html")
        {
          break;
        }

        auto html = CreateElement(token, Namespaces::HTML, _document);
        _document.AppendChild(*html);
        _openElementStack.Push({TagName::html, Namespace::HTML, *html, Krys::Move(token.Attributes())});
        _insertionMode = InsertionMode::BeforeHead;

        return;
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == u8"head" || token.Name() == u8"body" || token.Name() == u8"html"
            || token.Name() == u8"br")
        {
          break; // treat as the 'default' case below.
        }

        // TODO(HTMLTREEBUILDER, HTML): parse error
        return; // ignore the token
      }
      default: break;
    }

    auto html = HTMLElementFactory::TryCreate(_document, TagName::html);
    assert(html != nullptr);
    _document.AppendChild(*html);
    _openElementStack.Push({TagName::html, Namespace::HTML, *html, {}});
    _insertionMode = InsertionMode::BeforeHead;

    BeforeHeadMode(Krys::Move(token));
  }

  void HTMLTreeBuilder::BeforeHeadMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        if (SkipCharacterTokenWhitespace(token))
        {
          break; // Reprocess remaining characters in the InHead insertion mode.
        }

        return;
      }
      case HTMLTokenType::Comment:
      {
        InsertComment(DOMString(token.Data()));
        return;
      }
      case HTMLTokenType::DOCTYPE:
      {
        // TODO(HTMLTREEBUILDER, HTML): parse error
        return; // ignore the token
      }
      case HTMLTokenType::StartTag:
      {
        if (token.Name() == u8"html")
        {
          // Process the token using the rules for the "in body" insertion mode.
          InBodyMode(Krys::Move(token));
          return;
        }

        if (token.Name() == u8"head")
        {
          _head = CreateElement(token, Namespaces::HTML, _document);
          _openElementStack.Push({TagName::head, Namespace::HTML, *_head, Krys::Move(token.Attributes())});
          _insertionMode = InsertionMode::InHead;

          return;
        }

        break;
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == u8"head" || token.Name() == u8"body" || token.Name() == u8"html"
            || token.Name() == u8"br")
        {
          break; // treat as the 'default' case below.
        }

        // TODO(HTMLTREEBUILDER, HTML): parse error
        return; // ignore the token
      }
      default: break;
    }

    _head = HTMLElementFactory::TryCreate(_document, TagName::head);
    assert(_head != nullptr);
    _document.AppendChild(*_head);
    _openElementStack.Push({TagName::head, Namespace::HTML, *_head, {}});

    _insertionMode = InsertionMode::InHead;
    InHeadMode(Krys::Move(token));
  }

  void HTMLTreeBuilder::InHeadMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        if (InsertCharacterTokenWhitespace(token))
        {
          break;
        }

        return;
      }
      case HTMLTokenType::Comment:
      {
        InsertComment(DOMString(token.Data()));
        return;
      }
      case HTMLTokenType::DOCTYPE:
      {
        // TODO(HTMLTREEBUILDER, HTML): parse error
        return; // ignore the token
      }
      case HTMLTokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::html:
          {
            InBodyMode(Krys::Move(token));
            return;
          }
          case TagName::base:
          case TagName::basefont:
          case TagName::bgsound:
          case TagName::link:
          {
            InsertHTMLElement(Krys::Move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();
            return;
          }
          case TagName::meta:
          {
            InsertHTMLElement(Krys::Move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();

            // TODO(HTMLTREEBUILDER, HTML): If the active speculative HTML parser is null:
            // If the element has a charset attribute, and getting an encoding from its value results in an
            // encoding, and the confidence is currently tentative, then change the encoding to the resulting
            // encoding.
            // Otherwise, if the element has an http-equiv attribute whose value is an ASCII case-insensitive
            // match for "Content-Type", and the element has a content attribute, and applying the algorithm
            // for extracting a character encoding from a meta element to that attribute's value returns an
            // encoding, and the confidence is currently tentative, then change the encoding to the extracted
            // encoding.

            return;
          }
          case TagName::title:
          {
            ParseGenericRCDATATextElement(Krys::Move(token));
            return;
          }
          case TagName::noscript:
          {
            if (_scriptingMode == ParserScriptingMode::Disabled)
            {
              InsertHTMLElement(Krys::Move(token));
              _insertionMode = InsertionMode::InHeadNoscript;
            }
            else
            {
              ParseGenericRawTextElement(Krys::Move(token));
            }
            return;
          }
          case TagName::noframes:
          case TagName::style:
          {
            ParseGenericRawTextElement(Krys::Move(token));
            return;
          }
          case TagName::script:
          {
            auto [parent, beforeSibling] = AppropriateInsertionLocation();

            auto element = CreateElement(token, Namespaces::HTML, *parent);
            auto &script = Downcast<HTMLScriptElement>(*element);

            if (_scriptingMode != ParserScriptingMode::Fragment)
            {
              script._parserDocument = &_document;
            }

            script._forceAsync = false;

            if (_scriptingMode == ParserScriptingMode::Inert)
            {
              script._alreadyStarted = true;
            }

            // If the parser was invoked via the document.write() or document.writeln() methods, then
            // optionally set the script element's already started to true. (For example, the user agent might
            // use this clause to prevent execution of cross-origin scripts inserted via document.write()
            // under slow network conditions, or when the page has already taken a long time to load.)

            (void)MutationAlgorithms::PreInsert(script, *parent, beforeSibling);
            _openElementStack.Push(
              {TagName::script, Namespace::HTML, script, Krys::Move(token.Attributes())});

            _tokenizer.State(TokenizerState::ScriptData);
            _originalInsertionMode = _insertionMode;
            _insertionMode = InsertionMode::Text;

            return;
          }
          case TagName::template_:
          {
            // TODO(HTMLTREEBUILDER, HTML): handle template start tag in the head insertion mode.
            return;
          }
          case TagName::head:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error
            return; // ignore the token
          }
          default: break;
        }

        break;
      }
      case HTMLTokenType::EndTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::head:
          {
            _openElementStack.Pop();
            _insertionMode = InsertionMode::AfterHead;
            return;
          }
          case TagName::body:
          case TagName::html:
          case TagName::br:
          {
            break;
          }
          case TagName::template_:
          {
            // TODO(HTMLTREEBUILDER, HTML): handle template end tag in the head insertion mode.
            return;
          }
          default: break;
        }

        // TODO(HTMLTREEBUILDER, HTML): parse error
        return; // ignore the token
      }
      default: break;
    }

    _openElementStack.Pop();
    _insertionMode = InsertionMode::AfterHead;
    AfterHeadMode(Krys::Move(token));
  }

  void HTMLTreeBuilder::InHeadNoscriptMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::DOCTYPE:
      {
        // TODO(HTMLTREEBUILDER, HTML): parse error
        return; // ignore the token
      }
      case HTMLTokenType::Character:
      {
        if (InsertCharacterTokenWhitespace(token))
        {
          break;
        }

        return;
      }
      case HTMLTokenType::Comment:
      {
        InHeadMode(Krys::Move(token));
        return;
      }
      case HTMLTokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::html:
          {
            InBodyMode(Krys::Move(token));
            return;
          }
          case TagName::basefont:
          case TagName::bgsound:
          case TagName::link:
          case TagName::meta:
          case TagName::noframes:
          case TagName::style:
          {
            InHeadMode(Krys::Move(token));
            return;
          }
          case TagName::head:
          case TagName::noscript:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error
            return; // ignore the token
          }
          default: break;
        }
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == u8"noscript")
        {
          _openElementStack.Pop();
          _insertionMode = InsertionMode::InHead;
          return;
        }

        if (token.Name() == u8"br")
        {
          break;
        }

        // TODO(HTMLTREEBUILDER, HTML): parse error
        return; // ignore the token
      }
      default: break;
    }

    // TODO(HTMLTREEBUILDER, HTML): parse error
    _openElementStack.Pop();
    _insertionMode = InsertionMode::InHead;
    InHeadMode(Krys::Move(token));
  }

  void HTMLTreeBuilder::AfterHeadMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        if (InsertCharacterTokenWhitespace(token))
        {
          break;
        }

        return;
      }
      case HTMLTokenType::Comment:
      {
        InsertComment(DOMString(token.Data()));
        return;
      }
      case HTMLTokenType::DOCTYPE:
      {
        // TODO(HTMLTREEBUILDER, HTML): parse error
        return; // ignore the token
      }
      case HTMLTokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::html:
          {
            InBodyMode(Krys::Move(token));
            return;
          }
          case TagName::body:
          {
            InsertHTMLElement(Krys::Move(token));
            _framesetOk = false;
            _insertionMode = InsertionMode::InBody;
            return;
          }
          case TagName::frameset:
          {
            InsertHTMLElement(Krys::Move(token));
            _insertionMode = InsertionMode::InFrameset;
            return;
          }
          case TagName::base:
          case TagName::basefont:
          case TagName::bgsound:
          case TagName::link:
          case TagName::meta:
          case TagName::noframes:
          case TagName::script:
          case TagName::style:
          case TagName::template_:
          case TagName::title:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error

            _openElementStack.Push({TagName::html, Namespace::HTML, *_head, {}});
            InHeadMode(Krys::Move(token));

            // TODO(HTMLTREEBUILDER, HTML): Remove the node pointed to by the head element pointer from the
            // stack of open elements. (It might not be the current node at this point.)

            return;
          }
          case TagName::head:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error
            return; // ignore the token
          }
          default: break;
        }
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == u8"template")
        {
          InHeadMode(Krys::Move(token));
          return;
        }

        if (token.Name() == u8"body" || token.Name() == u8"html" || token.Name() == u8"br")
        {
          break;
        }

        // TODO(HTMLTREEBUILDER, HTML): parse error
        return; // ignore the token
      }
      default: break;
    }

    auto body = HTMLElementFactory::TryCreate(_document, TagName::body);
    assert(body != nullptr);

    InsertElementAtAdjustedInsertionLocation(*body);
    _openElementStack.Push({TagName::body, Namespace::HTML, *body, {}});
    _insertionMode = InsertionMode::InBody;

    InBodyMode(Krys::Move(token));
  }

  void HTMLTreeBuilder::InBodyMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        _activeFormattingElements.Reconstruct(_openElementStack);

        if (!InsertCharacterTokenWhitespace(token))
        {
          return;
        }

        InsertCharacter(DOMString(token.Data()));
        _framesetOk = false;

        return;
      }
      case HTMLTokenType::Comment:
      {
        InsertComment(DOMString(token.Data()));
        return;
      }
      case HTMLTokenType::DOCTYPE:
      {
        // TODO(HTMLTREEBUILDER, HTML): parse error
        return; // ignore the token
      }
      case HTMLTokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::html:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error

            if (_openElementStack.ContainsTemplateElement())
            {
              return; // ignore the token
            }

            assert(Is<HTMLHtmlElement>(_openElementStack.Top().Node()));
            auto &htmlElement = Downcast<HTMLHtmlElement>(_openElementStack.Top().Node());

            for (auto &attr : token.Attributes())
            {
              if (!htmlElement.HasAttribute(attr.NameView()))
              {
                htmlElement.SetAttribute(attr.NameView(), DOMString(attr.ValueView()));
              }
            }

            return;
          }
          case TagName::base:
          case TagName::basefont:
          case TagName::bgsound:
          case TagName::link:
          case TagName::meta:
          case TagName::noframes:
          case TagName::script:
          case TagName::style:
          case TagName::template_:
          case TagName::title:
          {
            InHeadMode(Krys::Move(token));
            return;
          }
          case TagName::body:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error

            if (_openElementStack.Size() == 1 || _openElementStack[1].TagName() != TagName::body
                || _openElementStack.ContainsTemplateElement())
            {
              return; // ignore the token
            }

            _framesetOk = false;

            assert(Is<HTMLBodyElement>(_openElementStack[1].Node()));
            auto &bodyElement = Downcast<HTMLBodyElement>(_openElementStack[1].Node());

            for (auto &attr : token.Attributes())
            {
              if (!bodyElement.HasAttribute(attr.NameView()))
              {
                bodyElement.SetAttribute(attr.NameView(), DOMString(attr.ValueView()));
              }
            }

            return;
          }
          case TagName::frameset:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error

            if (_openElementStack.Size() == 1 || _openElementStack[1].TagName() != TagName::body
                || _openElementStack.ContainsTemplateElement())
            {
              return; // ignore the token
            }

            if (!_framesetOk)
            {
              return; // ignore the token
            }

            auto &bodyElement = Downcast<HTMLBodyElement>(_openElementStack[1].Node());
            (void)MutationAlgorithms::Remove(bodyElement);

            while (_openElementStack.Size() > 1uz)
            {
              _openElementStack.Pop();
            }

            InsertHTMLElement(Krys::Move(token));
            _insertionMode = InsertionMode::InFrameset;

            return;
          }
          case TagName::address:
          case TagName::article:
          case TagName::aside:
          case TagName::blockquote:
          case TagName::center:
          case TagName::details:
          case TagName::dialog:
          case TagName::dir:
          case TagName::div:
          case TagName::dl:
          case TagName::fieldset:
          case TagName::figcaption:
          case TagName::figure:
          case TagName::footer:
          case TagName::header:
          case TagName::hgroup:
          case TagName::main:
          case TagName::menu:
          case TagName::nav:
          case TagName::ol:
          case TagName::p:
          case TagName::search:
          case TagName::section:
          case TagName::summary:
          case TagName::ul:
          {
            if (HasElementInButtonScope(TagName::p))
            {
              ClosePElement();
            }

            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::h1:
          case TagName::h2:
          case TagName::h3:
          case TagName::h4:
          case TagName::h5:
          case TagName::h6:
          {
            if (HasElementInButtonScope(TagName::p))
            {
              ClosePElement();
            }

            auto &currentNode = _openElementStack.Bottom();
            if (currentNode.TagName() == TagName::h1 || currentNode.TagName() == TagName::h2
                || currentNode.TagName() == TagName::h3 || currentNode.TagName() == TagName::h4
                || currentNode.TagName() == TagName::h5 || currentNode.TagName() == TagName::h6)
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              _openElementStack.Pop();
            }

            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::pre:
          case TagName::listing:
          {
            if (HasElementInButtonScope(TagName::p))
            {
              ClosePElement();
            }

            InsertHTMLElement(Krys::Move(token));

            // TODO(HTMLTREEBUILDER, HTML): If the next token is a U+000A LINE FEED (LF) character token, then
            // ignore that token and move on to the next one. (Newlines at the start of pre blocks are ignored
            // as an authoring convenience.)

            _framesetOk = false;
            return;
          }
          case TagName::form:
          {
            auto containsTemplateElement = _openElementStack.ContainsTemplateElement();
            if (_form != nullptr && !containsTemplateElement)
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }

            if (HasElementInButtonScope(TagName::p))
            {
              ClosePElement();
            }

            auto element = InsertHTMLElement(Krys::Move(token));
            if (!containsTemplateElement)
            {
              _form = Krys::Move(element);
            }

            return;
          }
          case TagName::li:
          {
            _framesetOk = false;

            auto *node = &_openElementStack.Bottom();
          body_li_loop:
            if (node->TagName() == TagName::li)
            {
              GenerateImpliedEndTags(TagName::li);

              if (_openElementStack.Bottom().TagName() != TagName::li)
              {
                // TODO(HTMLTREEBUILDER, HTML): parse error
              }

              while (_openElementStack.Bottom().TagName() != TagName::li)
              {
                _openElementStack.Pop();
              }
              _openElementStack.Pop();

              goto body_li_done;
            }

            if (IsSpecialElement(node->TagName())
                && (node->TagName() != TagName::address || node->TagName() != TagName::div
                    || node->TagName() != TagName::p))
            {
              goto body_li_done;
            }

            node = _openElementStack.EntryBefore(node->Node());
            goto body_li_loop;

          body_li_done:
            if (HasElementInButtonScope(TagName::p))
            {
              ClosePElement();
            }

            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::dd:
          case TagName::dt:
          {
            _framesetOk = false;

            auto *node = &_openElementStack.Bottom();
          body_dd_loop:
            if (node->TagName() == TagName::dd)
            {
              GenerateImpliedEndTags(TagName::dd);

              if (_openElementStack.Bottom().TagName() != TagName::dd)
              {
                // TODO(HTMLTREEBUILDER, HTML): parse error
              }

              while (_openElementStack.Bottom().TagName() != TagName::dd)
              {
                _openElementStack.Pop();
              }
              _openElementStack.Pop();

              goto body_dd_done;
            }

            if (node->TagName() == TagName::dt)
            {
              GenerateImpliedEndTags(TagName::dt);

              // TODO: this needs to be the current node, not the bottom of the stack.
              if (_openElementStack.Bottom().TagName() != TagName::dt)
              {
                // TODO(HTMLTREEBUILDER, HTML): parse error
              }

              // TODO: this needs to be the current node, not the bottom of the stack.
              while (_openElementStack.Bottom().TagName() != TagName::dt)
              {
                _openElementStack.Pop();
              }
              _openElementStack.Pop();

              goto body_dd_done;
            }

            if (IsSpecialElement(node->TagName())
                && (node->TagName() != TagName::address || node->TagName() != TagName::div
                    || node->TagName() != TagName::p))
            {
              goto body_dd_done;
            }

            node = _openElementStack.EntryBefore(node->Node());
            goto body_dd_loop;

          body_dd_done:
            if (HasElementInButtonScope(TagName::p))
            {
              ClosePElement();
            }

            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::plaintext:
          {
            if (HasElementInButtonScope(TagName::p))
            {
              ClosePElement();
            }

            InsertHTMLElement(Krys::Move(token));

            _tokenizer.State(TokenizerState::PLAINTEXT);

            return;
          }
          case TagName::button:
          {
            if (HasElementInButtonScope(TagName::button))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              ClosePElement();

              while (_openElementStack.Bottom().TagName() != TagName::button)
              {
                _openElementStack.Pop();
              }
              _openElementStack.Pop();
            }

            _activeFormattingElements.Reconstruct(_openElementStack);

            InsertHTMLElement(Krys::Move(token));

            _framesetOk = false;

            return;
          }
          case TagName::a:
          {
            // If an a element is in the formatting list between the end and the last marker, this is a
            // parse error; run the adoption agency algorithm, then remove that element from the list and
            // stack of open elements if the adoption agency algorithm didn't already remove it.
            if (auto *existing = _activeFormattingElements.FindFormattingElementFromLastMarker(TagName::a))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              auto &existingNode = existing->Node();
              RunAdoptionAgency(token);
              _activeFormattingElements.RemoveFormattingElement(existingNode);
              _openElementStack.Remove(existingNode);
            }

            _activeFormattingElements.Reconstruct(_openElementStack);
            InsertHTMLElement(Krys::Move(token));

            auto &aOpenItem = _openElementStack.Bottom();
            ParsedAttributeList aAttrsCopy = aOpenItem.Attributes();
            _activeFormattingElements.PushElement(HTMLStackItem(aOpenItem.TagName(), aOpenItem.Namespace(),
                                                                Downcast<Element>(aOpenItem.Node()),
                                                                Krys::Move(aAttrsCopy)));
            return;
          }
          case TagName::b:
          case TagName::big:
          case TagName::code:
          case TagName::em:
          case TagName::font:
          case TagName::i:
          case TagName::s:
          case TagName::small:
          case TagName::strike:
          case TagName::strong:
          case TagName::tt:
          case TagName::u:
          {
            _activeFormattingElements.Reconstruct(_openElementStack);
            InsertHTMLElement(Krys::Move(token));

            auto &fmtOpenItem = _openElementStack.Bottom();
            ParsedAttributeList fmtAttrsCopy = fmtOpenItem.Attributes();
            _activeFormattingElements.PushElement(
              HTMLStackItem(fmtOpenItem.TagName(), fmtOpenItem.Namespace(),
                            Downcast<Element>(fmtOpenItem.Node()), Krys::Move(fmtAttrsCopy)));
            return;
          }
          case TagName::nobr:
          {
            _activeFormattingElements.Reconstruct(_openElementStack);

            if (HasElementInScope(TagName::nobr))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              RunAdoptionAgency(token);
              _activeFormattingElements.Reconstruct(_openElementStack);
            }

            InsertHTMLElement(Krys::Move(token));

            auto &nobrOpenItem = _openElementStack.Bottom();
            ParsedAttributeList nobrAttrsCopy = nobrOpenItem.Attributes();
            _activeFormattingElements.PushElement(
              HTMLStackItem(nobrOpenItem.TagName(), nobrOpenItem.Namespace(),
                            Downcast<Element>(nobrOpenItem.Node()), Krys::Move(nobrAttrsCopy)));
            return;
          }
          case TagName::applet:
          case TagName::marquee:
          case TagName::object:
          {
            _activeFormattingElements.Reconstruct(_openElementStack);
            InsertHTMLElement(Krys::Move(token));
            _activeFormattingElements.PushMarker();
            _framesetOk = false;
            return;
          }
          case TagName::table:
          {
            if (_document._quirksMode != QuirksMode::Quirks && HasElementInButtonScope(TagName::p))
            {
              ClosePElement();
            }

            InsertHTMLElement(Krys::Move(token));
            _framesetOk = false;
            _insertionMode = InsertionMode::InTable;
            return;
          }
          case TagName::area:
          case TagName::br:
          case TagName::embed:
          case TagName::img:
          case TagName::keygen:
          case TagName::wbr:
          {
            _activeFormattingElements.Reconstruct(_openElementStack);
            InsertHTMLElement(Krys::Move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();
            _framesetOk = false;
            return;
          }
          case TagName::input:
          {
            if (_scriptingMode == ParserScriptingMode::Fragment && Is<HTMLSelectElement>(_contextElement))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }

            if (HasElementInScope(TagName::select))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              while (_openElementStack.Bottom().TagName() != TagName::select)
              {
                _openElementStack.Pop();
              }
              _openElementStack.Pop();
            }

            bool hasHiddenType = false;
            for (const auto &attr : token.Attributes())
            {
              if (attr.NameView() == u8"type")
              {
                hasHiddenType = StringAlgorithms::ASCIICaseInsensitiveMatch(attr.ValueView(), u8"hidden");
                break;
              }
            }

            _activeFormattingElements.Reconstruct(_openElementStack);
            InsertHTMLElement(Krys::Move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();

            if (!hasHiddenType)
            {
              _framesetOk = false;
            }

            return;
          }
          case TagName::param:
          case TagName::source:
          case TagName::track:
          {
            InsertHTMLElement(Krys::Move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();
            return;
          }
          case TagName::hr:
          {
            if (HasElementInButtonScope(TagName::p))
            {
              ClosePElement();
            }

            if (HasElementInScope(TagName::select))
            {
              GenerateImpliedEndTags();
              if (HasElementInScope(TagName::option) || HasElementInScope(TagName::optgroup))
              {
                // TODO(HTMLTREEBUILDER, HTML): parse error
              }
            }

            InsertHTMLElement(Krys::Move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();
            _framesetOk = false;
            return;
          }
          case TagName::image:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error
            // NOTE: The spec says to treat this as a parse error and then reprocess the token as an img tag.
            token._name = u8"img";
            InBodyMode(Krys::Move(token));
            return;
          }
          case TagName::textarea:
          {
            InsertHTMLElement(Krys::Move(token));

            // TODO(HTMLTREEBUILDER, HTML): If the next token is a U+000A LINE FEED (LF) character token,
            // then ignore that token and move on to the next one. (Newlines at the start of textarea
            // elements are ignored as an authoring convenience.)

            _tokenizer.State(TokenizerState::RCDATA);
            _originalInsertionMode = _insertionMode;
            _framesetOk = false;
            _insertionMode = InsertionMode::Text;
            return;
          }
          case TagName::xmp:
          {
            if (HasElementInButtonScope(TagName::p))
            {
              ClosePElement();
            }

            _activeFormattingElements.Reconstruct(_openElementStack);
            _framesetOk = false;
            ParseGenericRawTextElement(Krys::Move(token));
            return;
          }
          case TagName::iframe:
          {
            _framesetOk = false;
            ParseGenericRawTextElement(Krys::Move(token));
            return;
          }
          case TagName::noembed:
          {
            ParseGenericRawTextElement(Krys::Move(token));
            return;
          }
          case TagName::noscript:
          {
            if (_scriptingMode != ParserScriptingMode::Disabled)
            {
              ParseGenericRawTextElement(Krys::Move(token));
              return;
            }

            // Scripting is disabled: treat as 'any other start tag'.
            _activeFormattingElements.Reconstruct(_openElementStack);
            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::select:
          {
            if (_scriptingMode == ParserScriptingMode::Fragment && _contextElement != nullptr
                && Is<HTMLSelectElement>(_contextElement))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }

            if (HasElementInScope(TagName::select))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              while (_openElementStack.Bottom().TagName() != TagName::select)
              {
                _openElementStack.Pop();
              }
              _openElementStack.Pop();
              return; // ignore the token
            }

            _activeFormattingElements.Reconstruct(_openElementStack);
            InsertHTMLElement(Krys::Move(token));
            _framesetOk = false;
            return;
          }
          case TagName::option:
          {
            if (HasElementInScope(TagName::select))
            {
              GenerateImpliedEndTags(TagName::optgroup);
              if (HasElementInScope(TagName::option))
              {
                // TODO(HTMLTREEBUILDER, HTML): parse error
              }
            }
            else
            {
              if (_openElementStack.Bottom().TagName() == TagName::option)
              {
                _openElementStack.Pop();
              }
            }

            _activeFormattingElements.Reconstruct(_openElementStack);
            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::optgroup:
          {
            if (HasElementInScope(TagName::select))
            {
              GenerateImpliedEndTags();
              if (HasElementInScope(TagName::option) || HasElementInScope(TagName::optgroup))
              {
                // TODO(HTMLTREEBUILDER, HTML): parse error
              }
            }
            else
            {
              if (_openElementStack.Bottom().TagName() == TagName::option)
              {
                _openElementStack.Pop();
              }
            }

            _activeFormattingElements.Reconstruct(_openElementStack);
            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::rb:
          case TagName::rtc:
          {
            if (HasElementInScope(TagName::ruby))
            {
              GenerateImpliedEndTags();
              if (_openElementStack.Bottom().TagName() != TagName::ruby)
              {
                // TODO(HTMLTREEBUILDER, HTML): parse error
              }
            }

            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::rp:
          case TagName::rt:
          {
            if (HasElementInScope(TagName::ruby))
            {
              GenerateImpliedEndTags(TagName::rtc);

              auto &rtCurrentNode = _openElementStack.Bottom();
              if (rtCurrentNode.TagName() != TagName::rtc && rtCurrentNode.TagName() != TagName::ruby)
              {
                // TODO(HTMLTREEBUILDER, HTML): parse error
              }
            }

            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::caption:
          case TagName::col:
          case TagName::colgroup:
          case TagName::frame:
          case TagName::head:
          case TagName::tbody:
          case TagName::td:
          case TagName::tfoot:
          case TagName::th:
          case TagName::thead:
          case TagName::tr:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error
            return; // ignore the token
          }
          default:
          {
            // TODO(HTMLTREEBUILDER, HTML): add math and svg to the TagName enum and handle them in the switch
            // statement above.
            auto nameView = token.Name().View();

            // math and svg require foreign element insertion; handle them here since they are not in
            // the TagName enum.
            if (nameView == u8"math")
            {
              _activeFormattingElements.Reconstruct(_openElementStack);
              AdjustMathMLAttributes(token);
              AdjustForeignAttributes(token);
              InsertForeignElement(Krys::Move(token), Namespaces::MathML, false);

              if (token.IsSelfClosing())
              {
                _openElementStack.Pop();
                token.AcknowledgeSelfClosingTag();
              }

              return;
            }

            if (nameView == u8"svg")
            {
              _activeFormattingElements.Reconstruct(_openElementStack);
              AdjustSVGAttributes(token);
              AdjustForeignAttributes(token);
              InsertForeignElement(Krys::Move(token), Namespaces::SVG, false);

              if (token.IsSelfClosing())
              {
                _openElementStack.Pop();
                token.AcknowledgeSelfClosingTag();
              }

              return;
            }

            // any other start tag

            _activeFormattingElements.Reconstruct(_openElementStack);
            InsertHTMLElement(Krys::Move(token));
            return;
          }
        }
      }
      case HTMLTokenType::EndTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::template_:
          {
            InHeadMode(Krys::Move(token));
            return;
          }
          case TagName::body:
          {
            if (!HasElementInScope(TagName::body))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }

            if (_openElementStack.ContainsInvalidUnclosedElements())
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
            }

            _insertionMode = InsertionMode::AfterBody;

            return;
          }
          case TagName::html:
          {
            if (!HasElementInScope(TagName::body))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }

            if (_openElementStack.ContainsInvalidUnclosedElements())
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
            }

            _insertionMode = InsertionMode::AfterBody;
            AfterBodyMode(Krys::Move(token)); // reprocess the token.

            return;
          }
          case TagName::address:
          case TagName::article:
          case TagName::aside:
          case TagName::blockquote:
          case TagName::center:
          case TagName::details:
          case TagName::dialog:
          case TagName::dir:
          case TagName::div:
          case TagName::dl:
          case TagName::fieldset:
          case TagName::figcaption:
          case TagName::figure:
          case TagName::footer:
          case TagName::header:
          case TagName::hgroup:
          case TagName::main:
          case TagName::menu:
          case TagName::nav:
          case TagName::ol:
          case TagName::search:
          case TagName::section:
          case TagName::summary:
          case TagName::ul:
          {
            if (!HasElementInScope(tagName))
            {
              return; // ignore the token
            }

            GenerateImpliedEndTags();

            if (_openElementStack.Bottom().TagName() != tagName)
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
            }

            while (_openElementStack.Bottom().TagName() != tagName)
            {
              _openElementStack.Pop();
            }
            _openElementStack.Pop();

            return;
          }
          case TagName::form:
          {
            if (!_openElementStack.ContainsTemplateElement())
            {
              RefPtr<HTMLFormElement> form = Krys::Move(_form);
              _form = nullptr;

              if (form == nullptr || !HasElementInScope(TagName::form))
              {
                // TODO(HTMLTREEBUILDER, HTML): parse error
                return; // ignore the token
              }

              GenerateImpliedEndTags();

              if (_openElementStack.Bottom().Node() != form.get())
              {
                // TODO(HTMLTREEBUILDER, HTML): parse error
              }

              _openElementStack.Remove(*form);
            }
            else
            {
              if (!HasElementInScope(TagName::form))
              {
                // TODO(HTMLTREEBUILDER, HTML): parse error
                return; // ignore the token
              }

              GenerateImpliedEndTags();

              if (_openElementStack.Bottom().TagName() != TagName::form)
              {
                // TODO(HTMLTREEBUILDER, HTML): parse error
              }

              while (_openElementStack.Bottom().TagName() != TagName::form)
              {
                _openElementStack.Pop();
              }
              _openElementStack.Pop();
            }

            return;
          }
          case TagName::p:
          {
            if (!HasElementInButtonScope(TagName::p))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              auto pElement = HTMLElementFactory::TryCreate(_document, TagName::p);
              assert(pElement != nullptr);
              InsertElementAtAdjustedInsertionLocation(*pElement);
              _openElementStack.Push({TagName::p, Namespace::HTML, *pElement, {}});
            }

            ClosePElement();
            return;
          }
          case TagName::li:
          {
            if (!HasElementInListItemScope(TagName::li))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }

            GenerateImpliedEndTags(TagName::li);

            if (_openElementStack.Bottom().TagName() != TagName::li)
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
            }

            while (_openElementStack.Bottom().TagName() != TagName::li)
            {
              _openElementStack.Pop();
            }
            _openElementStack.Pop();

            return;
          }
          case TagName::dd:
          case TagName::dt:
          {
            if (!HasElementInScope(tagName))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }

            GenerateImpliedEndTags(tagName);

            if (_openElementStack.Bottom().TagName() != tagName)
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
            }

            while (_openElementStack.Bottom().TagName() != tagName)
            {
              _openElementStack.Pop();
            }
            _openElementStack.Pop();

            return;
          }
          case TagName::h1:
          case TagName::h2:
          case TagName::h3:
          case TagName::h4:
          case TagName::h5:
          case TagName::h6:
          {
            if (!HasElementInScope(TagName::h1) && !HasElementInScope(TagName::h2)
                && !HasElementInScope(TagName::h3) && !HasElementInScope(TagName::h4)
                && !HasElementInScope(TagName::h5) && !HasElementInScope(TagName::h6))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }

            GenerateImpliedEndTags();

            if (_openElementStack.Bottom().TagName() != tagName)
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
            }

            while (true)
            {
              auto currentTagName = _openElementStack.Bottom().TagName();
              _openElementStack.Pop();

              if (currentTagName == TagName::h1 || currentTagName == TagName::h2
                  || currentTagName == TagName::h3 || currentTagName == TagName::h4
                  || currentTagName == TagName::h5 || currentTagName == TagName::h6)
              {
                break;
              }
            }

            return;
          }
          case TagName::a:
          case TagName::b:
          case TagName::big:
          case TagName::code:
          case TagName::em:
          case TagName::font:
          case TagName::i:
          case TagName::nobr:
          case TagName::s:
          case TagName::small:
          case TagName::strike:
          case TagName::strong:
          case TagName::tt:
          case TagName::u:
          {
            RunAdoptionAgency(token);
            return;
          }
          case TagName::br:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error

            // NOTE: We drop attributes from the token and act as if this were a "br" start tag with no
            // attributes.
            token._attributes.clear();
            _activeFormattingElements.Reconstruct(_openElementStack);
            InsertHTMLElement(Krys::Move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();

            _framesetOk = false;
            return;
          }
          case TagName::applet:
          case TagName::marquee:
          case TagName::object:
          {
            if (!HasElementInScope(tagName))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }

            GenerateImpliedEndTags();

            if (_openElementStack.Bottom().TagName() != tagName)
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
            }

            while (_openElementStack.Bottom().TagName() != tagName)
            {
              _openElementStack.Pop();
            }
            _openElementStack.Pop();

            _activeFormattingElements.ClearUpToLastMarker();

            return;
          }
          default:
          {
            // Any other end tag: walk the open element stack upward from the current node.
            auto *nodeEntry = &_openElementStack.Bottom();
            while (true)
            {
              if (Is<HTMLElement>(nodeEntry->Node()) && nodeEntry->TagName() == tagName)
              {
                // TODO(HTMLTREEBUILDER, HTML): the spec says to generate implied end tags for HTMLElements with the same tag name,
                // not just the same tag name. I don't think this is a problem, but it might be worth
                // checking.
                GenerateImpliedEndTags(tagName);

                if (nodeEntry != &_openElementStack.Bottom())
                {
                  // TODO(HTMLTREEBUILDER, HTML): parse error
                }

                auto &targetNode = nodeEntry->Node();
                while (true)
                {
                  bool isTarget = (&_openElementStack.Bottom().Node() == &targetNode);
                  _openElementStack.Pop();
                  if (isTarget)
                  {
                    break;
                  }
                }

                return;
              }

              if (IsSpecialElement(nodeEntry->TagName()))
              {
                // TODO(HTMLTREEBUILDER, HTML): parse error
                return; // ignore the token
              }

              nodeEntry = _openElementStack.EntryBefore(nodeEntry->Node());
              assert(nodeEntry != nullptr);
            }
          }
        }
      }
      case HTMLTokenType::EndOfFile:
      {
        if (!_templateInsertionModes.empty())
        {
          InTemplateMode(Krys::Move(token));
          return;
        }

        if (_openElementStack.ContainsInvalidUnclosedElements())
        {
          // TODO(HTMLTREEBUILDER, HTML): parse error
        }

        // TODO(HTMLTREEBUILDER, HTML): Stop parsing.
        return;
      }
    }
  }

  void HTMLTreeBuilder::TextMode(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InTableMode(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InTableTextMode(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InCaptionMode(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InColumnGroupMode(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InTableBodyMode(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InRowMode(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InCellMode(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InSelectMode(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InSelectInTableMode(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InTemplateMode(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::AfterBodyMode(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::InFramesetMode(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::AfterFramesetMode(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::AfterAfterBodyMode(HTMLTokenAtom &&token) noexcept
  {
  }

  void HTMLTreeBuilder::AfterAfterFramesetMode(HTMLTokenAtom &&token) noexcept
  {
  }

  bool HTMLTreeBuilder::IsQuirksModeDOCTYPE(const HTMLTokenAtom &token) const noexcept
  {
    assert(token.Type() == HTMLTokenType::DOCTYPE);

    // TODO(HTMLTREEBUILDER, HTML): return true if any of the following conditions are met:
    // The name is not "html".
    // The public identifier is set to: "-//W3O//DTD W3 HTML Strict 3.0//EN//"
    // The public identifier is set to: "-/W3C/DTD HTML 4.0 Transitional/EN"
    // The public identifier is set to: "HTML"
    // The system identifier is set to: "http://www.ibm.com/data/dtd/v11/ibmxhtml1-transitional.dtd"
    // The public identifier starts with: "+//Silmaril//dtd html Pro v0r11 19970101//"
    // The public identifier starts with: "-//AS//DTD HTML 3.0 asWedit + extensions//"
    // The public identifier starts with: "-//AdvaSoft Ltd//DTD HTML 3.0 asWedit + extensions//"
    // The public identifier starts with: "-//IETF//DTD HTML 2.0 Level 1//"
    // The public identifier starts with: "-//IETF//DTD HTML 2.0 Level 2//"
    // The public identifier starts with: "-//IETF//DTD HTML 2.0 Strict Level 1//"
    // The public identifier starts with: "-//IETF//DTD HTML 2.0 Strict Level 2//"
    // The public identifier starts with: "-//IETF//DTD HTML 2.0 Strict//"
    // The public identifier starts with: "-//IETF//DTD HTML 2.0//"
    // The public identifier starts with: "-//IETF//DTD HTML 2.1E//"
    // The public identifier starts with: "-//IETF//DTD HTML 3.0//"
    // The public identifier starts with: "-//IETF//DTD HTML 3.2 Final//"
    // The public identifier starts with: "-//IETF//DTD HTML 3.2//"
    // The public identifier starts with: "-//IETF//DTD HTML 3//"
    // The public identifier starts with: "-//IETF//DTD HTML Level 0//"
    // The public identifier starts with: "-//IETF//DTD HTML Level 1//"
    // The public identifier starts with: "-//IETF//DTD HTML Level 2//"
    // The public identifier starts with: "-//IETF//DTD HTML Level 3//"
    // The public identifier starts with: "-//IETF//DTD HTML Strict Level 0//"
    // The public identifier starts with: "-//IETF//DTD HTML Strict Level 1//"
    // The public identifier starts with: "-//IETF//DTD HTML Strict Level 2//"
    // The public identifier starts with: "-//IETF//DTD HTML Strict Level 3//"
    // The public identifier starts with: "-//IETF//DTD HTML Strict//"
    // The public identifier starts with: "-//IETF//DTD HTML//"
    // The public identifier starts with: "-//Metrius//DTD Metrius Presentational//"
    // The public identifier starts with: "-//Microsoft//DTD Internet Explorer 2.0 HTML Strict//"
    // The public identifier starts with: "-//Microsoft//DTD Internet Explorer 2.0 HTML//"
    // The public identifier starts with: "-//Microsoft//DTD Internet Explorer 2.0 Tables//"
    // The public identifier starts with: "-//Microsoft//DTD Internet Explorer 3.0 HTML Strict//"
    // The public identifier starts with: "-//Microsoft//DTD Internet Explorer 3.0 HTML//"
    // The public identifier starts with: "-//Microsoft//DTD Internet Explorer 3.0 Tables//"
    // The public identifier starts with: "-//Netscape Comm. Corp.//DTD HTML//"
    // The public identifier starts with: "-//Netscape Comm. Corp.//DTD Strict HTML//"
    // The public identifier starts with: "-//O'Reilly and Associates//DTD HTML 2.0//"
    // The public identifier starts with: "-//O'Reilly and Associates//DTD HTML Extended 1.0//"
    // The public identifier starts with: "-//O'Reilly and Associates//DTD HTML Extended Relaxed 1.0//"
    // The public identifier starts with: "-//SQ//DTD HTML 2.0 HoTMetaL + extensions//"
    // The public identifier starts with: "-//SoftQuad Software//DTD HoTMetaL PRO 6.0::19990601::extensions to
    // HTML 4.0//"
    // The public identifier starts with: "-//SoftQuad//DTD HoTMetaL PRO 4.0::19971010::extensions to
    // HTML 4.0//"
    // The public identifier starts with: "-//Spyglass//DTD HTML 2.0 Extended//"
    // The public identifier starts with: "-//Sun Microsystems Corp.//DTD HotJava HTML//"
    // The public identifier starts with: "-//Sun Microsystems Corp.//DTD HotJava Strict HTML//"
    // The public identifier starts with: "-//W3C//DTD HTML 3 1995-03-24//"
    // The public identifier starts with: "-//W3C//DTD HTML 3.2 Draft//"
    // The public identifier starts with: "-//W3C//DTD HTML 3.2 Final//"
    // The public identifier starts with: "-//W3C//DTD HTML 3.2//"
    // The public identifier starts with: "-//W3C//DTD HTML 3.2S Draft//"
    // The public identifier starts with: "-//W3C//DTD HTML 4.0 Frameset//"
    // The public identifier starts with: "-//W3C//DTD HTML 4.0 Transitional//"
    // The public identifier starts with: "-//W3C//DTD HTML Experimental 19960712//"
    // The public identifier starts with: "-//W3C//DTD HTML Experimental 970421//"
    // The public identifier starts with: "-//W3C//DTD W3 HTML//"
    // The public identifier starts with: "-//W3O//DTD W3 HTML 3.0//"
    // The public identifier starts with: "-//WebTechs//DTD Mozilla HTML 2.0//"
    // The public identifier starts with: "-//WebTechs//DTD Mozilla HTML//"
    // The system identifier is missing or the empty string, and the public identifier starts with:
    // "-//W3C//DTD HTML 4.01 Frameset//"
    // The system identifier is missing or the empty string, and the public identifier starts with:
    // "-//W3C//DTD HTML 4.01 Transitional//"

    return token.IsForceQuirks();
  }

  bool HTMLTreeBuilder::IsLimitedQuirksModeDOCTYPE(const HTMLTokenAtom &token) const noexcept
  {
    // TODO(HTMLTREEBUILDER, HTML): return true if any of the following conditions are met:
    // The public identifier starts with: "-//W3C//DTD XHTML 1.0 Frameset//"
    // The public identifier starts with: "-//W3C//DTD XHTML 1.0 Transitional//"
    // The system identifier is neither missing nor the empty string, and the public identifier starts with:
    // "-//W3C//DTD HTML 4.01 Frameset//"
    // The system identifier is neither missing nor the empty string, and the public identifier starts with:
    // "-//W3C//DTD HTML 4.01 Transitional//"

    return false;
  }

  bool HTMLTreeBuilder::SkipCharacterTokenWhitespace(HTMLTokenAtom &token) noexcept
  {
    assert(token.Type() == HTMLTokenType::Character);

    auto position = token.Data().begin();
    StringAlgorithms::SkipWhitespace(token.Data(), position);

    if (position == token.Data().end())
    {
      return false;
    }

    token._data = DOMString(position, token.Data().end());
    return true;
  }

  bool HTMLTreeBuilder::InsertCharacterTokenWhitespace(HTMLTokenAtom &token) noexcept
  {
    assert(token.Type() == HTMLTokenType::Character);

    auto position = token.Data().begin();
    StringAlgorithms::SkipWhitespace(token.Data(), position);

    if (position == token.Data().begin())
    {
      return !token.Data().empty();
    }

    InsertCharacter(DOMString(token.Data().begin(), position));

    if (position == token.Data().end())
    {
      return false;
    }

    token._data = DOMString(position, token.Data().end());
    return true;
  }

  void HTMLTreeBuilder::ClosePElement() noexcept
  {
    GenerateImpliedEndTags(TagName::p);

    if (_openElementStack.Bottom().TagName() != TagName::p)
    {
      // TODO(HTMLTREEBUILDER, HTML): parse error
    }

    while (_openElementStack.Bottom().TagName() != TagName::p)
    {
      _openElementStack.Pop();
    }
    _openElementStack.Pop();
  }

  void HTMLTreeBuilder::RunAdoptionAgency(HTMLTokenAtom &token) noexcept
  {
    // TODO(HTMLTREEBUILDER, HTML): Implement the adoption agency algorithm.
    // https://html.spec.whatwg.org/multipage/parsing.html#adoption-agency-algorithm
    (void)token;
  }

#pragma endregion
}
