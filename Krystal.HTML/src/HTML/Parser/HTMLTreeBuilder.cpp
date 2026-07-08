#include "Krystal.HTML/HTML/Parser/HTMLTreeBuilder.hpp"
#include "Krystal.HTML/DOM/Algorithms/DocumentAlgorithms.hpp"
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
#include "Krystal.HTML/SVG/SVGScriptElement.hpp"
#include "Krystal.Lib/Types/Array.hpp"
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
    if (_ignoreNextNewline)
    {
      _ignoreNextNewline = false;
      if (token.Type() == HTMLTokenType::Character && !token.Data().empty() && token.Data()[0uz] == u8'\n')
      {
        token._data = token._data.substr(1uz);
        if (token.Data().empty())
        {
          return;
        }
      }
    }

    if (ShouldProcessAccordingToRulesForHTMLContent(token))
    {
      ProcessAccordingToRulesForHTMLContent(Krys::Move(token));
    }
    else
    {
      ProcessAccordingToRulesForForeignContent(Krys::Move(token));
    }
  }

  bool HTMLTreeBuilder::ShouldProcessAccordingToRulesForHTMLContent(const HTMLTokenAtom &token) noexcept
  {
    if (_openElementStack.IsEmpty())
    {
      return true;
    }

    auto &adjustedCurrentNode = AdjustedCurrentNode();
    if (auto *adjustedCurrentElement = DynamicDowncast<Element>(adjustedCurrentNode))
    {
      if (adjustedCurrentElement->NamespaceURI() == Namespaces::HTML)
      {
        return true;
      }

      if (IsMathMLTextIntegrationPoint(*adjustedCurrentElement))
      {
        if (token.Type() == HTMLTokenType::StartTag)
        {
          auto tagName = token.Name();
          if (tagName != u8"mglyph" && tagName != u8"malignmark")
          {
            return true;
          }
        }
        else if (token.Type() == HTMLTokenType::Character)
        {
          return true;
        }
      }

      if (adjustedCurrentElement->NamespaceURI() == Namespaces::MathML
          && adjustedCurrentElement->LocalName() == u8"annotation-xml"
          && token.Type() == HTMLTokenType::StartTag && token.Name() == u8"svg")
      {
        return true;
      }

      if (IsHTMLIntegrationPoint(*adjustedCurrentElement)
          && (token.Type() == HTMLTokenType::StartTag || token.Type() == HTMLTokenType::Character))
      {
        return true;
      }
    }

    if (token.Type() == HTMLTokenType::EndOfFile)
    {
      return true;
    }

    return false;
  }

  void HTMLTreeBuilder::ProcessAccordingToRulesForHTMLContent(HTMLTokenAtom &&token) noexcept
  {
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

  void HTMLTreeBuilder::ProcessAccordingToRulesForForeignContent(HTMLTokenAtom &&token) noexcept
  {
    auto InvalidTag = [&]()
    {
      ParseError(token);

      while (true)
      {
        auto &currentNode = CurrentNode();
        if (!Is<Element>(currentNode))
        {
          _openElementStack.Pop();
          continue;
        }
        auto &currentElement = Downcast<Element>(currentNode);

        if (!IsMathMLTextIntegrationPoint(currentElement) && !IsHTMLIntegrationPoint(currentElement)
            && currentElement.NamespaceURI() != Namespaces::HTML)
        {
          _openElementStack.Pop();
          continue;
        }

        break;
      }

      ProcessAccordingToRulesForHTMLContent(Krys::Move(token));
    };

    auto ScriptTag = [&]()
    {
      _openElementStack.Pop();

      // TODO(HTMLTREEBUILDER, SVG): Implement the following steps from the HTML specification:
      // Let the old insertion point have the same value as the current insertion point. Let the insertion
      // point be just before the next input character.
      //
      // Increment the parser's script nesting level by one. Set the parser pause flag to true.
      //
      // If the active speculative HTML parser is null and the user agent supports SVG, then Process the SVG
      // script element according to the SVG rules. [SVG]
      //
      // Even if this causes new characters to be inserted into the tokenizer, the parser will not be executed
      // reentrantly, since the parser pause flag is true.
      //
      // Decrement the parser's script nesting level by one. If the parser's script nesting level is zero,
      // then set the parser pause flag to false.
      //
      // Let the insertion point have the value of the old insertion point. (In other words, restore the
      // insertion point to its previous value. This value might be the "undefined" value.)
    };

    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        auto data = DOMString(token.Data());

        if (HandlePotentialNullCharacters(data, true))
        {
          ParseError(token);
        }

        if (data.empty())
        {
          return; // ignore the token
        }

        if (!InsertCharacterTokenWhitespace(token))
        {
          return;
        }

        InsertCharacter(token, Krys::Move(data));
        _framesetOk = false;

        return;
      }
      case HTMLTokenType::Comment:
      {
        InsertComment(DOMString(token.Comment()));
        return;
      }
      // NOTE: new ProcessingInstruction token type would go here.
      case HTMLTokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case HTMLTokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::b:
          case TagName::big:
          case TagName::blockquote:
          case TagName::body:
          case TagName::br:
          case TagName::center:
          case TagName::code:
          case TagName::dd:
          case TagName::div:
          case TagName::dl:
          case TagName::dt:
          case TagName::em:
          case TagName::embed:
          case TagName::h1:
          case TagName::h2:
          case TagName::h3:
          case TagName::h4:
          case TagName::h5:
          case TagName::h6:
          case TagName::head:
          case TagName::hr:
          case TagName::i:
          case TagName::img:
          case TagName::li:
          case TagName::listing:
          case TagName::menu:
          case TagName::meta:
          case TagName::nobr:
          case TagName::ol:
          case TagName::p:
          case TagName::pre:
          case TagName::ruby:
          case TagName::s:
          case TagName::small:
          case TagName::span:
          case TagName::strong:
          case TagName::strike:
          case TagName::sub:
          case TagName::sup:
          case TagName::table:
          case TagName::tt:
          case TagName::u:
          case TagName::ul:
          case TagName::var:
          {
            InvalidTag();
            return;
          }
          case TagName::font:
          {
            if (std::ranges::any_of(token.Attributes(),
                                    [](const auto &attr)
                                    {
                                      return attr.NameView() == u8"color" || attr.NameView() == u8"face"
                                             || attr.NameView() == u8"size";
                                    }))
            {
              InvalidTag();
              return;
            }

            break;
          }
        }

        auto &adjustedCurrentNode = AdjustedCurrentNode();
        if (Is<Element>(adjustedCurrentNode))
        {
          auto &adjustedCurrentElement = Downcast<Element>(adjustedCurrentNode);

          if (adjustedCurrentElement.NamespaceURI() == Namespaces::MathML)
          {
            AdjustMathMLAttributes(token);
          }
          else if (adjustedCurrentElement.NamespaceURI() == Namespaces::SVG)
          {
            constexpr static Array<Array<DOMStringView, 2uz>, 37uz> svgTagNameAdjustments = {
              Array<DOMStringView, 2uz> {u8"altglyph", u8"altGlyph"},
              Array<DOMStringView, 2uz> {u8"altglyphdef", u8"altGlyphDef"},
              Array<DOMStringView, 2uz> {u8"altglyphitem", u8"altGlyphItem"},
              Array<DOMStringView, 2uz> {u8"animatecolor", u8"animateColor"},
              Array<DOMStringView, 2uz> {u8"animatemotion", u8"animateMotion"},
              Array<DOMStringView, 2uz> {u8"animatetransform", u8"animateTransform"},
              Array<DOMStringView, 2uz> {u8"clippath", u8"clipPath"},
              Array<DOMStringView, 2uz> {u8"feblend", u8"feBlend"},
              Array<DOMStringView, 2uz> {u8"fecolormatrix", u8"feColorMatrix"},
              Array<DOMStringView, 2uz> {u8"fecomponenttransfer", u8"feComponentTransfer"},
              Array<DOMStringView, 2uz> {u8"fecomposite", u8"feComposite"},
              Array<DOMStringView, 2uz> {u8"feconvolvematrix", u8"feConvolveMatrix"},
              Array<DOMStringView, 2uz> {u8"fediffuselighting", u8"feDiffuseLighting"},
              Array<DOMStringView, 2uz> {u8"fedisplacementmap", u8"feDisplacementMap"},
              Array<DOMStringView, 2uz> {u8"fedistantlight", u8"feDistantLight"},
              Array<DOMStringView, 2uz> {u8"fedropshadow", u8"feDropShadow"},
              Array<DOMStringView, 2uz> {u8"feflood", u8"feFlood"},
              Array<DOMStringView, 2uz> {u8"fefunca", u8"feFuncA"},
              Array<DOMStringView, 2uz> {u8"fefuncb", u8"feFuncB"},
              Array<DOMStringView, 2uz> {u8"fefuncg", u8"feFuncG"},
              Array<DOMStringView, 2uz> {u8"fefuncr", u8"feFuncR"},
              Array<DOMStringView, 2uz> {u8"fegaussianblur", u8"feGaussianBlur"},
              Array<DOMStringView, 2uz> {u8"feimage", u8"feImage"},
              Array<DOMStringView, 2uz> {u8"femerge", u8"feMerge"},
              Array<DOMStringView, 2uz> {u8"femergenode", u8"feMergeNode"},
              Array<DOMStringView, 2uz> {u8"femorphology", u8"feMorphology"},
              Array<DOMStringView, 2uz> {u8"feoffset", u8"feOffset"},
              Array<DOMStringView, 2uz> {u8"fepointlight", u8"fePointLight"},
              Array<DOMStringView, 2uz> {u8"fespecularlighting", u8"feSpecularLighting"},
              Array<DOMStringView, 2uz> {u8"fespotlight", u8"feSpotLight"},
              Array<DOMStringView, 2uz> {u8"fetile", u8"feTile"},
              Array<DOMStringView, 2uz> {u8"feturbulence", u8"feTurbulence"},
              Array<DOMStringView, 2uz> {u8"foreignobject", u8"foreignObject"},
              Array<DOMStringView, 2uz> {u8"glyphref", u8"glyphRef"},
              Array<DOMStringView, 2uz> {u8"lineargradient", u8"linearGradient"},
              Array<DOMStringView, 2uz> {u8"radialgradient", u8"radialGradient"},
              Array<DOMStringView, 2uz> {u8"textpath", u8"textPath"},
            };

            auto adjustedNameIt =
              std::find_if(svgTagNameAdjustments.begin(), svgTagNameAdjustments.end(),
                           [&](const auto &pair) { return pair[0] == token.Name().View(); });
            if (adjustedNameIt != svgTagNameAdjustments.end())
            {
              token._name = (*adjustedNameIt)[1];
            }

            AdjustSVGAttributes(token);
          }

          AdjustForeignAttributes(token);
          InsertForeignElement(Krys::Move(token), adjustedCurrentElement.NamespaceURI(), false);

          if (token.IsSelfClosing())
          {
            auto &newCurrentNode = _openElementStack.Bottom();
            if (newCurrentNode.TagName() == TagName::script && newCurrentNode.Namespace() == Namespace::SVG)
            {
              token.AcknowledgeSelfClosingTag();
              ScriptTag();
              return;
            }

            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();
            return;
          }
        }

        break;
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == TagNames::HTML::br || token.Name() == TagNames::HTML::p)
        {
          InvalidTag();
          return;
        }

        if (token.Name() == TagNames::HTML::script && Is<SVGScriptElement>(CurrentNode()))
        {
          ScriptTag();
          return;
        }

        auto *currentNode = DynamicDowncast<Element>(CurrentNode());
        if (currentNode == nullptr
            || StringAlgorithms::ASCIICaseInsensitiveMatch(currentNode->LocalName().View(),
                                                           token.Name().View()))
        {
          ParseError(token);
        }

        for (auto nodeIt = _openElementStack.rbegin(); nodeIt != _openElementStack.rend(); ++nodeIt)
        {
          auto &node = *nodeIt;

          if (&node.Node() == &_openElementStack.Top().Node())
          {
            return;
          }

          if (node.IsElement())
          {
            if (StringAlgorithms::ASCIICaseInsensitiveMatch(node.AsElement().LocalName().View(),
                                                            token.Name().View()))
            {
              _openElementStack.PopUntilPopped(node.Node());
              return;
            }

            if (node.AsElement().NamespaceURI() != Namespaces::HTML)
            {
              continue;
            }
          }

          ProcessAccordingToRulesForHTMLContent(Krys::Move(token));
          return;
        }

        break;
      }
    }
  }

  ContainerNode &HTMLTreeBuilder::CurrentNode() noexcept
  {
    return _openElementStack.Bottom().Node();
  }

  ContainerNode &HTMLTreeBuilder::AdjustedCurrentNode() noexcept
  {
    if (_contextElement != nullptr && _openElementStack.Size() == 1uz)
    {
      return *_contextElement;
    }
    else
    {
      return CurrentNode();
    }
  }

  void HTMLTreeBuilder::ParseError(const HTMLTokenAtom &token) noexcept
  {
    // TODO(HTMLTREEBUILDER, HTML): Implement parse error handling/logging.
    (void)token;
  }

#pragma region InsertionMode Algorithms

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
        AppendCommentToDocument(DOMString(token.Comment()));
        return;
      }
      // NOTE: new ProcessingInstruction token type would go here.
      case HTMLTokenType::DOCTYPE:
      {
        auto &docTypeData = *token.DOCTYPEData();

        auto name = DOMString(token.Name().View());
        if (name != TagNames::HTML::html)
        {
          ParseError(token);
        }

        auto publicIdentifier =
          DOMString(docTypeData.PublicIdentifier.begin(), docTypeData.PublicIdentifier.end());
        if (docTypeData.HasPublicIdentifier && !publicIdentifier.empty())
        {
          ParseError(token);
        }

        auto systemIdentifier =
          DOMString(docTypeData.SystemIdentifier.begin(), docTypeData.SystemIdentifier.end());
        if (docTypeData.HasSystemIdentifier && !systemIdentifier.empty()
            && systemIdentifier != u8"about:legacy-compat")
        {
          ParseError(token);
        }

        auto doctype =
          _document.Implementation().CreateDocumentType(name, publicIdentifier, systemIdentifier);
        if (!doctype.HasException())
        {
          _document.AppendChild(**doctype);
        }

        if (!DocumentAlgorithms::IsIFrameSrcdocDocument(_document) && !_document._parserCannotChangeTheMode)
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
    }

    if (!DocumentAlgorithms::IsIFrameSrcdocDocument(_document))
    {
      ParseError(token);
    }

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
        ParseError(token);
        return; // ignore the token
      }
      case HTMLTokenType::Comment:
      {
        AppendCommentToDocument(DOMString(token.Comment()));
        return;
      }
      // NOTE: new ProcessingInstruction token type would go here.
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
        if (token.Name() == TagNames::HTML::html)
        {
          auto html = CreateElement(token.Name(), Namespaces::HTML, token.Attributes(), _document);
          _document.AppendChild(*html);
          _openElementStack.Push({TagName::html, Namespace::HTML, *html, Krys::Move(token.Attributes())});

          _insertionMode = InsertionMode::BeforeHead;
          return;
        }

        break;
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == TagNames::HTML::head || token.Name() == TagNames::HTML::body
            || token.Name() == TagNames::HTML::html || token.Name() == TagNames::HTML::br)
        {
          break;
        }

        ParseError(token);
        return; // ignore the token
      }
    }

    auto html = ElementFactory::Create(_document, QualifiedName(Namespaces::HTML, DOMStringAtom::Null(),
                                                                u8"html", TagName::html, Namespace::HTML));
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
        InsertComment(DOMString(token.Comment()));
        return;
      }
      // NOTE: new ProcessingInstruction token type would go here.
      case HTMLTokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case HTMLTokenType::StartTag:
      {
        if (token.Name() == TagNames::HTML::html)
        {
          // Process the token using the rules for the "in body" insertion mode.
          InBodyMode(Krys::Move(token));
          return;
        }

        if (token.Name() == TagNames::HTML::head)
        {
          _head = InsertHTMLElement(Krys::Move(token));
          _insertionMode = InsertionMode::InHead;
          return;
        }

        break;
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == TagNames::HTML::head || token.Name() == TagNames::HTML::body
            || token.Name() == TagNames::HTML::html || token.Name() == TagNames::HTML::br)
        {
          break;
        }

        ParseError(token);
        return; // ignore the token
      }
    }

    _head =
      ElementFactory::Create(_document, QualifiedName(Namespaces::HTML, DOMStringAtom::Null(),
                                                      TagNames::HTML::head, TagName::head, Namespace::HTML));
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
        InsertComment(DOMString(token.Comment()));
        return;
      }
      // NOTE: new ProcessingInstruction token type would go here.
      case HTMLTokenType::DOCTYPE:
      {
        ParseError(token);
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

            auto element = CreateElement(token.Name(), Namespaces::HTML, token.Attributes(), *parent);
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
            ParseError(token);
            return; // ignore the token
          }
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
            break; // treat as "anything else"
          }
          case TagName::template_:
          {
            // TODO(HTMLTREEBUILDER, HTML): handle template end tag in the head insertion mode.
            return;
          }
          default:
          {
            ParseError(token);
            return; // ignore the token
          }
        }
      }
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
        ParseError(token);
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
            ParseError(token);
            return; // ignore the token
          }
        }

        break;
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == TagNames::HTML::noscript)
        {
          _openElementStack.Pop();
          _insertionMode = InsertionMode::InHead;
          return;
        }

        if (token.Name() == TagNames::HTML::br)
        {
          break;
        }

        ParseError(token);
        return; // ignore the token
      }
    }

    ParseError(token);
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
        InsertComment(DOMString(token.Comment()));
        return;
      }
      // NOTE: new ProcessingInstruction token type would go here.
      case HTMLTokenType::DOCTYPE:
      {
        ParseError(token);
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
            ParseError(token);

            _openElementStack.Push({TagName::html, Namespace::HTML, *_head, {}});
            InHeadMode(Krys::Move(token));
            _openElementStack.Remove(*_head);

            return;
          }
          case TagName::head:
          {
            ParseError(token);
            return; // ignore the token
          }
        }
        break;
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == TagNames::HTML::template_)
        {
          InHeadMode(Krys::Move(token));
          return;
        }

        if (token.Name() == TagNames::HTML::body || token.Name() == TagNames::HTML::html
            || token.Name() == TagNames::HTML::br)
        {
          break;
        }

        ParseError(token);
        return; // ignore the token
      }
    }

    auto body =
      ElementFactory::Create(_document, QualifiedName(Namespaces::HTML, DOMStringAtom::Null(),
                                                      TagNames::HTML::body, TagName::body, Namespace::HTML));
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
        auto data = DOMString(token.Data());

        if (HandlePotentialNullCharacters(data, false))
        {
          ParseError(token);
        }

        if (data.empty())
        {
          return; // ignore the token
        }

        ReconstructActiveFormattingElements();

        if (!InsertCharacterTokenWhitespace(token))
        {
          return;
        }

        InsertCharacter(token, DOMString(token.Data()));
        _framesetOk = false;

        return;
      }
      case HTMLTokenType::Comment:
      {
        InsertComment(DOMString(token.Comment()));
        return;
      }
      // NOTE: new ProcessingInstruction token type would go here.
      case HTMLTokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case HTMLTokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::html:
          {
            ParseError(token);

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
            ParseError(token);

            if (_openElementStack.Size() <= 1 || _openElementStack[1].TagName() != TagName::body
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
            ParseError(token);

            if (_openElementStack.Size() <= 1 || _openElementStack[1].TagName() != TagName::body
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
            _openElementStack.PopUntil(_openElementStack.Top().Node());
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
            if (_openElementStack.HasElementInButtonScope(TagName::p))
            {
              ClosePElement(token);
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
            if (_openElementStack.HasElementInButtonScope(TagName::p))
            {
              ClosePElement(token);
            }

            auto &currentNode = _openElementStack.Bottom();
            switch (currentNode.TagName())
            {
              case TagName::h1:
              case TagName::h2:
              case TagName::h3:
              case TagName::h4:
              case TagName::h5:
              case TagName::h6:
              {
                ParseError(token);
                _openElementStack.Pop();
                break;
              }
            }

            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::pre:
          case TagName::listing:
          {
            if (_openElementStack.HasElementInButtonScope(TagName::p))
            {
              ClosePElement(token);
            }

            InsertHTMLElement(Krys::Move(token));

            _ignoreNextNewline = true;
            _framesetOk = false;
            return;
          }
          case TagName::form:
          {
            auto containsTemplateElement = _openElementStack.ContainsTemplateElement();
            if (_form != nullptr && !containsTemplateElement)
            {
              ParseError(token);
              return; // ignore the token
            }

            if (_openElementStack.HasElementInButtonScope(TagName::p))
            {
              ClosePElement(token);
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

            for (auto nodeIt = _openElementStack.rbegin(); nodeIt != _openElementStack.rend(); ++nodeIt)
            {
              auto &node = *nodeIt;

              if (node.TagName() == TagName::li)
              {
                _openElementStack.GenerateImpliedEndTags(TagName::li);

                if (_openElementStack.Bottom().TagName() != TagName::li)
                {
                  ParseError(token);
                }

                _openElementStack.PopUntilPopped(TagName::li, Namespace::HTML);
                break;
              }

              if (IsSpecialElement(node.TagName(), node.Namespace()) && node.TagName() != TagName::address
                  && node.TagName() != TagName::div && node.TagName() != TagName::p)
              {
                break;
              }
            }

            if (_openElementStack.HasElementInButtonScope(TagName::p))
            {
              ClosePElement(token);
            }

            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::dd:
          case TagName::dt:
          {
            _framesetOk = false;

            for (auto nodeIt = _openElementStack.rbegin(); nodeIt != _openElementStack.rend(); ++nodeIt)
            {
              auto &node = *nodeIt;

              if (node.TagName() == TagName::dd)
              {
                _openElementStack.GenerateImpliedEndTags(TagName::dd);

                if (_openElementStack.Bottom().TagName() != TagName::dd)
                {
                  ParseError(token);
                }

                _openElementStack.PopUntilPopped(TagName::dd, Namespace::HTML);
                break;
              }

              if (node.TagName() == TagName::dt)
              {
                _openElementStack.GenerateImpliedEndTags(TagName::dt);

                if (_openElementStack.Bottom().TagName() != TagName::dt)
                {
                  ParseError(token);
                }

                _openElementStack.PopUntilPopped(TagName::dt, Namespace::HTML);
                break;
              }

              if (IsSpecialElement(node.TagName(), node.Namespace()) && node.TagName() != TagName::address
                  && node.TagName() != TagName::div && node.TagName() != TagName::p)
              {
                break;
              }
            }

            if (_openElementStack.HasElementInButtonScope(TagName::p))
            {
              ClosePElement(token);
            }

            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::plaintext:
          {
            if (_openElementStack.HasElementInButtonScope(TagName::p))
            {
              ClosePElement(token);
            }

            InsertHTMLElement(Krys::Move(token));

            _tokenizer.State(TokenizerState::PLAINTEXT);
            return;
          }
          case TagName::button:
          {
            if (_openElementStack.HasElementInButtonScope(TagName::button))
            {
              ParseError(token);
              ClosePElement(token);
              _openElementStack.PopUntilPopped(TagName::button, Namespace::HTML);
            }

            ReconstructActiveFormattingElements();
            InsertHTMLElement(Krys::Move(token));

            _framesetOk = false;

            return;
          }
          case TagName::a:
          {
            // If an a element is in the formatting list between the end and the last marker, this is a
            // parse error; run the adoption agency algorithm, then remove that element from the list and
            // stack of open elements if the adoption agency algorithm didn't already remove it.
            if (auto *existing = _activeFormattingElements.FindFromLastMarker(TagName::a))
            {
              ParseError(token);

              // Stack item might be removed by AAA so we need to keep a reference to the node here.
              auto &existingNode = existing->Node();

              RunAdoptionAgency(token);

              _activeFormattingElements.Remove(existingNode);
              _openElementStack.Remove(existingNode);
            }

            ReconstructActiveFormattingElements();
            InsertHTMLElement(Krys::Move(token));

            auto &aOpenItem = _openElementStack.Bottom();
            ParsedAttributeList aAttrsCopy = aOpenItem.Attributes();
            _activeFormattingElements.PushElement(HTMLStackItem(
              aOpenItem.TagName(), aOpenItem.Namespace(), aOpenItem.AsElement(), Krys::Move(aAttrsCopy)));

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
            ReconstructActiveFormattingElements();
            InsertHTMLElement(Krys::Move(token));

            auto &fmtOpenItem = _openElementStack.Bottom();
            ParsedAttributeList fmtAttrsCopy = fmtOpenItem.Attributes();
            _activeFormattingElements.PushElement(
              HTMLStackItem(fmtOpenItem.TagName(), fmtOpenItem.Namespace(), fmtOpenItem.AsElement(),
                            Krys::Move(fmtAttrsCopy)));

            return;
          }
          case TagName::nobr:
          {
            ReconstructActiveFormattingElements();

            if (_openElementStack.HasElementInScope(TagName::nobr))
            {
              ParseError(token);
              RunAdoptionAgency(token);
              ReconstructActiveFormattingElements();
            }

            InsertHTMLElement(Krys::Move(token));

            auto &nobrOpenItem = _openElementStack.Bottom();
            ParsedAttributeList nobrAttrsCopy = nobrOpenItem.Attributes();
            _activeFormattingElements.PushElement(
              HTMLStackItem(nobrOpenItem.TagName(), nobrOpenItem.Namespace(), nobrOpenItem.AsElement(),
                            Krys::Move(nobrAttrsCopy)));

            return;
          }
          case TagName::applet:
          case TagName::marquee:
          case TagName::object:
          {
            ReconstructActiveFormattingElements();
            InsertHTMLElement(Krys::Move(token));
            _activeFormattingElements.PushMarker();
            _framesetOk = false;

            return;
          }
          case TagName::table:
          {
            if (_document._quirksMode != QuirksMode::Quirks
                && _openElementStack.HasElementInButtonScope(TagName::p))
            {
              ClosePElement(token);
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
            ReconstructActiveFormattingElements();
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
              ParseError(token);
              return; // ignore the token
            }

            if (_openElementStack.HasElementInScope(TagName::select))
            {
              ParseError(token);
              _openElementStack.PopUntilPopped(TagName::select, Namespace::HTML);
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

            ReconstructActiveFormattingElements();
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
            if (_openElementStack.HasElementInButtonScope(TagName::p))
            {
              ClosePElement(token);
            }

            if (_openElementStack.HasElementInScope(TagName::select))
            {
              _openElementStack.GenerateImpliedEndTags();
              if (_openElementStack.HasElementInScope(TagName::option)
                  || _openElementStack.HasElementInScope(TagName::optgroup))
              {
                ParseError(token);
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
            ParseError(token);
            token._name = u8"img"; // reprocess the token as an img tag.

            InBodyMode(Krys::Move(token));
            return;
          }
          case TagName::textarea:
          {
            InsertHTMLElement(Krys::Move(token));

            _ignoreNextNewline = true;
            _tokenizer.State(TokenizerState::RCDATA);
            _originalInsertionMode = _insertionMode;
            _framesetOk = false;

            _insertionMode = InsertionMode::Text;
            return;
          }
          case TagName::xmp:
          {
            if (_openElementStack.HasElementInButtonScope(TagName::p))
            {
              ClosePElement(token);
            }

            ReconstructActiveFormattingElements();
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
            ReconstructActiveFormattingElements();
            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::select:
          {
            if (_scriptingMode == ParserScriptingMode::Fragment && Is<HTMLSelectElement>(_contextElement))
            {
              ParseError(token);
              return; // ignore the token
            }

            if (_openElementStack.HasElementInScope(TagName::select))
            {
              ParseError(token);
              _openElementStack.PopUntilPopped(TagName::select, Namespace::HTML);
              return; // ignore the token
            }

            ReconstructActiveFormattingElements();
            InsertHTMLElement(Krys::Move(token));
            _framesetOk = false;

            return;
          }
          case TagName::option:
          {
            if (_openElementStack.HasElementInScope(TagName::select))
            {
              _openElementStack.GenerateImpliedEndTags(TagName::optgroup);
              if (_openElementStack.HasElementInScope(TagName::option))
              {
                ParseError(token);
              }
            }
            else
            {
              if (_openElementStack.Bottom().TagName() == TagName::option)
              {
                _openElementStack.Pop();
              }
            }

            ReconstructActiveFormattingElements();
            InsertHTMLElement(Krys::Move(token));

            return;
          }
          case TagName::optgroup:
          {
            if (_openElementStack.HasElementInScope(TagName::select))
            {
              _openElementStack.GenerateImpliedEndTags();
              if (_openElementStack.HasElementInScope(TagName::option)
                  || _openElementStack.HasElementInScope(TagName::optgroup))
              {
                ParseError(token);
              }
            }
            else
            {
              if (_openElementStack.Bottom().TagName() == TagName::option)
              {
                _openElementStack.Pop();
              }
            }

            ReconstructActiveFormattingElements();
            InsertHTMLElement(Krys::Move(token));

            return;
          }
          case TagName::rb:
          case TagName::rtc:
          {
            if (_openElementStack.HasElementInScope(TagName::ruby))
            {
              _openElementStack.GenerateImpliedEndTags();
              if (_openElementStack.Bottom().TagName() != TagName::ruby)
              {
                ParseError(token);
              }
            }

            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::rp:
          case TagName::rt:
          {
            if (_openElementStack.HasElementInScope(TagName::ruby))
            {
              _openElementStack.GenerateImpliedEndTags(TagName::rtc);

              auto &rtCurrentNode = _openElementStack.Bottom();
              if (rtCurrentNode.TagName() != TagName::rtc && rtCurrentNode.TagName() != TagName::ruby)
              {
                ParseError(token);
              }
            }

            InsertHTMLElement(Krys::Move(token));
            return;
          }
          case TagName::math:
          {
            ReconstructActiveFormattingElements();
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
          case TagName::svg:
          {
            ReconstructActiveFormattingElements();
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
            ParseError(token);
            return; // ignore the token
          }
          default:
          {
            ReconstructActiveFormattingElements();
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
            if (!_openElementStack.HasElementInScope(TagName::body))
            {
              ParseError(token);
              return; // ignore the token
            }

            if (_openElementStack.ContainsInvalidUnclosedElements())
            {
              ParseError(token);
            }

            _insertionMode = InsertionMode::AfterBody;
            return;
          }
          case TagName::html:
          {
            if (!_openElementStack.HasElementInScope(TagName::body))
            {
              ParseError(token);
              return; // ignore the token
            }

            if (_openElementStack.ContainsInvalidUnclosedElements())
            {
              ParseError(token);
            }

            _insertionMode = InsertionMode::AfterBody;
            AfterBodyMode(Krys::Move(token)); // reprocess the token.

            return;
          }
          case TagName::address:
          case TagName::article:
          case TagName::aside:
          case TagName::blockquote:
          case TagName::button:
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
          case TagName::listing:
          case TagName::main:
          case TagName::menu:
          case TagName::nav:
          case TagName::ol:
          case TagName::pre:
          case TagName::search:
          case TagName::section:
          case TagName::select:
          case TagName::summary:
          case TagName::ul:
          {
            if (!_openElementStack.HasElementInScope(tagName))
            {
              return; // ignore the token
            }

            _openElementStack.GenerateImpliedEndTags();

            if (_openElementStack.Bottom().TagName() != tagName)
            {
              ParseError(token);
            }

            _openElementStack.PopUntilPopped(tagName, Namespace::HTML);
            return;
          }
          case TagName::form:
          {
            if (!_openElementStack.ContainsTemplateElement())
            {
              RefPtr<HTMLFormElement> form = Krys::Move(_form);
              _form = nullptr;

              if (form == nullptr || !_openElementStack.HasElementInScope(*form))
              {
                ParseError(token);
                return; // ignore the token
              }

              _openElementStack.GenerateImpliedEndTags();

              if (&CurrentNode() != form.get())
              {
                ParseError(token);
              }

              _openElementStack.Remove(*form);
            }
            else
            {
              if (!_openElementStack.HasElementInScope(TagName::form))
              {
                ParseError(token);
                return; // ignore the token
              }

              _openElementStack.GenerateImpliedEndTags();

              if (_openElementStack.Bottom().TagName() != TagName::form)
              {
                ParseError(token);
              }

              _openElementStack.PopUntilPopped(TagName::form, Namespace::HTML);
            }

            return;
          }
          case TagName::p:
          {
            if (!_openElementStack.HasElementInButtonScope(TagName::p))
            {
              ParseError(token);
              auto pElement = ElementFactory::Create(
                _document, QualifiedName(Namespaces::HTML, DOMStringAtom::Null(), TagNames::HTML::p,
                                         TagName::p, Namespace::HTML));
              InsertElementAtAdjustedInsertionLocation(*pElement);
              _openElementStack.Push({TagName::p, Namespace::HTML, *pElement, {}});
            }

            ClosePElement(token);
            return;
          }
          case TagName::li:
          {
            if (!_openElementStack.HasElementInListItemScope(TagName::li))
            {
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.GenerateImpliedEndTags(TagName::li);

            if (_openElementStack.Bottom().TagName() != TagName::li)
            {
              ParseError(token);
            }

            _openElementStack.PopUntilPopped(TagName::li, Namespace::HTML);
            return;
          }
          case TagName::dd:
          case TagName::dt:
          {
            // TODO(HTMLTREEBUILDER): the spec specifies these steps as specific to HTMLElements, check that
            // this approach is acceptable.

            if (!_openElementStack.HasElementInScope(tagName))
            {
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.GenerateImpliedEndTags(tagName);

            if (_openElementStack.Bottom().TagName() != tagName)
            {
              ParseError(token);
            }

            _openElementStack.PopUntilPopped(tagName, Namespace::HTML);
            return;
          }
          case TagName::h1:
          case TagName::h2:
          case TagName::h3:
          case TagName::h4:
          case TagName::h5:
          case TagName::h6:
          {
            // TODO(HTMLTREEBUILDER): the spec specifies these steps as specific to HTMLElements, check that
            // this approach is acceptable.

            if (!_openElementStack.HasElementInScope(TagName::h1)
                && !_openElementStack.HasElementInScope(TagName::h2)
                && !_openElementStack.HasElementInScope(TagName::h3)
                && !_openElementStack.HasElementInScope(TagName::h4)
                && !_openElementStack.HasElementInScope(TagName::h5)
                && !_openElementStack.HasElementInScope(TagName::h6))
            {
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.GenerateImpliedEndTags();

            if (_openElementStack.Bottom().TagName() != tagName)
            {
              ParseError(token);
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
            ParseError(token);

            // NOTE: act as if this were a "br" start tag with no attributes.
            token._attributes.clear();
            ReconstructActiveFormattingElements();
            InsertHTMLElement(Krys::Move(token));
            _openElementStack.Pop();

            _framesetOk = false;
            return;
          }
          case TagName::applet:
          case TagName::marquee:
          case TagName::object:
          {
            // TODO(HTMLTREEBUILDER): the spec specifies these steps as specific to HTMLElements, check that
            // this approach is acceptable.

            if (!_openElementStack.HasElementInScope(tagName))
            {
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.GenerateImpliedEndTags();

            if (_openElementStack.Bottom().TagName() != tagName)
            {
              ParseError(token);
            }

            _openElementStack.PopUntilPopped(tagName, Namespace::HTML);
            _activeFormattingElements.ClearUpToLastMarker();

            return;
          }
          default:
          {
            InBodyGenericEndTag(token, tagName);
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
          ParseError(token);
        }

        StopParsing();
        return;
      }
    }
  }

  void HTMLTreeBuilder::TextMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        InsertCharacter(token, DOMString(token.Data()));
        return;
      }
      case HTMLTokenType::EndOfFile:
      {
        ParseError(token);

        if (_openElementStack.Bottom().TagName() == TagName::script)
        {
          Downcast<HTMLScriptElement>(_openElementStack.Bottom().Node())._alreadyStarted = true;
        }

        _openElementStack.Pop();

        _insertionMode = _originalInsertionMode;
        ProcessToken(Krys::Move(token));

        return;
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name().View() == u8"script")
        {
          // TODO(HTMLTREEBUILDER, HTML): HTMLScriptElement end tag handling
        }

        _openElementStack.Pop();

        _insertionMode = _originalInsertionMode;
        return;
      }
    }
  }

  void HTMLTreeBuilder::InTableMode(HTMLTokenAtom &&token) noexcept
  {
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
            InTableTextMode(Krys::Move(token));

            return;
          }
        }

        break; // fall through to "anything else"
      }
      case HTMLTokenType::Comment:
      {
        InsertComment(DOMString(token.Comment()));
        return;
      }
      // NOTE: new ProcessingInstruction token type would go here.
      case HTMLTokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case HTMLTokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::caption:
          {
            _openElementStack.PopUntilTableContext();
            _activeFormattingElements.PushMarker();
            InsertHTMLElement(Krys::Move(token));

            _insertionMode = InsertionMode::InCaption;
            return;
          }
          case TagName::colgroup:
          {
            _openElementStack.PopUntilTableContext();
            InsertHTMLElement(Krys::Move(token));

            _insertionMode = InsertionMode::InColumnGroup;
            return;
          }
          case TagName::col:
          {
            _openElementStack.PopUntilTableContext();

            auto colgroup = ElementFactory::Create(
              _document, QualifiedName(Namespaces::HTML, DOMStringAtom::Null(), TagNames::HTML::colgroup,
                                       TagName::colgroup, Namespace::HTML));
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
            _openElementStack.PopUntilTableContext();
            InsertHTMLElement(Krys::Move(token));

            _insertionMode = InsertionMode::InTableBody;
            return;
          }
          case TagName::td:
          case TagName::th:
          case TagName::tr:
          {
            _openElementStack.PopUntilTableContext();

            auto tbody = ElementFactory::Create(
              _document, QualifiedName(Namespaces::HTML, DOMStringAtom::Null(), TagNames::HTML::tbody,
                                       TagName::tbody, Namespace::HTML));
            InsertElementAtAdjustedInsertionLocation(*tbody);
            _openElementStack.Push({TagName::tbody, Namespace::HTML, *tbody, {}});

            _insertionMode = InsertionMode::InTableBody;
            ProcessToken(Krys::Move(token));

            return;
          }
          case TagName::table:
          {
            ParseError(token);

            if (!_openElementStack.HasElementInTableScope(TagName::table))
            {
              return; // ignore the token
            }

            _openElementStack.PopUntilPopped(TagName::table, Namespace::HTML);
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
            auto typeIt = std::ranges::find_if(token.Attributes(),
                                               [](const ParsedAttribute &attr)
                                               {
                                                 return attr.NameView() == u8"type"
                                                        && StringAlgorithms::ASCIICaseInsensitiveMatch(
                                                          attr.ValueView(), u8"hidden");
                                               });

            if (typeIt == token.Attributes().end())
            {
              break; // fall through to "anything else"
            }

            ParseError(token);

            InsertHTMLElement(Krys::Move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();

            return;
          }
          case TagName::form:
          {
            ParseError(token);

            if (_openElementStack.ContainsTemplateElement() || _form != nullptr)
            {
              return; // ignore the token
            }

            _form = InsertHTMLElement(Krys::Move(token));
            _openElementStack.Pop();

            return;
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
            if (!_openElementStack.HasElementInTableScope(TagName::table))
            {
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.PopUntilPopped(TagName::table, Namespace::HTML);
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
            ParseError(token);
            return; // ignore the token
          }
          case TagName::template_:
          {
            InHeadMode(Krys::Move(token));
            return;
          }
        }
        break; // fall through to "anything else"
      }
      case HTMLTokenType::EndOfFile:
      {
        InBodyMode(Krys::Move(token));
        return;
      }
    }

    ParseError(token);

    _fosterParenting = true;
    InBodyMode(Krys::Move(token));
    _fosterParenting = false;
  }

  void HTMLTreeBuilder::InTableTextMode(HTMLTokenAtom &&token) noexcept
  {
    if (token.Type() == HTMLTokenType::Character)
    {
      auto data = DOMString(token.Data());

      if (HandlePotentialNullCharacters(data, false))
      {
        ParseError(token);
      }

      if (data.empty())
      {
        return; // ignore the token
      }

      _pendingTableCharacterTokens.emplace_back(Krys::Move(data));
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
      ParseError(token);

      // Reprocess each pending character token using the "anything else" entry in the "in table" mode.

      _fosterParenting = true;

      ReconstructActiveFormattingElements();
      for (auto &chars : _pendingTableCharacterTokens)
      {
        InsertCharacter(token, DOMString(chars));
      }
      _pendingTableCharacterTokens.clear();

      _framesetOk = false;
      _fosterParenting = false;
    }
    else
    {
      for (auto &chars : _pendingTableCharacterTokens)
      {
        InsertCharacter(token, DOMString(chars));
      }
      _pendingTableCharacterTokens.clear();
    }

    _insertionMode = _originalInsertionMode;
    ProcessToken(Krys::Move(token));
  }

  void HTMLTreeBuilder::InCaptionMode(HTMLTokenAtom &&token) noexcept
  {
    auto CloseCaption = [&]() -> bool
    {
      if (!_openElementStack.HasElementInTableScope(TagName::caption))
      {
        ParseError(token);
        return false; // fragment case — ignore
      }

      _openElementStack.GenerateImpliedEndTags();
      if (_openElementStack.Bottom().TagName() != TagName::caption)
      {
        ParseError(token);
      }

      _openElementStack.PopUntilPopped(TagName::caption, Namespace::HTML);
      _activeFormattingElements.ClearUpToLastMarker();

      _insertionMode = InsertionMode::InTable;
      return true;
    };

    switch (token.Type())
    {
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
            if (CloseCaption())
            {
              ProcessToken(Krys::Move(token));
            }
            return;
          }
        }
      }
      case HTMLTokenType::EndTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::caption:
          {
            CloseCaption();
            return;
          }
          case TagName::table:
          {
            if (CloseCaption())
            {
              ProcessToken(Krys::Move(token));
            }
            return;
          }
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
            ParseError(token);
            return; // ignore the token
          }
        }
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
          break;
        }

        return;
      }
      case HTMLTokenType::Comment:
      {
        InsertComment(DOMString(token.Comment()));
        return;
      }
      // NOTE: new ProcessingInstruction token type would go here.
      case HTMLTokenType::DOCTYPE:
      {
        ParseError(token);
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
        }
        break;
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
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.Pop();

            _insertionMode = InsertionMode::InTable;
            return;
          }
          case TagName::col:
          {
            ParseError(token);
            return; // ignore the token
          }
          case TagName::template_:
          {
            InHeadMode(Krys::Move(token));
            return;
          }
        }
        break;
      }
      case HTMLTokenType::EndOfFile:
      {
        InBodyMode(Krys::Move(token));
        return;
      }
    }

    if (_openElementStack.Bottom().TagName() != TagName::colgroup)
    {
      ParseError(token);
      return; // ignore the token
    }

    _openElementStack.Pop();

    _insertionMode = InsertionMode::InTable;
    InTableMode(Krys::Move(token));
  }

  void HTMLTreeBuilder::InTableBodyMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::tr:
          {
            _openElementStack.PopUntilTableBodyContext();
            InsertHTMLElement(Krys::Move(token));

            _insertionMode = InsertionMode::InRow;
            return;
          }
          case TagName::th:
          case TagName::td:
          {
            ParseError(token);

            _openElementStack.PopUntilTableBodyContext();

            auto tr = ElementFactory::Create(_document,
                                             QualifiedName(Namespaces::HTML, DOMStringAtom::Null(),
                                                           TagNames::HTML::tr, TagName::tr, Namespace::HTML));
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
            if (!_openElementStack.HasElementInTableScope(TagName::tbody)
                && !_openElementStack.HasElementInTableScope(TagName::thead)
                && !_openElementStack.HasElementInTableScope(TagName::tfoot))
            {
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.PopUntilTableBodyContext();
            _openElementStack.Pop();

            _insertionMode = InsertionMode::InTable;
            ProcessToken(Krys::Move(token));

            return;
          }
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
            // TODO(HTMLTREEBUILDER): the spec specifies these steps as specific to HTMLElements, check that
            // this approach is acceptable.

            if (!_openElementStack.HasElementInTableScope(tagName))
            {
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.PopUntilTableBodyContext();
            _openElementStack.Pop();

            _insertionMode = InsertionMode::InTable;
            return;
          }
          case TagName::table:
          {
            if (!_openElementStack.HasElementInTableScope(TagName::tbody)
                && !_openElementStack.HasElementInTableScope(TagName::thead)
                && !_openElementStack.HasElementInTableScope(TagName::tfoot))
            {
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.PopUntilTableBodyContext();
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
            ParseError(token);
            return; // ignore the token
          }
        }
        break;
      }
    }

    InTableMode(Krys::Move(token));
  }

  void HTMLTreeBuilder::InRowMode(HTMLTokenAtom &&token) noexcept
  {
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
            _openElementStack.PopUntilTableRowContext();

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
            if (!_openElementStack.HasElementInTableScope(TagName::tr))
            {
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.PopUntilTableRowContext();
            _openElementStack.Pop();

            _insertionMode = InsertionMode::InTableBody;
            ProcessToken(Krys::Move(token));

            return;
          }
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
            if (!_openElementStack.HasElementInTableScope(TagName::tr))
            {
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.PopUntilTableRowContext();
            _openElementStack.Pop();

            _insertionMode = InsertionMode::InTableBody;
            return;
          }
          case TagName::table:
          {
            if (!_openElementStack.HasElementInTableScope(TagName::tr))
            {
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.PopUntilTableRowContext();
            _openElementStack.Pop();

            _insertionMode = InsertionMode::InTableBody;
            ProcessToken(Krys::Move(token));

            return;
          }
          case TagName::tbody:
          case TagName::tfoot:
          case TagName::thead:
          {
            // TODO(HTMLTREEBUILDER): the spec specifies these steps as specific to HTMLElements, check that
            // this approach is acceptable.

            if (!_openElementStack.HasElementInTableScope(tagName))
            {
              ParseError(token);
              return; // ignore the token
            }

            if (!_openElementStack.HasElementInTableScope(TagName::tr))
            {
              return; // ignore the token
            }

            _openElementStack.PopUntilTableRowContext();
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
            ParseError(token);
            return; // ignore the token
          }
        }
        break;
      }
    }

    InTableMode(Krys::Move(token));
  }

  void HTMLTreeBuilder::InCellMode(HTMLTokenAtom &&token) noexcept
  {
    auto CloseTheCell = [&]()
    {
      _openElementStack.GenerateImpliedEndTags();

      auto &current = _openElementStack.Bottom();

      if (current.TagName() != TagName::td && current.TagName() != TagName::th)
      {
        ParseError(token);
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
            assert(_openElementStack.HasElementInTableScope(TagName::td)
                   || _openElementStack.HasElementInTableScope(TagName::th));
            CloseTheCell();
            ProcessToken(Krys::Move(token));
            return;
          }
        }
        break;
      }
      case HTMLTokenType::EndTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::td:
          case TagName::th:
          {
            if (!_openElementStack.HasElementInTableScope(tagName))
            {
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.GenerateImpliedEndTags();

            if (_openElementStack.Bottom().TagName() != tagName)
            {
              ParseError(token);
            }

            _openElementStack.PopUntilPopped(tagName, Namespace::HTML);
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
            ParseError(token);
            return; // ignore the token
          }
          case TagName::table:
          case TagName::tbody:
          case TagName::tfoot:
          case TagName::thead:
          case TagName::tr:
          {
            if (!_openElementStack.HasElementInTableScope(tagName))
            {
              ParseError(token);
              return; // ignore the token
            }

            CloseTheCell();
            ProcessToken(Krys::Move(token));

            return;
          }
        }
        break;
      }
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

        ParseError(token);
        return; // ignore the token
      }
      case HTMLTokenType::EndOfFile:
      {
        if (!_openElementStack.ContainsTemplateElement())
        {
          StopParsing();
          return;
        }

        ParseError(token);

        _openElementStack.PopUntilPopped(TagName::template_, Namespace::HTML);
        _activeFormattingElements.ClearUpToLastMarker();

        _templateInsertionModes.pop_back();
        ResetInsertionModeAppropriately();

        ProcessToken(Krys::Move(token));
        return;
      }
    }
  }

  void HTMLTreeBuilder::AfterBodyMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        ReconstructActiveFormattingElements();
        if (!InsertCharacterTokenWhitespace(token))
        {
          return;
        }

        break;
      }
      case HTMLTokenType::Comment:
      {
        InsertComment(DOMString(token.Comment()),
                      AdjustedInsertionLocation {&_openElementStack.Top().Node(), nullptr});
        return;
      }
      // NOTE: new ProcessingInstruction token type would go here.
      case HTMLTokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case HTMLTokenType::StartTag:
      {
        if (token.Name() == u8"html")
        {
          InBodyMode(Krys::Move(token));
          return;
        }
        break;
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == u8"html")
        {
          if (_contextElement != nullptr)
          {
            ParseError(token); // fragment case, ignore
            return;
          }

          _insertionMode = InsertionMode::AfterAfterBody;
          return;
        }
        break;
      }
      case HTMLTokenType::EndOfFile:
      {
        StopParsing();
        return;
      }
    }

    ParseError(token);

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
        {
          break;
        }

        return;
      }
      case HTMLTokenType::Comment:
      {
        InsertComment(DOMString(token.Comment()));
        return;
      }
      // NOTE: new ProcessingInstruction token type would go here.
      case HTMLTokenType::DOCTYPE:
      {
        ParseError(token);
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
        }

        break;
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == u8"frameset")
        {
          if (&_openElementStack.Bottom().Node() == &_openElementStack.Top().Node())
          {
            ParseError(token);
            return; // fragment case, ignore
          }

          _openElementStack.Pop();

          if (_contextElement == nullptr && _openElementStack.Bottom().TagName() != TagName::frameset)
          {
            _insertionMode = InsertionMode::AfterFrameset;
          }

          return;
        }

        break;
      }
      case HTMLTokenType::EndOfFile:
      {
        if (&_openElementStack.Bottom().Node() != &_openElementStack.Top().Node())
        {
          ParseError(token);
        }

        StopParsing();
        return;
      }
    }

    ParseError(token);
  }

  void HTMLTreeBuilder::AfterFramesetMode(HTMLTokenAtom &&token) noexcept
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
        InsertComment(DOMString(token.Comment()));
        return;
      }
      // NOTE: new ProcessingInstruction token type would go here.
      case HTMLTokenType::DOCTYPE:
      {
        ParseError(token);
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
        }
        break;
      }
      case HTMLTokenType::EndTag:
      {
        if (token.Name() == u8"html")
        {
          _insertionMode = InsertionMode::AfterAfterFrameset;
          return;
        }

        break;
      }
      case HTMLTokenType::EndOfFile:
      {
        StopParsing();
        return;
      }
    }

    ParseError(token);
  }

  void HTMLTreeBuilder::AfterAfterBodyMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Comment:
      {
        AppendCommentToDocument(DOMString(token.Comment()));
        return;
      }
      // NOTE: new ProcessingInstruction token type would go here.
      case HTMLTokenType::DOCTYPE:
      {
        InBodyMode(Krys::Move(token));
        return;
      }
      case HTMLTokenType::Character:
      {
        ReconstructActiveFormattingElements();
        if (InsertCharacterTokenWhitespace(token))
        {
          break;
        }

        return;
      }
      case HTMLTokenType::StartTag:
      {
        if (token.Name() == u8"html")
        {
          InBodyMode(Krys::Move(token));
          return;
        }

        break;
      }
      case HTMLTokenType::EndOfFile:
      {
        StopParsing();
        return;
      }
    }

    ParseError(token);

    _insertionMode = InsertionMode::InBody;
    ProcessToken(Krys::Move(token));
  }

  void HTMLTreeBuilder::AfterAfterFramesetMode(HTMLTokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Comment:
      {
        AppendCommentToDocument(DOMString(token.Comment()));
        return;
      }
      // NOTE: new ProcessingInstruction token type would go here.
      case HTMLTokenType::DOCTYPE:
      {
        InBodyMode(Krys::Move(token));
        return;
      }
      case HTMLTokenType::Character:
      {
        ReconstructActiveFormattingElements();
        if (InsertCharacterTokenWhitespace(token))
        {
          break;
        }

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
        }
        break;
      }
      case HTMLTokenType::EndOfFile:
      {
        StopParsing();
        return;
      }
    }

    ParseError(token);
  }

  void HTMLTreeBuilder::InBodyGenericEndTag(const HTMLTokenAtom &token, TagName tagName)
  {
    for (auto nodeIt = _openElementStack.rbegin(); nodeIt != _openElementStack.rend(); ++nodeIt)
    {
      auto &nodeEntry = *nodeIt;
      // NOTE: this part of the spec ensures misnested end tags are properly closed. We check the local name
      // instead of the parsed tag name to avoid issues with foreign/unknown elements.
      if (nodeEntry.Namespace() == Namespace::HTML && nodeEntry.IsElement()
          && nodeEntry.AsElement().LocalName() == token.Name())
      {
        _openElementStack.GenerateImpliedEndTags(tagName);

        if (&nodeEntry != &_openElementStack.Bottom())
        {
          ParseError(token);
        }

        _openElementStack.PopUntilPopped(nodeEntry.Node());
        return;
      }

      if (IsSpecialElement(nodeEntry.TagName(), nodeEntry.Namespace()))
      {
        ParseError(token);
        return; // ignore the token
      }
    }
  }

  void HTMLTreeBuilder::StopParsing() noexcept
  {
    // TODO(HTMLTREEBUILDER, HTML): Stop parsing.
    _openElementStack.PopAll();
  }

#pragma endregion

#pragma region Insertion Algorithms

  AdjustedInsertionLocation
    HTMLTreeBuilder::AppropriateInsertionLocation(RawPtr<HTMLStackItem> targetOverride) noexcept
  {
    auto &target = targetOverride ? *targetOverride : _openElementStack.Bottom();

    AdjustedInsertionLocation location {};

    if (_fosterParenting && target.Namespace() == Namespace::HTML
        && (target.TagName() == TagName::table || target.TagName() == TagName::tbody
            || target.TagName() == TagName::tfoot || target.TagName() == TagName::thead
            || target.TagName() == TagName::tr))
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
      location.Parent = &target.Node();
      location.BeforeSibling = nullptr;
    }

    if (auto *templateElement = DynamicDowncast<HTMLTemplateElement>(location.Parent))
    {
      location.Parent = templateElement->Content().get();
      location.BeforeSibling = nullptr;
    }

    return location;
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
    auto element = CreateElement(token.Name(), namespaceURI, token.Attributes(), *parent);

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

  void HTMLTreeBuilder::InsertCharacter(HTMLTokenAtom &token, DOMString &&data) noexcept
  {
    assert(token.Type() == HTMLTokenType::Character || _insertionMode == InsertionMode::InTableText);

    if (HandlePotentialNullCharacters(data, false))
    {
      ParseError(token);
    }

    if (data.empty())
    {
      return; // ignore the token
    }

    auto [parent, beforeSibling] = AppropriateInsertionLocation();

    if (Is<Document>(parent))
    {
      return;
    }

    auto previousSibling = beforeSibling ? beforeSibling->PreviousSibling() : nullptr;
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

    auto data = token.Data();

    auto position = data.begin();
    StringAlgorithms::SkipWhitespace(data, position);

    if (position == data.begin())
    {
      return !data.empty();
    }

    InsertCharacter(token, DOMString(data.begin(), position));

    if (position == data.end())
    {
      return false;
    }

    token._data = DOMStringView(position, data.end());
    return true;
  }

  bool HTMLTreeBuilder::HandlePotentialNullCharacters(DOMString &data, bool replace) noexcept
  {
    if (!replace)
    {
      auto removedNullCharacters = std::erase_if(data, [](char8 c) { return c == u8'\0'; });
      return removedNullCharacters > 0uz;
    }

    auto &&pos = data.find(u8"\0", size_t {});
    size_t found = 0uz;

    DOMString from = u8"a";
    from[0] = u8'\0';

    DOMString to = u8"\uFDDD";

    while (pos != std::string::npos)
    {
      found++;
      data.replace(pos, from.length(), to);
      // easy to forget to add to.length()
      pos = data.find(from, pos + to.length());
    }

    return found > 0uz;
  }

  void HTMLTreeBuilder::InsertComment(DOMString &&data, Maybe<AdjustedInsertionLocation> position) noexcept
  {
    auto [parent, beforeSibling] = position.has_value() ? *position : AppropriateInsertionLocation();
    auto commentNode = CreateRef<Comment>(parent->NodeDocument(), Krys::Move(data));

    // NOTE: We purposely ignore the error here if it happens.
    (void)MutationAlgorithms::Insert(*commentNode, *parent, beforeSibling);
  }

  void HTMLTreeBuilder::AppendCommentToDocument(DOMString &&data) noexcept
  {
    InsertComment(Krys::Move(data), AdjustedInsertionLocation {&_document, nullptr});
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

  Ref<Element> HTMLTreeBuilder::CreateElement(DOMStringAtom name, DOMStringAtom namespaceURI,
                                              const ParsedAttributeList &attributes,
                                              ContainerNode &intendedParent) noexcept
  {
    auto &document = intendedParent.NodeDocument();
    auto is = [&]() -> DOMStringAtom
    {
      auto it = std::ranges::find_if(attributes,
                                     [](const ParsedAttribute &attr) { return attr.NameView() == u8"is"; });

      return it != std::ranges::end(attributes) ? it->NameView() : DOMStringAtom::Null();
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

    auto element = ElementFactory::Create(document, {namespaceURI, DOMStringAtom::Null(), name}, is,
                                          willExecuteScript, registry);
    for (auto &attr : attributes)
    {
      ElementAlgorithms::SetAttributeValue(*element, attr.NameView(), DOMString(attr.ValueView()));
    }

    // TODO(HTMLTREEBUILDER, CUSTOMELEMENTS, HTML): HTMLScriptElement handling
    // If willExecuteScript is true:
    //     Let queue be the result of popping from document's relevant agent's custom element reactions stack.
    //     (This will be the same element queue as was pushed above.)
    //     Invoke custom element reactions in queue.
    //     Decrement document's throw-on-dynamic-markup-insertion counter.
    //
    // TODO(HTMLTREEBUILDER, HTML):create element parse error
    // If element has an xmlns attribute in the XMLNS namespace whose value is not exactly the same as the
    // element's namespace, that is a parse error. Similarly, if element has an xmlns:xlink attribute in the
    // XMLNS namespace whose value is not the XLink Namespace, that is a parse error.
    //
    // TODO(HTMLTREEBUILDER, HTML): create element resettable element
    // If element is a resettable element and not a form-associated custom element, then invoke its reset
    // algorithm. (This initializes the element's value and checkedness based on the element's attributes.)
    //
    // TODO(HTMLTREEBUILDER, HTML): create element form-associated element
    // If element is a form-associated element and not a form-associated custom element, the form element
    // pointer is not null, there is no template element on the stack of open elements, element is either not
    // listed or doesn't have a form attribute, and the intendedParent is in the same tree as the element
    // pointed to by the form element pointer, then associate element with the form element pointed to by the
    // form element pointer and set element's parser inserted flag.

    return element;
  }

  Ref<Element> HTMLTreeBuilder::CreateElement(const HTMLStackItem &item,
                                              RawPtr<ContainerNode> intendedParent) noexcept
  {
    assert(item.IsElement());

    auto &node = item.AsElement();
    auto &parent = intendedParent != nullptr ? *intendedParent : *node.ParentNode();
    return CreateElement(node.LocalName(), node.NamespaceURI(), item.Attributes(), parent);
  }

  void HTMLTreeBuilder::ReconstructActiveFormattingElements() noexcept
  {
    // If there are no entries in the list of active formatting elements, then there is nothing to
    // reconstruct; stop this algorithm.
    if (_activeFormattingElements.IsEmpty())
    {
      return;
    }

    // Let entry be the last (most recently added) element in the list of active formatting elements.
    auto entry = _activeFormattingElements.Last();

    // If the last (most recently added) entry in the list of active formatting elements is a marker, or if it
    // is an element that is in the stack of open elements, then there is nothing to reconstruct; stop this
    // algorithm.
    if (entry->IsMarker() || _openElementStack.Contains(entry->Item().AsElement()))
    {
      return;
    }

    // Rewind: If there are no entries before entry in the list of active formatting elements, then jump to
    // the step labeled create.
    // Let entry be the entry one earlier than entry in the list of active formatting elements.
    // If entry is neither a marker nor an element that is also in the stack of open elements, go to the step
    // labeled rewind.
    while (true)
    {
      if (entry == _activeFormattingElements.begin())
      {
        break;
      }

      auto previous = std::prev(entry);
      if (!previous->IsMarker() && !_openElementStack.Contains(previous->Item().AsElement()))
      {
        entry = previous;
        continue;
      }

      break;
    }

    // Advance: Let entry be the element one later than entry in the list of active formatting elements.
    // Create: Insert an HTML element for the token for which the element entry was created, to obtain new
    // element.
    // Replace the entry for entry in the list with an entry for new element.
    // If the entry for new element in the list of active formatting elements is not the last entry in the
    // list, return to the step labeled advance.
    while (true)
    {
      if (entry->IsMarker() || _openElementStack.Contains(entry->Item().AsElement()))
      {
        entry = std::next(entry);
      }

      auto &entryStackItem = entry->Item();
      auto newElement = CreateElement(entryStackItem);
      InsertElementAtAdjustedInsertionLocation(*newElement);
      _openElementStack.Push({entryStackItem.TagName(), entryStackItem.Namespace(), *newElement,
                              ParsedAttributeList(entryStackItem.Attributes())});
      entry->ReplaceItem(_openElementStack.Bottom());

      entry = std::next(entry);
      if (entry == _activeFormattingElements.end())
      {
        break;
      }
    }
  }

  void HTMLTreeBuilder::ClosePElement(const HTMLTokenAtom &token) noexcept
  {
    _openElementStack.GenerateImpliedEndTags(TagName::p);

    if (_openElementStack.Bottom().TagName() != TagName::p)
    {
      ParseError(token);
    }

    _openElementStack.PopUntilPopped(TagName::p, Namespace::HTML);
  }

#pragma endregion

#pragma region Adjust Attribute Algorithms

  void HTMLTreeBuilder::AdjustForeignAttributes(HTMLTokenAtom &token) noexcept
  {
    // TODO(HTMLTREEBUILDER, HTML): Adjust attributes for foreign elements.
  }

  void HTMLTreeBuilder::AdjustSVGAttributes(HTMLTokenAtom &token) noexcept
  {
    constexpr static Array<Array<DOMStringView, 2uz>, 58uz> attributesToAdjust = {
      Array<DOMStringView, 2> {u8"attributename", u8"attributeName"},
      Array<DOMStringView, 2> {u8"attributetype", u8"attributeType"},
      Array<DOMStringView, 2> {u8"basefrequency", u8"baseFrequency"},
      Array<DOMStringView, 2> {u8"baseprofile", u8"baseProfile"},
      Array<DOMStringView, 2> {u8"calcmode", u8"calcMode"},
      Array<DOMStringView, 2> {u8"clippathunits", u8"clipPathUnits"},
      Array<DOMStringView, 2> {u8"diffuseconstant", u8"diffuseConstant"},
      Array<DOMStringView, 2> {u8"edgemode", u8"edgeMode"},
      Array<DOMStringView, 2> {u8"filterunits", u8"filterUnits"},
      Array<DOMStringView, 2> {u8"glyphref", u8"glyphRef"},
      Array<DOMStringView, 2> {u8"gradienttransform", u8"gradientTransform"},
      Array<DOMStringView, 2> {u8"gradientunits", u8"gradientUnits"},
      Array<DOMStringView, 2> {u8"kernelmatrix", u8"kernelMatrix"},
      Array<DOMStringView, 2> {u8"kernelunitlength", u8"kernelUnitLength"},
      Array<DOMStringView, 2> {u8"keypoints", u8"keyPoints"},
      Array<DOMStringView, 2> {u8"keysplines", u8"keySplines"},
      Array<DOMStringView, 2> {u8"keytimes", u8"keyTimes"},
      Array<DOMStringView, 2> {u8"lengthadjust", u8"lengthAdjust"},
      Array<DOMStringView, 2> {u8"limitingconeangle", u8"limitingConeAngle"},
      Array<DOMStringView, 2> {u8"markerheight", u8"markerHeight"},
      Array<DOMStringView, 2> {u8"markerunits", u8"markerUnits"},
      Array<DOMStringView, 2> {u8"markerwidth", u8"markerWidth"},
      Array<DOMStringView, 2> {u8"maskcontentunits", u8"maskContentUnits"},
      Array<DOMStringView, 2> {u8"maskunits", u8"maskUnits"},
      Array<DOMStringView, 2> {u8"numoctaves", u8"numOctaves"},
      Array<DOMStringView, 2> {u8"pathlength", u8"pathLength"},
      Array<DOMStringView, 2> {u8"patterncontentunits", u8"patternContentUnits"},
      Array<DOMStringView, 2> {u8"patterntransform", u8"patternTransform"},
      Array<DOMStringView, 2> {u8"patternunits", u8"patternUnits"},
      Array<DOMStringView, 2> {u8"pointsatx", u8"pointsAtX"},
      Array<DOMStringView, 2> {u8"pointsaty", u8"pointsAtY"},
      Array<DOMStringView, 2> {u8"pointsatz", u8"pointsAtZ"},
      Array<DOMStringView, 2> {u8"preservealpha", u8"preserveAlpha"},
      Array<DOMStringView, 2> {u8"preserveaspectratio", u8"preserveAspectRatio"},
      Array<DOMStringView, 2> {u8"primitiveunits", u8"primitiveUnits"},
      Array<DOMStringView, 2> {u8"refx", u8"refX"},
      Array<DOMStringView, 2> {u8"refy", u8"refY"},
      Array<DOMStringView, 2> {u8"repeatcount", u8"repeatCount"},
      Array<DOMStringView, 2> {u8"repeatdur", u8"repeatDur"},
      Array<DOMStringView, 2> {u8"requiredextensions", u8"requiredExtensions"},
      Array<DOMStringView, 2> {u8"requiredfeatures", u8"requiredFeatures"},
      Array<DOMStringView, 2> {u8"specularconstant", u8"specularConstant"},
      Array<DOMStringView, 2> {u8"specularexponent", u8"specularExponent"},
      Array<DOMStringView, 2> {u8"spreadmethod", u8"spreadMethod"},
      Array<DOMStringView, 2> {u8"startoffset", u8"startOffset"},
      Array<DOMStringView, 2> {u8"stddeviation", u8"stdDeviation"},
      Array<DOMStringView, 2> {u8"stitchtiles", u8"stitchTiles"},
      Array<DOMStringView, 2> {u8"surfacescale", u8"surfaceScale"},
      Array<DOMStringView, 2> {u8"systemlanguage", u8"systemLanguage"},
      Array<DOMStringView, 2> {u8"tablevalues", u8"tableValues"},
      Array<DOMStringView, 2> {u8"targetx", u8"targetX"},
      Array<DOMStringView, 2> {u8"targety", u8"targetY"},
      Array<DOMStringView, 2> {u8"textlength", u8"textLength"},
      Array<DOMStringView, 2> {u8"viewbox", u8"viewBox"},
      Array<DOMStringView, 2> {u8"viewtarget", u8"viewTarget"},
      Array<DOMStringView, 2> {u8"xchannelselector", u8"xChannelSelector"},
      Array<DOMStringView, 2> {u8"ychannelselector", u8"yChannelSelector"},
      Array<DOMStringView, 2> {u8"zoomandpan", u8"zoomAndPan"},
    };

    auto &attributes = token.Attributes();
    for (auto &[attrName, adjustedAttrName] : attributesToAdjust)
    {
      auto attrIt = std::ranges::find_if(attributes, [attrName](const auto &attr)
                                         { return attr.NameView() == attrName; });

      if (attrIt != attributes.end())
      {
        attrIt->SetName(adjustedAttrName);
      }
    }
  }

  void HTMLTreeBuilder::AdjustMathMLAttributes(HTMLTokenAtom &token) noexcept
  {
    auto &attributes = token.Attributes();

    auto attrIt =
      std::ranges::find_if(attributes, [](const auto &attr) { return attr.NameView() == u8"definitionurl"; });

    if (attrIt != attributes.end())
    {
      attrIt->SetName(u8"definitionURL");
    }
  }

#pragma endregion

#pragma region Quirks Mode Algorithms

  bool HTMLTreeBuilder::IsQuirksModeDOCTYPE(const HTMLTokenAtom &token) const noexcept
  {
    assert(token.Type() == HTMLTokenType::DOCTYPE);

    if (token.IsForceQuirks() || token.Name() != u8"html")
    {
      return true;
    }

    auto &data = *token.DOCTYPEData();

    if (data.HasSystemIdentifier)
    {
      auto systemIdentifier = DOMStringView(data.SystemIdentifier.begin(), data.SystemIdentifier.end());
      if (StringAlgorithms::ASCIICaseInsensitiveMatch(
            systemIdentifier, u8"http://www.ibm.com/data/dtd/v11/ibmxhtml1-transitional.dtd"))
      {
        return true;
      }
    }

    if (data.HasPublicIdentifier)
    {
      auto publicIdentifier = DOMStringView(data.PublicIdentifier.begin(), data.PublicIdentifier.end());

      auto Matches = [publicIdentifier](DOMStringView identifier)
      {
        return StringAlgorithms::ASCIICaseInsensitiveMatch(publicIdentifier, identifier);
      };

      auto StartsWith = [publicIdentifier](DOMStringView identifier)
      {
        if (publicIdentifier.size() < identifier.size())
        {
          return false;
        }

        auto publicIdentifierPrefix = publicIdentifier.substr(0uz, identifier.size());
        return StringAlgorithms::ASCIICaseInsensitiveMatch(publicIdentifierPrefix, identifier);
      };

      if (Matches(u8"-//W3O//DTD W3 HTML Strict 3.0//EN//") || Matches(u8"-/W3C/DTD HTML 4.0 Transitional/EN")
          || Matches(u8"HTML"))
      {
        return true;
      }

      if (StartsWith(u8"+//Silmaril//dtd html Pro v0r11 19970101//")
          || StartsWith(u8"-//AS//DTD HTML 3.0 asWedit + extensions//")
          || StartsWith(u8"-//AdvaSoft Ltd//DTD HTML 3.0 asWedit + extensions//")
          || StartsWith(u8"-//IETF//DTD HTML 2.0 Level 1//")
          || StartsWith(u8"-//IETF//DTD HTML 2.0 Level 2//")
          || StartsWith(u8"-//IETF//DTD HTML 2.0 Strict Level 1//")
          || StartsWith(u8"-//IETF//DTD HTML 2.0 Strict Level 2//")
          || StartsWith(u8"-//IETF//DTD HTML 2.0 Strict//") || StartsWith(u8"-//IETF//DTD HTML 2.0//")
          || StartsWith(u8"-//IETF//DTD HTML 2.1E//") || StartsWith(u8"-//IETF//DTD HTML 3.0//")
          || StartsWith(u8"-//IETF//DTD HTML 3.2 Final//") || StartsWith(u8"-//IETF//DTD HTML 3.2//")
          || StartsWith(u8"-//IETF//DTD HTML 3//") || StartsWith(u8"-//IETF//DTD HTML Level 0//")
          || StartsWith(u8"-//IETF//DTD HTML Level 1//") || StartsWith(u8"-//IETF//DTD HTML Level 2//")
          || StartsWith(u8"-//IETF//DTD HTML Level 3//") || StartsWith(u8"-//IETF//DTD HTML Strict Level 0//")
          || StartsWith(u8"-//IETF//DTD HTML Strict Level 1//")
          || StartsWith(u8"-//IETF//DTD HTML Strict Level 2//")
          || StartsWith(u8"-//IETF//DTD HTML Strict Level 3//") || StartsWith(u8"-//IETF//DTD HTML Strict//")
          || StartsWith(u8"-//IETF//DTD HTML//") || StartsWith(u8"-//Metrius//DTD Metrius Presentational//")
          || StartsWith(u8"-//Microsoft//DTD Internet Explorer 2.0 HTML Strict//")
          || StartsWith(u8"-//Microsoft//DTD Internet Explorer 2.0 HTML//")
          || StartsWith(u8"-//Microsoft//DTD Internet Explorer 2.0 Tables//")
          || StartsWith(u8"-//Microsoft//DTD Internet Explorer 3.0 HTML Strict//")
          || StartsWith(u8"-//Microsoft//DTD Internet Explorer 3.0 HTML//")
          || StartsWith(u8"-//Microsoft//DTD Internet Explorer 3.0 Tables//")
          || StartsWith(u8"-//Netscape Comm. Corp.//DTD HTML//")
          || StartsWith(u8"-//Netscape Comm. Corp.//DTD Strict HTML//")
          || StartsWith(u8"-//O'Reilly and Associates//DTD HTML 2.0//")
          || StartsWith(u8"-//O'Reilly and Associates//DTD HTML Extended 1.0//")
          || StartsWith(u8"-//O'Reilly and Associates//DTD HTML Extended Relaxed 1.0//")
          || StartsWith(u8"-//SQ//DTD HTML 2.0 HoTMetaL + extensions//")
          || StartsWith(u8"-//SoftQuad Software//DTD HoTMetaL PRO 6.0::19990601::extensions to HTML 4.0//")
          || StartsWith(u8"-//SoftQuad//DTD HoTMetaL PRO 4.0::19971010::extensions to HTML 4.0//")
          || StartsWith(u8"-//Spyglass//DTD HTML 2.0 Extended//")
          || StartsWith(u8"-//Sun Microsystems Corp.//DTD HotJava HTML//")
          || StartsWith(u8"-//Sun Microsystems Corp.//DTD HotJava Strict HTML//")
          || StartsWith(u8"-//W3C//DTD HTML 3 1995-03-24//") || StartsWith(u8"-//W3C//DTD HTML 3.2 Draft//")
          || StartsWith(u8"-//W3C//DTD HTML 3.2 Final//") || StartsWith(u8"-//W3C//DTD HTML 3.2//")
          || StartsWith(u8"-//W3C//DTD HTML 3.2S Draft//") || StartsWith(u8"-//W3C//DTD HTML 4.0 Frameset//")
          || StartsWith(u8"-//W3C//DTD HTML 4.0 Transitional//")
          || StartsWith(u8"-//W3C//DTD HTML Experimental 19960712//")
          || StartsWith(u8"-//W3C//DTD HTML Experimental 970421//") || StartsWith(u8"-//W3C//DTD W3 HTML//")
          || StartsWith(u8"-//W3O//DTD W3 HTML 3.0//") || StartsWith(u8"-//WebTechs//DTD Mozilla HTML 2.0//")
          || StartsWith(u8"-//WebTechs//DTD Mozilla HTML//"))
      {
        return true;
      }

      if (!data.HasSystemIdentifier || data.SystemIdentifier.empty())
      {
        if (StartsWith(u8"-//W3C//DTD HTML 4.01 Frameset//EN")
            || StartsWith(u8"-//W3C//DTD HTML 4.01 Transitional//EN"))
        {
          return true;
        }
      }
    }

    return false;
  }

  bool HTMLTreeBuilder::IsLimitedQuirksModeDOCTYPE(const HTMLTokenAtom &token) const noexcept
  {
    assert(token.Type() == HTMLTokenType::DOCTYPE);

    auto &data = *token.DOCTYPEData();

    if (data.HasPublicIdentifier)
    {
      auto publicIdentifier = DOMStringView(data.PublicIdentifier.begin(), data.PublicIdentifier.end());

      auto StartsWith = [publicIdentifier](DOMStringView identifier)
      {
        if (publicIdentifier.size() < identifier.size())
        {
          return false;
        }

        auto publicIdentifierPrefix = publicIdentifier.substr(0uz, identifier.size());
        return StringAlgorithms::ASCIICaseInsensitiveMatch(publicIdentifierPrefix, identifier);
      };

      if (StartsWith(u8"-//W3C//DTD XHTML 1.0 Frameset//")
          || StartsWith(u8"-//W3C//DTD XHTML 1.0 Transitional//"))
      {
        return true;
      }

      if (data.HasSystemIdentifier && !data.SystemIdentifier.empty())
      {
        if (StartsWith(u8"-//W3C//DTD HTML 4.01 Frameset//")
            || StartsWith(u8"-//W3C//DTD HTML 4.01 Transitional//"))
        {
          return true;
        }
      }
    }

    return false;
  }

#pragma endregion

#pragma region Adoption Agency Algorithms

  void HTMLTreeBuilder::RunAdoptionAgency(HTMLTokenAtom &token) noexcept
  {
    // AAA(1): Let subject be token's tag name.
    auto subject = ParseTagName(token.Name().View());

    // AAA(2): If the current node is an HTML element whose tag name is subject, and the current node is not
    // in the list of active formatting elements, then pop the current node off the stack of open elements and
    // return.
    auto &currentNode = _openElementStack.Bottom();
    if (currentNode.Namespace() == Namespace::HTML && currentNode.TagName() == subject
        && !_activeFormattingElements.Contains(currentNode.Node()))
    {
      _openElementStack.Pop();
      return;
    }

    // AAA(3): Let outerLoopCounter be 0.
    size_t outerLoopCounter = 0uz;

    // AAA(4): While true:
    while (true)
    {
      // AAA(4.1): If outerLoopCounter is greater than or equal to 8, then return.
      if (outerLoopCounter >= 8uz)
      {
        return;
      }

      // AAA(4.2): Increment outerLoopCounter by 1.
      outerLoopCounter++;

      // AAA(4.3): Let formattingElement be the last element in the list of active formatting elements that:
      //     - is between the end of the list and the last marker in the list, if any, or the start of the
      //       list otherwise
      //     - has the tag name subject.
      // If there is no such element, then act as described in the "any other end tag" entry above and return.
      RawPtr<HTMLStackItem> formattingElement = _activeFormattingElements.FindFromLastMarker(subject);
      if (formattingElement == nullptr)
      {
        InBodyGenericEndTag(token, subject);
        return;
      }

      // NOTE: This is just for convenience; the stack item will only be destroyed right before the function
      // returns so the reference will remain valid regardless.
      auto &formattingElementNode = formattingElement->AsElement();

      // AAA(4.4): If formattingElement is not in the stack of open elements, then this is a parse error;
      // remove the element from the list, and return.
      if (!_openElementStack.Contains(formattingElementNode))
      {
        ParseError(token);
        _activeFormattingElements.Remove(formattingElementNode);
        return;
      }

      // AAA(4.5): If formattingElement is in the stack of open elements, but the element is not in scope,
      // then this is a parse error; return.
      if (!_openElementStack.HasElementInScope(formattingElementNode))
      {
        ParseError(token);
        return;
      }

      // AAA(4.6): If formattingElement is not the current node, this is a parse error. (But do not return.)
      if (&_openElementStack.Bottom().Node() != &formattingElementNode)
      {
        ParseError(token);
      }

      // AAA(4.7): Let furthestBlock be the topmost node in the stack of open elements that is lower in the
      // stack than formattingElement, and is an element in the special category. There might not be one.
      RawPtr<HTMLStackItem> furthestBlock = FurthestSpecialElementBlock(formattingElementNode);

      // AAA(4.8) - If there is no furthestBlock, then the UA must first pop all the nodes from the bottom of
      // the stack of open elements, from the current node up to and including formattingElement, then remove
      // formattingElement from the list of active formatting elements, and finally return.
      if (furthestBlock == nullptr)
      {
        _openElementStack.PopUntilPopped(formattingElementNode);
        _activeFormattingElements.Remove(formattingElementNode);
        return;
      }

      // NOTE: Save a reference to the node now in case the stack item gets destroyed. The node will still be
      // alive.
      auto &furthestBlockNode = furthestBlock->Node();

      // AAA(4.9): Let commonAncestor be the element immediately above formattingElement in the stack of open
      // elements.
      RawPtr<HTMLStackItem> commonAncestor = _openElementStack.EntryBefore(formattingElementNode);

      // AAA(4.10): Let a bookmark note the position of formattingElement in the list of active formatting
      // elements relative to the elements on either side of it in the list.
      auto bookmark = _activeFormattingElements.BookmarkFor(formattingElementNode);

      // AAA(4.11): Let node and lastNode be furthestBlock.
      RawPtr<ContainerNode> node = &furthestBlockNode;
      RawPtr<ContainerNode> lastNode = node;

      // AAA(4.12): Let innerLoopCounter be 0.
      size_t innerLoopCounter = 0uz;

      auto ElementAbove = [&](ContainerNode &node) -> RawPtr<ContainerNode>
      {
        RawPtr<HTMLStackItem> entry = _openElementStack.EntryBefore(node);
        return entry != nullptr ? &entry->Node() : nullptr;
      };
      RawPtr<ContainerNode> immediatelyAbove = ElementAbove(*node);

      // AAA(4.13): While true:
      while (true)
      {
        // AAA(4.13.1): Increment innerLoopCounter by 1.
        ++innerLoopCounter;

        // AAA(4.13.2): Let node be the element immediately above node in the stack of open elements, or if
        // node is no longer in the stack of open elements (e.g. because it got removed by this algorithm),
        // the element that was immediately above node in the stack of open elements before node was removed.
        node = immediatelyAbove;
        assert(node != nullptr);

        // NOTE: Fetch the next node now in case the stack item gets destroyed.
        immediatelyAbove = ElementAbove(*node);

        // AAA(4.13.3): If node is formattingElement, then break.
        if (node == &formattingElementNode)
        {
          break;
        }

        // AAA(4.13.4): If innerLoopCounter is greater than 3 and node is in the list of active formatting
        // elements, then remove node from the list of active formatting elements.
        if (innerLoopCounter > 3uz && _activeFormattingElements.Contains(*node))
        {
          _activeFormattingElements.RemoveAndUpdateBookmark(*node, bookmark);
        }

        // AAA(4.13.5): If node is not in the list of active formatting elements, then remove node from the
        // stack of open elements and continue.
        // NOTE: We retrieve the entry for node here in case the previous step removes it.
        RawPtr<FormattingListEntry> nodeFormattingEntry = _activeFormattingElements.Find(*node);
        if (nodeFormattingEntry == nullptr)
        {
          _openElementStack.Remove(*node);
          continue;
        }

        // AAA(4.13.6): Create an element for the token for which the element node was created, in the HTML
        // namespace, with commonAncestor as the intended parent; replace the entry for node in the list of
        // active formatting elements with an entry for the new element, replace the entry for node in the
        // stack of open elements with an entry for the new element, and let node be the new element.
        auto newElement = CreateElement(nodeFormattingEntry->Item(), &commonAncestor->Node());
        nodeFormattingEntry->Item().UpdateElement(*newElement);
        _openElementStack.Find(*node)->UpdateElement(*newElement);
        node = newElement.get();

        // AAA(4.13.7): If lastNode is furthestBlock, then move the aforementioned bookmark to be immediately
        // after the new node in the list of active formatting elements.
        if (lastNode == &furthestBlockNode)
        {
          _activeFormattingElements.MoveBookmarkAfter(bookmark, *nodeFormattingEntry);
        }

        // AAA(4.13.8): Append lastNode to node.
        (void)MutationAlgorithms::Append(*lastNode, *node);

        // AAA(4.13.9): Set lastNode to node.
        lastNode = node;
      }

      // AAA(4.14): Insert whatever lastNode ended up being in the previous step at the appropriate place for
      // inserting a node, but using commonAncestor as the override target.
      auto location = AppropriateInsertionLocation(commonAncestor);
      (void)MutationAlgorithms::Insert(*lastNode, *location.Parent, location.BeforeSibling);

      // AAA(4.15): Create an element for the token for which formattingElement was created, in the HTML
      // namespace, with furthestBlock as the intended parent.
      auto newElement = CreateElement(*formattingElement, &furthestBlockNode);

      // AAA(4.16): Take all of the child nodes of furthestBlock and append them to the element created in the
      // last step.
      while (auto *child = furthestBlockNode.FirstChild())
      {
        (void)MutationAlgorithms::Append(*child, *newElement);
      }

      // AAA(4.17): Append that new element to furthestBlock.
      (void)MutationAlgorithms::Append(*newElement, furthestBlockNode);

      auto newTagName = formattingElement->TagName();
      auto newTagNamespace = formattingElement->Namespace();
      auto newTagAttributes = ParsedAttributeList(formattingElement->Attributes());
      auto newFormattingElementItem =
        HTMLStackItem(newTagName, newTagNamespace, *newElement, newTagAttributes);
      auto newStackItem =
        HTMLStackItem(newTagName, newTagNamespace, *newElement, Krys::Move(newTagAttributes));

      // AAA(4.18): Remove formattingElement from the list of active formatting elements, and insert the new
      // element into the list of active formatting elements at the position of the aforementioned bookmark.
      _activeFormattingElements.RemoveAndUpdateBookmark(formattingElementNode, bookmark);
      _activeFormattingElements.Insert(Krys::Move(newFormattingElementItem), bookmark);

      /// AAA(4.19): Remove formattingElement from the stack of open elements, and insert the new element into
      /// the stack of open elements immediately below the position of furthestBlock in that stack.
      _openElementStack.Remove(formattingElementNode);
      _openElementStack.InsertBelow(Krys::Move(newStackItem), furthestBlockNode);
    }
  }

  RawPtr<HTMLStackItem>
    HTMLTreeBuilder::FurthestSpecialElementBlock(const Element &formattingElement) noexcept
  {
    // Get the index of the formatting element in the open element stack.
    size_t formattingIndex = std::distance(
      _openElementStack.begin(), std::ranges::find_if(_openElementStack, [&](const auto &item)
                                                      { return &item.Node() == &formattingElement; }));

    // Look for the topmost special element in the stack of open elements that is below the formatting
    // element.
    for (size_t i = formattingIndex + 1uz; i < _openElementStack.Size(); ++i)
    {
      auto &item = _openElementStack[i];
      if (IsSpecialElement(item.TagName(), item.Namespace()))
      {
        return &item;
      }
    }

    return nullptr;
  }

#pragma endregion

#pragma region IntegrationPoint Algorithms

  bool HTMLTreeBuilder::IsMathMLTextIntegrationPoint(const Element &element) const noexcept
  {
    if (element.NamespaceURI() == Namespaces::MathML)
    {
      auto name = ParseTagName(element.LocalName().View());
      switch (name)
      {
        case TagName::mi:
        case TagName::mo:
        case TagName::mn:
        case TagName::ms:
        case TagName::mtext:
        {
          return true;
        }
      }
    }

    return false;
  }

  bool HTMLTreeBuilder::IsHTMLIntegrationPoint(const Element &element) const noexcept
  {
    if (element.NamespaceURI() == Namespaces::MathML)
    {
      if (element.LocalName() == u8"annotation-xml")
      {
        auto attr = element.GetAttribute(u8"encoding");
        if (attr.has_value())
        {
          auto &encoding = attr.value();
          if (StringAlgorithms::ASCIICaseInsensitiveMatch(encoding, u8"text/html")
              || StringAlgorithms::ASCIICaseInsensitiveMatch(encoding, u8"application/xhtml+xml"))
          {
            return true;
          }
        }

        return false;
      }
    }
    else if (element.NamespaceURI() == Namespaces::SVG)
    {
      auto name = ParseTagName(element.LocalName().View());
      switch (name)
      {
        case TagName::desc:
        case TagName::title:
        case TagName::foreignObject:
        {
          return true;
        }
      }
    }

    return false;
  }

#pragma endregion
}
