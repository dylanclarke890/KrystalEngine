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

    auto html = ElementFactory::Create(_document, QualifiedName(Namespaces::HTML, DOMStringAtom::Null(),
                                                                u8"html", TagName::html, Namespace::HTML));
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

    _head = ElementFactory::Create(_document, QualifiedName(Namespaces::HTML, DOMStringAtom::Null(), u8"head",
                                                            TagName::head, Namespace::HTML));
    InsertElementAtAdjustedInsertionLocation(*_head);
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
        }
        break;
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

    auto body = ElementFactory::Create(_document, QualifiedName(Namespaces::HTML, DOMStringAtom::Null(),
                                                                u8"body", TagName::body, Namespace::HTML));
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

              _openElementStack.PopUntilPopped(TagName::li);

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

              _openElementStack.PopUntilPopped(TagName::dd);

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

              _openElementStack.PopUntilPopped(TagName::dt);

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

              _openElementStack.PopUntilPopped(TagName::button);
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
              _openElementStack.PopUntilPopped(TagName::select);
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
              _openElementStack.PopUntilPopped(TagName::select);
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

            _openElementStack.PopUntilPopped(tagName);
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

              _openElementStack.PopUntilPopped(TagName::form);
            }

            return;
          }
          case TagName::p:
          {
            if (!HasElementInButtonScope(TagName::p))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              auto pElement =
                ElementFactory::Create(_document, QualifiedName(Namespaces::HTML, DOMStringAtom::Null(),
                                                                u8"p", TagName::p, Namespace::HTML));
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

            _openElementStack.PopUntilPopped(TagName::li);
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

            _openElementStack.PopUntilPopped(tagName);
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

            _openElementStack.PopUntilPopped(tagName);
            _activeFormattingElements.ClearUpToLastMarker();

            return;
          }
          default:
          {
            InBodyGenericEndTag(tagName);
            return;
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

  void HTMLTreeBuilder::InBodyGenericEndTag(TagName tagName)
  {
    // walk the open element stack upward from the current node.
    auto *nodeEntry = &_openElementStack.Bottom();
    while (true)
    {
      if (Is<HTMLElement>(nodeEntry->Node()) && nodeEntry->TagName() == tagName)
      {
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

  void HTMLTreeBuilder::TextMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        InsertCharacter(DOMString(token.Data()));
        return;
      }
      case HTMLTokenType::EndOfFile:
      {
        // TODO(HTMLTREEBUILDER, HTML): parse error

        if (_openElementStack.Bottom().TagName() == TagName::script)
        {
          auto &scriptElement = Downcast<HTMLScriptElement>(_openElementStack.Bottom().Node());
          scriptElement._alreadyStarted = true;
          _openElementStack.Pop();
        }

        _insertionMode = _originalInsertionMode;
        ProcessToken(Krys::Move(token));
        return;
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name().View() == u8"script")
        {
          // TODO(HTMLTREEBUILDER, HTML): HTMLScriptElement end tag handling
          _openElementStack.Pop();
          _insertionMode = _originalInsertionMode;
        }
        else
        {
          _openElementStack.Pop();
          _insertionMode = _originalInsertionMode;
        }
        return;
      }
    }
  }

  void HTMLTreeBuilder::InTableMode(HTMLTokenAtom &&token) noexcept
  {
    auto clearStackBackToTableContext = [&]()
    {
      while (true)
      {
        auto tagName = _openElementStack.Bottom().TagName();
        if (tagName == TagName::table || tagName == TagName::template_ || tagName == TagName::html)
        {
          break;
        }
        _openElementStack.Pop();
      }
    };

    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        switch (_openElementStack.Bottom().TagName())
        {
          case TagName::table:
          case TagName::tbody:
          case TagName::template_:
          case TagName::tfoot:
          case TagName::thead:
          case TagName::tr:
          {
            _pendingTableCharacterTokens.clear();
            _originalInsertionMode = _insertionMode;
            _insertionMode = InsertionMode::InTableText;
            ProcessToken(Krys::Move(token));
            return;
          }
          default: break;
        }
        break; // fall through to "anything else"
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
          case TagName::caption:
          {
            clearStackBackToTableContext();
            _activeFormattingElements.PushMarker();
            InsertHTMLElement(Krys::Move(token));
            _insertionMode = InsertionMode::InCaption;
            return;
          }
          case TagName::colgroup:
          {
            clearStackBackToTableContext();
            InsertHTMLElement(Krys::Move(token));
            _insertionMode = InsertionMode::InColumnGroup;
            return;
          }
          case TagName::col:
          {
            clearStackBackToTableContext();
            auto colgroup = ElementFactory::Create(
              _document, QualifiedName(Namespaces::HTML, DOMStringAtom::Null(), u8"colgroup",
                                       TagName::colgroup, Namespace::HTML));
            assert(colgroup != nullptr);
            InsertElementAtAdjustedInsertionLocation(*colgroup);
            _openElementStack.Push({TagName::colgroup, Namespace::HTML, *colgroup, {}});
            _insertionMode = InsertionMode::InColumnGroup;
            ProcessToken(Krys::Move(token));
            return;
          }
          case TagName::tbody:
          case TagName::tfoot:
          case TagName::thead:
          {
            clearStackBackToTableContext();
            InsertHTMLElement(Krys::Move(token));
            _insertionMode = InsertionMode::InTableBody;
            return;
          }
          case TagName::td:
          case TagName::th:
          case TagName::tr:
          {
            clearStackBackToTableContext();
            auto tbody =
              ElementFactory::Create(_document, QualifiedName(Namespaces::HTML, DOMStringAtom::Null(),
                                                              u8"tbody", TagName::tbody, Namespace::HTML));
            assert(tbody != nullptr);
            InsertElementAtAdjustedInsertionLocation(*tbody);
            _openElementStack.Push({TagName::tbody, Namespace::HTML, *tbody, {}});
            _insertionMode = InsertionMode::InTableBody;
            ProcessToken(Krys::Move(token));
            return;
          }
          case TagName::table:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error
            if (!HasElementInTableScope(TagName::table))
            {
              return; // ignore the token
            }

            _openElementStack.PopUntilPopped(TagName::table);
            ResetInsertionModeAppropriately();
            ProcessToken(Krys::Move(token));
            return;
          }
          case TagName::style:
          case TagName::script:
          case TagName::template_:
          {
            InHeadMode(Krys::Move(token));
            return;
          }
          case TagName::input:
          {
            auto typeIt = std::ranges::find_if(token.Attributes(), [](const ParsedAttribute &attr)
                                               { return attr.NameView() == u8"type"; });
            if (typeIt == token.Attributes().end()
                || !StringAlgorithms::ASCIICaseInsensitiveMatch(typeIt->ValueView(), u8"hidden"))
            {
              break; // fall through to "anything else"
            }
            // TODO(HTMLTREEBUILDER, HTML): parse error
            InsertHTMLElement(Krys::Move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();
            return;
          }
          case TagName::form:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error
            if (_openElementStack.ContainsTemplateElement() || _form != nullptr)
            {
              return; // ignore the token
            }
            auto element = InsertHTMLElement(Krys::Move(token));
            _form = Krys::Move(element);
            _openElementStack.Pop();
            return;
          }
          default:
          {
            break;
          }
        }
        break; // fall through to "anything else"
      }
      case HTMLTokenType::EndTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::table:
          {
            if (!HasElementInTableScope(TagName::table))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }

            _openElementStack.PopUntilPopped(TagName::table);
            ResetInsertionModeAppropriately();

            return;
          }
          case TagName::body:
          case TagName::caption:
          case TagName::col:
          case TagName::colgroup:
          case TagName::html:
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
          case TagName::template_:
          {
            InHeadMode(Krys::Move(token));
            return;
          }
          default: break;
        }
        break; // fall through to "anything else"
      }
      case HTMLTokenType::EndOfFile:
      {
        InBodyMode(Krys::Move(token));
        return;
      }
      default: break;
    }

    // Anything else: parse error, enable foster parenting, process with in body, then disable
    // TODO(HTMLTREEBUILDER, HTML): parse error
    _fosterParenting = true;
    InBodyMode(Krys::Move(token));
    _fosterParenting = false;
  }

  void HTMLTreeBuilder::InTableTextMode(HTMLTokenAtom &&token) noexcept
  {
    if (token.Type() == HTMLTokenType::Character)
    {
      // A character token that is U+0000 NULL: parse error, ignore.
      // TODO(HTMLTREEBUILDER, HTML): this only checks the first character of the token, but the spec says to
      // check each character. This is a bug.
      if (!token.Data().empty() && token.Data().front() == u8'\0')
      {
        // TODO(HTMLTREEBUILDER, HTML): parse error
        return;
      }
      _pendingTableCharacterTokens.emplace_back(token.Data());
      return;
    }

    // Anything else: check if any pending token contains non-ASCII-whitespace.
    bool hasNonWhitespace = false;
    for (const auto &chars : _pendingTableCharacterTokens)
    {
      if (hasNonWhitespace =
            std::ranges::any_of(chars, [](char8 ch) { return !StringAlgorithms::IsASCIIWhitespace(ch); }))
      {
        break;
      }
    }

    if (hasNonWhitespace)
    {
      // TODO(HTMLTREEBUILDER, HTML): parse error
      // Reprocess each pending character token using the "anything else" entry in the "in table" mode.
      for (auto &chars : _pendingTableCharacterTokens)
      {
        _fosterParenting = true;
        InsertCharacter(DOMString(chars));
        _fosterParenting = false;
      }
    }
    else
    {
      for (auto &chars : _pendingTableCharacterTokens)
        InsertCharacter(DOMString(chars));
    }

    _pendingTableCharacterTokens.clear();
    _insertionMode = _originalInsertionMode;
    ProcessToken(Krys::Move(token));
  }

  void HTMLTreeBuilder::InCaptionMode(HTMLTokenAtom &&token) noexcept
  {
    // Shared logic: close the caption and optionally reprocess the token.
    auto closeCaption = [&](bool reprocess) -> bool
    {
      if (!HasElementInTableScope(TagName::caption))
      {
        // TODO(HTMLTREEBUILDER, HTML): parse error
        return false; // fragment case — ignore
      }
      GenerateImpliedEndTags();
      if (_openElementStack.Bottom().TagName() != TagName::caption)
      {
        // TODO(HTMLTREEBUILDER, HTML): parse error
      }

      _openElementStack.PopUntilPopped(TagName::caption);
      _activeFormattingElements.ClearUpToLastMarker();
      _insertionMode = InsertionMode::InTable;
      return true;
    };

    if (token.Type() == HTMLTokenType::EndTag && token.Name() == u8"caption")
    {
      closeCaption(false);
      return;
    }

    if (token.Type() == HTMLTokenType::EndTag && token.Name() == u8"table")
    {
      if (closeCaption(false))
      {
        ProcessToken(Krys::Move(token));
      }
      return;
    }

    if (token.Type() == HTMLTokenType::StartTag)
    {
      auto tagName = ParseTagName(token.Name().View());
      switch (tagName)
      {
        case TagName::caption:
        case TagName::col:
        case TagName::colgroup:
        case TagName::tbody:
        case TagName::td:
        case TagName::tfoot:
        case TagName::th:
        case TagName::thead:
        case TagName::tr:
        {
          if (closeCaption(false))
          {
            ProcessToken(Krys::Move(token));
          }
          return;
        }
        default: break;
      }
    }

    if (token.Type() == HTMLTokenType::EndTag)
    {
      auto tagName = ParseTagName(token.Name().View());
      switch (tagName)
      {
        case TagName::body:
        case TagName::col:
        case TagName::colgroup:
        case TagName::html:
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
        default: break;
      }
    }

    InBodyMode(Krys::Move(token));
  }

  void HTMLTreeBuilder::InColumnGroupMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        if (InsertCharacterTokenWhitespace(token))
        {
          break; // has non-whitespace remaining — fall through to "anything else"
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
          case TagName::col:
          {
            InsertHTMLElement(Krys::Move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();
            return;
          }
          case TagName::template_:
          {
            InHeadMode(Krys::Move(token));
            return;
          }
          default: break;
        }
        break; // fall through to "anything else"
      }
      case HTMLTokenType::EndTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::colgroup:
          {
            if (_openElementStack.Bottom().TagName() != TagName::colgroup)
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }
            _openElementStack.Pop();
            _insertionMode = InsertionMode::InTable;
            return;
          }
          case TagName::col:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error
            return; // ignore the token
          }
          case TagName::template_:
          {
            InHeadMode(Krys::Move(token));
            return;
          }
          default: break;
        }
        break; // fall through to "anything else"
      }
      case HTMLTokenType::EndOfFile:
      {
        InBodyMode(Krys::Move(token));
        return;
      }
      default: break;
    }

    // Anything else: if current node is not a colgroup element, parse error and ignore.
    if (_openElementStack.Bottom().TagName() != TagName::colgroup)
    {
      // TODO(HTMLTREEBUILDER, HTML): parse error
      return; // ignore the token
    }
    _openElementStack.Pop();
    _insertionMode = InsertionMode::InTable;
    ProcessToken(Krys::Move(token));
  }

  void HTMLTreeBuilder::InTableBodyMode(HTMLTokenAtom &&token) noexcept
  {
    auto clearStackBackToTableBodyContext = [&]()
    {
      while (true)
      {
        auto tagName = _openElementStack.Bottom().TagName();
        if (tagName == TagName::tbody || tagName == TagName::tfoot || tagName == TagName::thead
            || tagName == TagName::template_ || tagName == TagName::html)
          break;
        _openElementStack.Pop();
      }
    };

    switch (token.Type())
    {
      case HTMLTokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::tr:
          {
            clearStackBackToTableBodyContext();
            InsertHTMLElement(Krys::Move(token));
            _insertionMode = InsertionMode::InRow;
            return;
          }
          case TagName::th:
          case TagName::td:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error
            clearStackBackToTableBodyContext();
            auto tr = ElementFactory::Create(_document, QualifiedName(Namespaces::HTML, DOMStringAtom::Null(),
                                                                      u8"tr", TagName::tr, Namespace::HTML));
            assert(tr != nullptr);
            InsertElementAtAdjustedInsertionLocation(*tr);
            _openElementStack.Push({TagName::tr, Namespace::HTML, *tr, {}});
            _insertionMode = InsertionMode::InRow;
            ProcessToken(Krys::Move(token));
            return;
          }
          case TagName::caption:
          case TagName::col:
          case TagName::colgroup:
          case TagName::tbody:
          case TagName::tfoot:
          case TagName::thead:
          {
            if (!HasElementInTableScope(TagName::tbody) && !HasElementInTableScope(TagName::thead)
                && !HasElementInTableScope(TagName::tfoot))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }
            clearStackBackToTableBodyContext();
            _openElementStack.Pop();
            _insertionMode = InsertionMode::InTable;
            ProcessToken(Krys::Move(token));
            return;
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
          case TagName::tbody:
          case TagName::tfoot:
          case TagName::thead:
          {
            if (!HasElementInTableScope(tagName))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }
            clearStackBackToTableBodyContext();
            _openElementStack.Pop();
            _insertionMode = InsertionMode::InTable;
            return;
          }
          case TagName::table:
          {
            if (!HasElementInTableScope(TagName::tbody) && !HasElementInTableScope(TagName::thead)
                && !HasElementInTableScope(TagName::tfoot))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }
            clearStackBackToTableBodyContext();
            _openElementStack.Pop();
            _insertionMode = InsertionMode::InTable;
            ProcessToken(Krys::Move(token));
            return;
          }
          case TagName::body:
          case TagName::caption:
          case TagName::col:
          case TagName::colgroup:
          case TagName::html:
          case TagName::td:
          case TagName::th:
          case TagName::tr:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error
            return; // ignore the token
          }
          default: break;
        }
        break;
      }
      default: break;
    }

    InTableMode(Krys::Move(token));
  }

  void HTMLTreeBuilder::InRowMode(HTMLTokenAtom &&token) noexcept
  {
    auto clearStackBackToTableRowContext = [&]()
    {
      while (true)
      {
        auto tagName = _openElementStack.Bottom().TagName();
        if (tagName == TagName::tr || tagName == TagName::template_ || tagName == TagName::html)
          break;
        _openElementStack.Pop();
      }
    };

    switch (token.Type())
    {
      case HTMLTokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::th:
          case TagName::td:
          {
            clearStackBackToTableRowContext();
            InsertHTMLElement(Krys::Move(token));
            _insertionMode = InsertionMode::InCell;
            _activeFormattingElements.PushMarker();
            return;
          }
          case TagName::caption:
          case TagName::col:
          case TagName::colgroup:
          case TagName::tbody:
          case TagName::tfoot:
          case TagName::thead:
          case TagName::tr:
          {
            if (!HasElementInTableScope(TagName::tr))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }
            clearStackBackToTableRowContext();
            _openElementStack.Pop();
            _insertionMode = InsertionMode::InTableBody;
            ProcessToken(Krys::Move(token));
            return;
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
          case TagName::tr:
          {
            if (!HasElementInTableScope(TagName::tr))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }
            clearStackBackToTableRowContext();
            _openElementStack.Pop();
            _insertionMode = InsertionMode::InTableBody;
            return;
          }
          case TagName::table:
          {
            if (!HasElementInTableScope(TagName::tr))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }
            clearStackBackToTableRowContext();
            _openElementStack.Pop();
            _insertionMode = InsertionMode::InTableBody;
            ProcessToken(Krys::Move(token));
            return;
          }
          case TagName::tbody:
          case TagName::tfoot:
          case TagName::thead:
          {
            if (!HasElementInTableScope(tagName))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }
            if (!HasElementInTableScope(TagName::tr))
              return; // ignore the token
            clearStackBackToTableRowContext();
            _openElementStack.Pop();
            _insertionMode = InsertionMode::InTableBody;
            ProcessToken(Krys::Move(token));
            return;
          }
          case TagName::body:
          case TagName::caption:
          case TagName::col:
          case TagName::colgroup:
          case TagName::html:
          case TagName::td:
          case TagName::th:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error
            return; // ignore the token
          }
          default: break;
        }
        break;
      }
      default: break;
    }

    InTableMode(Krys::Move(token));
  }

  void HTMLTreeBuilder::InCellMode(HTMLTokenAtom &&token) noexcept
  {
    auto closeTheCell = [&]()
    {
      GenerateImpliedEndTags();
      auto &current = _openElementStack.Bottom();
      if (current.TagName() != TagName::td && current.TagName() != TagName::th)
      {
        // TODO(HTMLTREEBUILDER, HTML): parse error
      }
      while (_openElementStack.Bottom().TagName() != TagName::td
             && _openElementStack.Bottom().TagName() != TagName::th)
      {
        _openElementStack.Pop();
      }
      _openElementStack.Pop();
      _activeFormattingElements.ClearUpToLastMarker();
      _insertionMode = InsertionMode::InRow;
    };

    switch (token.Type())
    {
      case HTMLTokenType::EndTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::td:
          case TagName::th:
          {
            if (!HasElementInTableScope(tagName))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }

            GenerateImpliedEndTags();

            if (_openElementStack.Bottom().TagName() != tagName)
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
            }
            _openElementStack.PopUntilPopped(tagName);

            _activeFormattingElements.ClearUpToLastMarker();
            _insertionMode = InsertionMode::InRow;

            return;
          }
          case TagName::body:
          case TagName::caption:
          case TagName::col:
          case TagName::colgroup:
          case TagName::html:
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error
            return; // ignore the token
          }
          case TagName::table:
          case TagName::tbody:
          case TagName::tfoot:
          case TagName::thead:
          case TagName::tr:
          {
            if (!HasElementInTableScope(tagName))
            {
              // TODO(HTMLTREEBUILDER, HTML): parse error
              return; // ignore the token
            }
            closeTheCell();
            ProcessToken(Krys::Move(token));
            return;
          }
          default: break;
        }
        break;
      }
      case HTMLTokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::caption:
          case TagName::col:
          case TagName::colgroup:
          case TagName::tbody:
          case TagName::td:
          case TagName::tfoot:
          case TagName::th:
          case TagName::thead:
          case TagName::tr:
          {
            // Assert: the stack has a td or th in table scope.
            closeTheCell();
            ProcessToken(Krys::Move(token));
            return;
          }
          default: break;
        }
        break;
      }
      default: break;
    }

    InBodyMode(Krys::Move(token));
  }

  void HTMLTreeBuilder::InTemplateMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      case HTMLTokenType::Comment:
      case HTMLTokenType::DOCTYPE:
      {
        InBodyMode(Krys::Move(token));
        return;
      }
      case HTMLTokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
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
          case TagName::caption:
          case TagName::colgroup:
          case TagName::tbody:
          case TagName::tfoot:
          case TagName::thead:
          {
            _templateInsertionModes.pop_back();
            _templateInsertionModes.push_back(InsertionMode::InTable);
            _insertionMode = InsertionMode::InTable;
            ProcessToken(Krys::Move(token));
            return;
          }
          case TagName::col:
          {
            _templateInsertionModes.pop_back();
            _templateInsertionModes.push_back(InsertionMode::InColumnGroup);
            _insertionMode = InsertionMode::InColumnGroup;
            ProcessToken(Krys::Move(token));
            return;
          }
          case TagName::tr:
          {
            _templateInsertionModes.pop_back();
            _templateInsertionModes.push_back(InsertionMode::InTableBody);
            _insertionMode = InsertionMode::InTableBody;
            ProcessToken(Krys::Move(token));
            return;
          }
          case TagName::td:
          case TagName::th:
          {
            _templateInsertionModes.pop_back();
            _templateInsertionModes.push_back(InsertionMode::InRow);
            _insertionMode = InsertionMode::InRow;
            ProcessToken(Krys::Move(token));
            return;
          }
          default:
          {
            _templateInsertionModes.pop_back();
            _templateInsertionModes.push_back(InsertionMode::InBody);
            _insertionMode = InsertionMode::InBody;
            ProcessToken(Krys::Move(token));
            return;
          }
        }
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == u8"template")
        {
          InHeadMode(Krys::Move(token));
          return;
        }
        // TODO(HTMLTREEBUILDER, HTML): parse error
        return; // ignore the token
      }
      case HTMLTokenType::EndOfFile:
      {
        if (!_openElementStack.ContainsTemplateElement())
        {
          // TODO(HTMLTREEBUILDER, HTML): Stop parsing. (fragment case)
          return;
        }
        // TODO(HTMLTREEBUILDER, HTML): parse error
        _openElementStack.PopUntilPopped(TagName::template_);
        _activeFormattingElements.ClearUpToLastMarker();
        _templateInsertionModes.pop_back();
        ResetInsertionModeAppropriately();
        ProcessToken(Krys::Move(token));
        return;
      }
      default: break;
    }
  }

  void HTMLTreeBuilder::AfterBodyMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        if (!InsertCharacterTokenWhitespace(token))
          return;
        // Non-whitespace remains — fall through to "anything else"
        break;
      }
      case HTMLTokenType::Comment:
      {
        // Insert as the last child of the html element (first element on the stack).
        InsertComment(DOMString(token.Data()),
                      AdjustedInsertionLocation {&_openElementStack.Top().Node(), nullptr});
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
          InBodyMode(Krys::Move(token));
          return;
        }
        break; // fall through to "anything else"
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == u8"html")
        {
          if (_contextElement != nullptr)
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error — fragment case, ignore
            return;
          }
          _insertionMode = InsertionMode::AfterAfterBody;
          return;
        }
        break; // fall through to "anything else"
      }
      case HTMLTokenType::EndOfFile:
      {
        // TODO(HTMLTREEBUILDER, HTML): Stop parsing.
        return;
      }
      default: break;
    }

    // Anything else: parse error, switch to "in body", reprocess.
    // TODO(HTMLTREEBUILDER, HTML): parse error
    _insertionMode = InsertionMode::InBody;
    ProcessToken(Krys::Move(token));
  }

  void HTMLTreeBuilder::InFramesetMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        if (InsertCharacterTokenWhitespace(token))
          break; // non-whitespace remaining — fall through to "anything else"
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
          case TagName::frameset:
          {
            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::frame:
          {
            InsertHTMLElement(Krys::Move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();
            return;
          }
          case TagName::noframes:
          {
            InHeadMode(Krys::Move(token));
            return;
          }
          default: break;
        }
        break; // fall through to "anything else"
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == u8"frameset")
        {
          if (&_openElementStack.Bottom().Node() == &_openElementStack.Top().Node())
          {
            // TODO(HTMLTREEBUILDER, HTML): parse error — fragment case, ignore
            return;
          }
          _openElementStack.Pop();
          if (_contextElement == nullptr && _openElementStack.Bottom().TagName() != TagName::frameset)
          {
            _insertionMode = InsertionMode::AfterFrameset;
          }
          return;
        }
        break; // fall through to "anything else"
      }
      case HTMLTokenType::EndOfFile:
      {
        if (&_openElementStack.Bottom().Node() != &_openElementStack.Top().Node())
        {
          // TODO(HTMLTREEBUILDER, HTML): parse error
        }
        // TODO(HTMLTREEBUILDER, HTML): Stop parsing.
        return;
      }
      default: break;
    }

    // Anything else: parse error, ignore.
    // TODO(HTMLTREEBUILDER, HTML): parse error
  }

  void HTMLTreeBuilder::AfterFramesetMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        if (InsertCharacterTokenWhitespace(token))
          break; // non-whitespace remaining — fall through to "anything else"
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
          case TagName::noframes:
          {
            InHeadMode(Krys::Move(token));
            return;
          }
          default: break;
        }
        break; // fall through to "anything else"
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == u8"html")
        {
          _insertionMode = InsertionMode::AfterAfterFrameset;
          return;
        }
        break; // fall through to "anything else"
      }
      case HTMLTokenType::EndOfFile:
      {
        // TODO(HTMLTREEBUILDER, HTML): Stop parsing.
        return;
      }
      default: break;
    }

    // Anything else: parse error, ignore.
    // TODO(HTMLTREEBUILDER, HTML): parse error
  }

  void HTMLTreeBuilder::AfterAfterBodyMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Comment:
      {
        InsertComment(DOMString(token.Data()), AdjustedInsertionLocation {&_document, nullptr});
        return;
      }
      case HTMLTokenType::DOCTYPE:
      case HTMLTokenType::Character:
      {
        // DOCTYPE and whitespace characters: process using the rules for "in body".
        if (token.Type() == HTMLTokenType::Character)
        {
          if (!InsertCharacterTokenWhitespace(token))
            return;
          // Non-whitespace remains — fall through to "anything else"
          break;
        }
        InBodyMode(Krys::Move(token));
        return;
      }
      case HTMLTokenType::StartTag:
      {
        if (token.Name() == u8"html")
        {
          InBodyMode(Krys::Move(token));
          return;
        }
        break; // fall through to "anything else"
      }
      case HTMLTokenType::EndOfFile:
      {
        // TODO(HTMLTREEBUILDER, HTML): Stop parsing.
        return;
      }
      default: break;
    }

    // Anything else: parse error, switch to "in body", reprocess.
    // TODO(HTMLTREEBUILDER, HTML): parse error
    _insertionMode = InsertionMode::InBody;
    ProcessToken(Krys::Move(token));
  }

  void HTMLTreeBuilder::AfterAfterFramesetMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Comment:
      {
        InsertComment(DOMString(token.Data()), AdjustedInsertionLocation {&_document, nullptr});
        return;
      }
      case HTMLTokenType::DOCTYPE:
      case HTMLTokenType::Character:
      {
        if (token.Type() == HTMLTokenType::Character)
        {
          if (!InsertCharacterTokenWhitespace(token))
            return;
          // Non-whitespace remains — fall through to "anything else"
          break;
        }
        InBodyMode(Krys::Move(token));
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
          case TagName::noframes:
          {
            InHeadMode(Krys::Move(token));
            return;
          }
          default: break;
        }
        break; // fall through to "anything else"
      }
      case HTMLTokenType::EndOfFile:
      {
        // TODO(HTMLTREEBUILDER, HTML): Stop parsing.
        return;
      }
      default: break;
    }

    // Anything else: parse error, ignore.
    // TODO(HTMLTREEBUILDER, HTML): parse error
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

    token._data = DOMStringView(position, token.Data().end());
    return true;
  }

  bool HTMLTreeBuilder::InsertCharacterTokenWhitespace(HTMLTokenAtom &token) noexcept
  {
    assert(token.Type() == HTMLTokenType::Character);

    auto position = token.Data().begin();
    StringAlgorithms::SkipWhitespace(token.Data(), position);

    if (position == token.Data().begin())
    {
      auto data = token.Data();
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
    _openElementStack.PopUntilPopped(TagName::p);
  }

  void HTMLTreeBuilder::RunAdoptionAgency(HTMLTokenAtom &token) noexcept
  {
    auto subject = ParseTagName(token.Name().View());

    auto &bottom = _openElementStack.Bottom();
    if (Is<HTMLElement>(bottom.Node()) && bottom.TagName() == subject
        && !_activeFormattingElements.ContainsFormattingElement(bottom.Node()))
    {
      _openElementStack.Pop();
      return;
    }

    size_t outerLoopCounter = 0uz;
    while (true)
    {
      if (outerLoopCounter >= 8uz)
      {
        return;
      }

      outerLoopCounter++;

      auto *formattingElement = _activeFormattingElements.FindFormattingElementFromLastMarker(subject);
      if (formattingElement == nullptr)
      {
        InBodyGenericEndTag(subject);
        return;
      }

      if (!_openElementStack.ContainsElement(formattingElement->Node()))
      {
        // TODO(HTMLTREEBUILDER, HTML): parse error
        _activeFormattingElements.RemoveFormattingElement(formattingElement->Node());
        return;
      }

      if (!HasElementInScope(formattingElement->TagName()))
      {
        // TODO(HTMLTREEBUILDER, HTML): parse error
        return;
      }

      if (&_openElementStack.Bottom() != formattingElement)
      {
        // TODO(HTMLTREEBUILDER, HTML): parse error
        // NOTE: do not return; continue with the algorithm.
      }

      auto *furthestBlock = FurthestSpecialElementBlock(*formattingElement);
      if (furthestBlock == nullptr)
      {
        _openElementStack.PopUntilPopped(formattingElement->Node());
        _activeFormattingElements.RemoveFormattingElement(formattingElement->Node());
        return;
      }

      auto *commonAncestor = _openElementStack.EntryBefore(formattingElement->Node());
      assert(commonAncestor != nullptr);

      // TODO(HTMLTREEBUILDER, HTML): Check the rest of the logic for the AdoptionAgency algorithm.

      // Step 4.10: Bookmark positioned at the formatting element in the active formatting list.
      auto bookmark = _activeFormattingElements.BookmarkFor(formattingElement->Node());

      // Step 4.11: node and lastNode both start at furthestBlock. Use stable DOM pointers to survive
      // vector reallocation caused by Remove() inside the inner loop.
      ContainerNode *nodeDOMNode = &furthestBlock->Node();
      ContainerNode *lastDOMNode = nodeDOMNode;

      auto entryBefore = [&](ContainerNode &node) -> ContainerNode *
      {
        auto *entry = _openElementStack.EntryBefore(node);
        return entry ? &entry->Node() : nullptr;
      };

      ContainerNode *nextDOMNode = entryBefore(*nodeDOMNode);

      // Step 4.13: Inner loop – walk from furthestBlock toward formattingElement.
      // The inner loop counter is used only to limit formatting-list membership removal (spec limit: 3).
      size_t innerLoopCounter = 0uz;
      while (true)
      {
        ++innerLoopCounter;

        // Step 4.13.2: Advance node; save the next pointer before any removal.
        nodeDOMNode = nextDOMNode;
        assert(nodeDOMNode != nullptr);
        nextDOMNode = entryBefore(*nodeDOMNode);

        // Step 4.13.3: Stop when we reach the formatting element.
        if (nodeDOMNode == &formattingElement->Node())
          break;

        // Step 4.13.4: If we have gone more than 3 steps and node is in the formatting list, remove it.
        bool nodeInFormattingList = _activeFormattingElements.ContainsFormattingElement(*nodeDOMNode);
        if (innerLoopCounter > 3uz && nodeInFormattingList)
        {
          _activeFormattingElements.RemoveAndUpdateBookmark(*nodeDOMNode, bookmark);
          nodeInFormattingList = false;
        }

        // Step 4.13.5: If node is not in the active formatting list, remove it from the open elements
        // stack and skip to the next iteration.
        auto *formattingEntry = _activeFormattingElements.Find(*nodeDOMNode);
        if (formattingEntry == nullptr)
        {
          _openElementStack.Remove(*nodeDOMNode);
          continue;
        }

        // Step 4.13.6: Create a new element from the saved token data.
        auto *stackEntry = _openElementStack.Find(*nodeDOMNode);
        assert(stackEntry != nullptr);

        auto newElement = CreateElementFromSavedItem(*stackEntry);

        // Replace the entry in the formatting list with one referencing the new element.
        formattingEntry->ReplaceItem(HTMLStackItem(stackEntry->TagName(), stackEntry->Namespace(),
                                                   *newElement, stackEntry->Attributes()));

        // Update the open elements stack entry to reference the new element.
        stackEntry->UpdateElement(*newElement);

        // nodeDOMNode now refers to the new element.
        nodeDOMNode = &stackEntry->Node();

        // Step 4.13.7: If lastNode is furthestBlock, move the bookmark to after this entry.
        if (lastDOMNode == &furthestBlock->Node())
        {
          _activeFormattingElements.MoveBookmarkAfter(bookmark, *formattingEntry);
        }

        // Step 4.13.8: Append lastNode as a child of node (reparent in the DOM tree).
        (void)MutationAlgorithms::Append(*lastDOMNode, *nodeDOMNode);

        // Step 4.13.9: lastNode = node.
        lastDOMNode = nodeDOMNode;
      }

      // Step 14: Insert lastNode at the appropriate location in the common ancestor.
      {
        auto location = AppropriateInsertionLocation(&commonAncestor->Node());
        if (location.Parent != nullptr)
          (void)MutationAlgorithms::Insert(*lastDOMNode, *location.Parent, location.BeforeSibling);
      }

      // Step 15: Create a new element from the formatting element's saved token data.
      auto newElement = CreateElementFromSavedItem(*formattingElement);

      // Steps 16–17: Move all children of furthestBlock to newElement, then append newElement to
      // furthestBlock.
      TakeAllChildrenAndReparent(newElement, *furthestBlock);

      // Step 16 (open elements): Insert newElement immediately above furthestBlock in the stack.
      _openElementStack.InsertAbove(HTMLStackItem(formattingElement->TagName(),
                                                  formattingElement->Namespace(), *newElement,
                                                  formattingElement->Attributes()),
                                    *furthestBlock);

      // Step 18: Replace the formatting element in the active formatting list with newElement at the
      // bookmark position.
      _activeFormattingElements.SwapTo(formattingElement->Node(),
                                       HTMLStackItem(formattingElement->TagName(),
                                                     formattingElement->Namespace(), *newElement,
                                                     formattingElement->Attributes()),
                                       bookmark);

      // Step 19: Remove the original formatting element from the open elements stack.
      _openElementStack.Remove(formattingElement->Node());
    }
  }

  Ref<Element> HTMLTreeBuilder::CreateElementFromSavedItem(const HTMLStackItem &item) noexcept
  {
    assert(item.IsElement());
    auto &sourceElement = Downcast<Element>(item.Node());
    auto element =
      ElementFactory::Create(sourceElement.NodeDocument(),
                             {sourceElement.NamespaceURI(), DOMStringAtom::Null(), sourceElement.LocalName()},
                             DOMStringAtom::Null(), false, nullptr);
    for (auto &attr : item.Attributes())
      ElementAlgorithms::SetAttributeValue(*element, attr.NameView(), DOMString(attr.ValueView()));
    return element;
  }

  void HTMLTreeBuilder::TakeAllChildrenAndReparent(Ref<Element> newParent, HTMLStackItem &oldParent) noexcept
  {
    auto &oldNode = oldParent.Node();

    while (auto *child = oldNode.FirstChild())
    {
      (void)MutationAlgorithms::Append(*child, *newParent);
    }

    (void)MutationAlgorithms::Append(*newParent, oldNode);
  }

  RawPtr<HTMLStackItem>
    HTMLTreeBuilder::FurthestSpecialElementBlock(const HTMLStackItem &formattingElement) noexcept
  {
    // Find the index of the formatting element in the open element stack.
    size_t formattingIndex = _openElementStack.Size();
    for (size_t i = 0uz; i < _openElementStack.Size(); ++i)
    {
      if (&_openElementStack[i] == &formattingElement)
      {
        formattingIndex = i;
        break;
      }
    }

    if (formattingIndex == _openElementStack.Size())
    {
      return nullptr;
    }

    // Scan from the formatting element toward the current element (increasing index).
    // Return the first (= closest to formattingElement) special element found.
    for (size_t i = formattingIndex + 1uz; i < _openElementStack.Size(); ++i)
    {
      auto &item = _openElementStack[i];
      if (item.IsElement() && IsSpecialElement(item.TagName()))
      {
        return &item;
      }
    }

    return nullptr;
  }

#pragma endregion
}
