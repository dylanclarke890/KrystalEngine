#include "Krystal.Booey/HTML/Parser/TreeBuilder.hpp"
#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Nodes/Comment.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentType.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLScriptElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableCellElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableRowElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableSectionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTemplateElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/Token.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/SVG/Elements/SVGScriptElement.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::html
{
  TreeBuilder::TreeBuilder(dom::Document &document, Tokenizer &tokenizer, Maybe<StackItem> &context) noexcept
      : _document(document), _tokenizer(tokenizer), _context(context)
  {
  }

  void TreeBuilder::ProcessToken(TokenAtom &&token) noexcept
  {
    if (_ignoreNextNewline)
    {
      _ignoreNextNewline = false;
      if (token.Type() == TokenType::Character && !token.Characters().empty()
          && token.Characters().front() == u8'\n')
      {
        token._data = token._data.substr(1uz);
        if (token.Characters().empty())
        {
          return;
        }
      }
    }

    if (ShouldProcessAccordingToRulesForHTMLContent(token))
    {
      ProcessAccordingToRulesForHTMLContent(krys::move(token));
    }
    else
    {
      ProcessAccordingToRulesForForeignContent(krys::move(token));
    }
  }

  void TreeBuilder::PushTemplateInsertionMode(InsertionMode mode) noexcept
  {
    _templateInsertionModes.push_back(mode);
  }

  bool TreeBuilder::ShouldProcessAccordingToRulesForHTMLContent(const TokenAtom &token) noexcept
  {
    if (_openElementStack.IsEmpty())
    {
      return true;
    }

    auto &adjustedCurrentNode = AdjustedCurrentNode();
    if (adjustedCurrentNode.NamespaceURI() == infra::Namespaces::HTML)
    {
      return true;
    }

    if (IsMathMLTextIntegrationPoint(adjustedCurrentNode))
    {
      if (token.Type() == TokenType::StartTag)
      {
        auto tagName = token.Name();
        if (tagName != u8"mglyph" && tagName != u8"malignmark")
        {
          return true;
        }
      }
      else if (token.Type() == TokenType::Character)
      {
        return true;
      }
    }

    if (adjustedCurrentNode.NamespaceURI() == infra::Namespaces::MathML
        && adjustedCurrentNode.LocalName() == u8"annotation-xml" && token.Type() == TokenType::StartTag
        && token.Name() == u8"svg")
    {
      return true;
    }

    if (IsHTMLIntegrationPoint(adjustedCurrentNode)
        && (token.Type() == TokenType::StartTag || token.Type() == TokenType::Character))
    {
      return true;
    }

    if (token.Type() == TokenType::EndOfFile)
    {
      return true;
    }

    return false;
  }

  void TreeBuilder::ProcessAccordingToRulesForHTMLContent(TokenAtom &&token) noexcept
  {
    switch (_insertionMode)
    {
      case InsertionMode::Initial:            InitialMode(krys::move(token)); break;
      case InsertionMode::BeforeHTML:         BeforeHTMLMode(krys::move(token)); break;
      case InsertionMode::BeforeHead:         BeforeHeadMode(krys::move(token)); break;
      case InsertionMode::InHead:             InHeadMode(krys::move(token)); break;
      case InsertionMode::InHeadNoscript:     InHeadNoscriptMode(krys::move(token)); break;
      case InsertionMode::AfterHead:          AfterHeadMode(krys::move(token)); break;
      case InsertionMode::InBody:             InBodyMode(krys::move(token)); break;
      case InsertionMode::Text:               TextMode(krys::move(token)); break;
      case InsertionMode::InTable:            InTableMode(krys::move(token)); break;
      case InsertionMode::InTableText:        InTableTextMode(krys::move(token)); break;
      case InsertionMode::InCaption:          InCaptionMode(krys::move(token)); break;
      case InsertionMode::InColumnGroup:      InColumnGroupMode(krys::move(token)); break;
      case InsertionMode::InTableBody:        InTableBodyMode(krys::move(token)); break;
      case InsertionMode::InRow:              InRowMode(krys::move(token)); break;
      case InsertionMode::InCell:             InCellMode(krys::move(token)); break;
      case InsertionMode::InTemplate:         InTemplateMode(krys::move(token)); break;
      case InsertionMode::AfterBody:          AfterBodyMode(krys::move(token)); break;
      case InsertionMode::InFrameset:         InFramesetMode(krys::move(token)); break;
      case InsertionMode::AfterFrameset:      AfterFramesetMode(krys::move(token)); break;
      case InsertionMode::AfterAfterBody:     AfterAfterBodyMode(krys::move(token)); break;
      case InsertionMode::AfterAfterFrameset: AfterAfterFramesetMode(krys::move(token)); break;
    }
  }

  void TreeBuilder::ProcessAccordingToRulesForForeignContent(TokenAtom &&token) noexcept
  {
    auto InvalidTag = [&]()
    {
      ParseError(token);

      while (true)
      {
        auto &currentNode = CurrentNode();
        if (!Is<dom::Element>(currentNode))
        {
          _openElementStack.Pop();
          continue;
        }

        if (!IsMathMLTextIntegrationPoint(currentNode) && !IsHTMLIntegrationPoint(currentNode)
            && currentNode.NamespaceURI() != infra::Namespaces::HTML)
        {
          _openElementStack.Pop();
          continue;
        }

        break;
      }

      ProcessAccordingToRulesForHTMLContent(krys::move(token));
    };

    auto ScriptTag = [&]()
    {
      // Pop the current node off the stack of open elements.
      _openElementStack.Pop();

      // (Skipped: there's no scripting, so there's no insertion point, script nesting level or parser pause
      // flag, and the SVG script element isn't processed.)
    };

    switch (token.Type())
    {
      case TokenType::Character:
      {
        if (!InsertLeadingWhitespaceAndNulls(token))
        {
          return;
        }

        InsertCharacters(token.ReleaseCharacters());
        _framesetOk = false;

        return;
      }
      case TokenType::Comment:
      {
        InsertComment(token.Comment());
        return;
      }
      case TokenType::ProcessingInstruction:
      {
        InsertProcessingInstruction(token);
        return;
      }
      case TokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case TokenType::StartTag:
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
        if (adjustedCurrentNode.NamespaceURI() == infra::Namespaces::MathML)
        {
          AdjustMathMLAttributes(token);
        }
        else if (adjustedCurrentNode.NamespaceURI() == infra::Namespaces::SVG)
        {
          constexpr static Array<Array<dom::DOMStringView, 2uz>, 37uz> svgTagNameAdjustments = {
            Array<dom::DOMStringView, 2uz> {u8"altglyph", u8"altGlyph"},
            Array<dom::DOMStringView, 2uz> {u8"altglyphdef", u8"altGlyphDef"},
            Array<dom::DOMStringView, 2uz> {u8"altglyphitem", u8"altGlyphItem"},
            Array<dom::DOMStringView, 2uz> {u8"animatecolor", u8"animateColor"},
            Array<dom::DOMStringView, 2uz> {u8"animatemotion", u8"animateMotion"},
            Array<dom::DOMStringView, 2uz> {u8"animatetransform", u8"animateTransform"},
            Array<dom::DOMStringView, 2uz> {u8"clippath", u8"clipPath"},
            Array<dom::DOMStringView, 2uz> {u8"feblend", u8"feBlend"},
            Array<dom::DOMStringView, 2uz> {u8"fecolormatrix", u8"feColorMatrix"},
            Array<dom::DOMStringView, 2uz> {u8"fecomponenttransfer", u8"feComponentTransfer"},
            Array<dom::DOMStringView, 2uz> {u8"fecomposite", u8"feComposite"},
            Array<dom::DOMStringView, 2uz> {u8"feconvolvematrix", u8"feConvolveMatrix"},
            Array<dom::DOMStringView, 2uz> {u8"fediffuselighting", u8"feDiffuseLighting"},
            Array<dom::DOMStringView, 2uz> {u8"fedisplacementmap", u8"feDisplacementMap"},
            Array<dom::DOMStringView, 2uz> {u8"fedistantlight", u8"feDistantLight"},
            Array<dom::DOMStringView, 2uz> {u8"fedropshadow", u8"feDropShadow"},
            Array<dom::DOMStringView, 2uz> {u8"feflood", u8"feFlood"},
            Array<dom::DOMStringView, 2uz> {u8"fefunca", u8"feFuncA"},
            Array<dom::DOMStringView, 2uz> {u8"fefuncb", u8"feFuncB"},
            Array<dom::DOMStringView, 2uz> {u8"fefuncg", u8"feFuncG"},
            Array<dom::DOMStringView, 2uz> {u8"fefuncr", u8"feFuncR"},
            Array<dom::DOMStringView, 2uz> {u8"fegaussianblur", u8"feGaussianBlur"},
            Array<dom::DOMStringView, 2uz> {u8"feimage", u8"feImage"},
            Array<dom::DOMStringView, 2uz> {u8"femerge", u8"feMerge"},
            Array<dom::DOMStringView, 2uz> {u8"femergenode", u8"feMergeNode"},
            Array<dom::DOMStringView, 2uz> {u8"femorphology", u8"feMorphology"},
            Array<dom::DOMStringView, 2uz> {u8"feoffset", u8"feOffset"},
            Array<dom::DOMStringView, 2uz> {u8"fepointlight", u8"fePointLight"},
            Array<dom::DOMStringView, 2uz> {u8"fespecularlighting", u8"feSpecularLighting"},
            Array<dom::DOMStringView, 2uz> {u8"fespotlight", u8"feSpotLight"},
            Array<dom::DOMStringView, 2uz> {u8"fetile", u8"feTile"},
            Array<dom::DOMStringView, 2uz> {u8"feturbulence", u8"feTurbulence"},
            Array<dom::DOMStringView, 2uz> {u8"foreignobject", u8"foreignObject"},
            Array<dom::DOMStringView, 2uz> {u8"glyphref", u8"glyphRef"},
            Array<dom::DOMStringView, 2uz> {u8"lineargradient", u8"linearGradient"},
            Array<dom::DOMStringView, 2uz> {u8"radialgradient", u8"radialGradient"},
            Array<dom::DOMStringView, 2uz> {u8"textpath", u8"textPath"},
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
        InsertForeignElement(krys::move(token), adjustedCurrentNode.NamespaceURI(), false);

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

        break;
      }
      case TokenType::EndTag:
      {
        if (token.Name() == u8"br" || token.Name() == u8"p")
        {
          InvalidTag();
          return;
        }

        if (token.Name() == u8"script" && Is<svg::SVGScriptElement>(CurrentNode()))
        {
          ScriptTag();
          return;
        }

        // Any other end tag:
        // 1. Initialize node to be the current node (the bottommost node of the stack).
        auto nodeIt = _openElementStack.rbegin();

        // 2. If node's tag name, converted to ASCII lowercase, is not the same as the tag name of the token,
        //    then this is a parse error.
        if (!infra::StringAlgorithms::ASCIICaseInsensitiveMatch(nodeIt->Element().LocalName().View(),
                                                                token.Name().View()))
        {
          ParseError(token);
        }

        while (true)
        {
          // 3. Loop: If node is the topmost element in the stack of open elements, then return.
          if (&nodeIt->Element() == &_openElementStack.Top().Element())
          {
            return;
          }

          // 4. If node's tag name, converted to ASCII lowercase, is the same as the tag name of the token,
          //    pop elements from the stack of open elements until node has been popped from the stack, and
          //    then return.
          if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(nodeIt->Element().LocalName().View(),
                                                                 token.Name().View()))
          {
            _openElementStack.PopUntilPopped(nodeIt->Element());
            return;
          }

          // 5. Set node to the previous entry in the stack of open elements.
          ++nodeIt;

          // 6. If node is not an element in the HTML namespace, return to the step labeled loop.
          if (nodeIt->Element().NamespaceURI() != infra::Namespaces::HTML)
          {
            continue;
          }

          // 7. Otherwise, process the token according to the rules given in the section corresponding to the
          //    current insertion mode in HTML content.
          ProcessAccordingToRulesForHTMLContent(krys::move(token));
          return;
        }
      }
    }
  }

  dom::Element &TreeBuilder::CurrentNode() noexcept
  {
    return _openElementStack.Bottom().Element();
  }

  dom::Element &TreeBuilder::AdjustedCurrentNode() noexcept
  {
    if (_context.has_value() && _openElementStack.Size() == 1uz)
    {
      return _context->Element();
    }
    else
    {
      return CurrentNode();
    }
  }

  void TreeBuilder::ParseError(const TokenAtom &token) noexcept
  {
    // Parse errors aren't reported (see DIFFERENCES.md).
    (void)token;
  }

#pragma region InsertionMode Algorithms

  void TreeBuilder::ResetInsertionModeAppropriately() noexcept
  {
    // 1. Let last be false.
    bool last = false;

    // 2. Let node be the last node in the stack of open elements.
    auto *stackElement = &_openElementStack.Bottom();
    auto *node = &stackElement->Element();

    while (true)
    {
      // 3. Loop: If node is the first node in the stack of open elements, then set last to true, and, if the
      //    parser's fragment context element is non-null, then set node to that element.
      if (node == &_openElementStack.Top().Element())
      {
        last = true;

        if (_context.has_value())
        {
          stackElement = &*_context;
          node = &_context->Element();
        }
      }

      if (stackElement->Namespace() == Namespace::HTML)
      {
        switch (stackElement->TagName())
        {
          // 4. If node is a td or th element and last is false, then switch the insertion mode to "in cell"
          // and
          //    return.
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
          // 5. If node is a tr element, then switch the insertion mode to "in row" and return.
          case TagName::tr:
          {
            _insertionMode = InsertionMode::InRow;
            return;
          }
          // 6. If node is a tbody, thead, or tfoot element, then switch the insertion mode to "in table body"
          //    and return.
          case TagName::tbody:
          case TagName::thead:
          case TagName::tfoot:
          {
            _insertionMode = InsertionMode::InTableBody;
            return;
          }
          // 7. If node is a caption element, then switch the insertion mode to "in caption" and return.
          case TagName::caption:
          {
            _insertionMode = InsertionMode::InCaption;
            return;
          }
          // 8. If node is a colgroup element, then switch the insertion mode to "in column group" and return.
          case TagName::colgroup:
          {
            _insertionMode = InsertionMode::InColumnGroup;
            return;
          }
          // 9. If node is a table element, then switch the insertion mode to "in table" and return.
          case TagName::table:
          {
            _insertionMode = InsertionMode::InTable;
            return;
          }
          // 10. If node is a template element, then switch the insertion mode to the current template
          //     insertion mode and return.
          case TagName::template_:
          {
            _insertionMode = CurrentTemplateInsertionMode();
            return;
          }
          // 11. If node is a head element and last is false, then switch the insertion mode to "in head" and
          //     return.
          case TagName::head:
          {
            if (!last)
            {
              _insertionMode = InsertionMode::InHead;
              return;
            }

            break;
          }
          // 12. If node is a body element, then switch the insertion mode to "in body" and return.
          case TagName::body:
          {
            _insertionMode = InsertionMode::InBody;
            return;
          }
          // 13. If node is a frameset element, then switch the insertion mode to "in frameset" and return.
          case TagName::frameset:
          {
            _insertionMode = InsertionMode::InFrameset;
            return;
          }
          // 14. If node is an html element, run these substeps:
          case TagName::html:
          {
            // 14a. If the head element pointer is null, switch the insertion mode to "before head" and
            // return.
            if (_head == nullptr)
            {
              _insertionMode = InsertionMode::BeforeHead;
            }
            // 14b. Otherwise, the head element pointer is not null, switch the insertion mode to "after head"
            // and
            //      return.
            else
            {
              _insertionMode = InsertionMode::AfterHead;
            }
            return;
          }
        }
      }

      // 15. If last is true, then switch the insertion mode to "in body" and return.
      if (last)
      {
        _insertionMode = InsertionMode::InBody;
        return;
      }

      // 16. Let node now be the node before node in the stack of open elements.
      // 17. Return to the step labeled loop.
      stackElement = _openElementStack.EntryBefore(*node);
      krys_debug_assert(stackElement != nullptr);

      node = &stackElement->Element();
    }
  }

  InsertionMode TreeBuilder::CurrentTemplateInsertionMode() const noexcept
  {
    krys_debug_assert(!_templateInsertionModes.empty());
    return _templateInsertionModes.back();
  }

  void TreeBuilder::InitialMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::Character:
      {
        if (SkipLeadingWhitespace(token))
        {
          break; // Reprocess remaining characters in the BeforeHTML insertion mode.
        }

        return;
      }
      case TokenType::Comment:
      {
        AppendCommentToDocument(token.Comment());
        return;
      }
      case TokenType::ProcessingInstruction:
      {
        InsertProcessingInstruction(token, InsertionLocation {&_document, nullptr});
        return;
      }
      case TokenType::DOCTYPE:
      {
        auto &docTypeData = *token.DOCTYPEData();

        auto name = dom::DOMString(token.Name().View());
        if (name != u8"html")
        {
          ParseError(token);
        }

        auto publicIdentifier =
          dom::DOMString(docTypeData.PublicIdentifier.begin(), docTypeData.PublicIdentifier.end());
        if (docTypeData.HasPublicIdentifier && !publicIdentifier.empty())
        {
          ParseError(token);
        }

        auto systemIdentifier =
          dom::DOMString(docTypeData.SystemIdentifier.begin(), docTypeData.SystemIdentifier.end());
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

        if (!dom::DocumentAlgorithms::IsIFrameSrcdocDocument(_document)
            && !_document._parserCannotChangeTheMode)
        {
          if (IsQuirksModeDOCTYPE(token))
          {
            _document.SetQuirksMode(dom::QuirksMode::Quirks);
          }
          else if (IsLimitedQuirksModeDOCTYPE(token))
          {
            _document.SetQuirksMode(dom::QuirksMode::LimitedQuirks);
          }
        }

        _insertionMode = InsertionMode::BeforeHTML;
        return;
      }
    }

    if (!dom::DocumentAlgorithms::IsIFrameSrcdocDocument(_document))
    {
      ParseError(token);
    }

    if (!_document._parserCannotChangeTheMode)
    {
      _document.SetQuirksMode(dom::QuirksMode::Quirks);
    }

    _insertionMode = InsertionMode::BeforeHTML;
    BeforeHTMLMode(krys::move(token));
  }

  void TreeBuilder::BeforeHTMLMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case TokenType::Comment:
      {
        AppendCommentToDocument(token.Comment());
        return;
      }
      case TokenType::ProcessingInstruction:
      {
        InsertProcessingInstruction(token, InsertionLocation {&_document, nullptr});
        return;
      }
      case TokenType::Character:
      {
        if (SkipLeadingWhitespace(token))
        {
          break; // Reprocess remaining characters in the BeforeHead insertion mode.
        }

        return;
      }
      case TokenType::StartTag:
      {
        if (token.Name() == u8"html")
        {
          auto html = CreateElement(token.Name(), infra::Namespaces::HTML, token.Attributes(), _document);
          _document.AppendChild(*html);
          _openElementStack.Push({TagName::html, Namespace::HTML, *html, krys::move(token.Attributes())});

          _insertionMode = InsertionMode::BeforeHead;
          return;
        }

        break;
      }
      case TokenType::EndTag:
      {
        if (token.Name() == u8"head" || token.Name() == u8"body" || token.Name() == u8"html"
            || token.Name() == u8"br")
        {
          break;
        }

        ParseError(token);
        return; // ignore the token
      }
    }

    auto html = dom::ElementFactory::Create(
      _document, dom::QualifiedName(infra::Namespaces::HTML, dom::DOMStringAtom::Null(), u8"html",
                                    TagName::html, Namespace::HTML));
    _document.AppendChild(*html);
    _openElementStack.Push({TagName::html, Namespace::HTML, *html, {}});

    _insertionMode = InsertionMode::BeforeHead;
    BeforeHeadMode(krys::move(token));
  }

  void TreeBuilder::BeforeHeadMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::Character:
      {
        if (SkipLeadingWhitespace(token))
        {
          break; // Reprocess remaining characters in the InHead insertion mode.
        }

        return;
      }
      case TokenType::Comment:
      {
        InsertComment(token.Comment());
        return;
      }
      case TokenType::ProcessingInstruction:
      {
        InsertProcessingInstruction(token);
        return;
      }
      case TokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case TokenType::StartTag:
      {
        if (token.Name() == u8"html")
        {
          // Process the token using the rules for the "in body" insertion mode.
          InBodyMode(krys::move(token));
          return;
        }

        if (token.Name() == u8"head")
        {
          _head = InsertHTMLElement(krys::move(token));
          _insertionMode = InsertionMode::InHead;
          return;
        }

        break;
      }
      case TokenType::EndTag:
      {
        if (token.Name() == u8"head" || token.Name() == u8"body" || token.Name() == u8"html"
            || token.Name() == u8"br")
        {
          break;
        }

        ParseError(token);
        return; // ignore the token
      }
    }

    _head = dom::ElementFactory::Create(_document, dom::QualifiedName(infra::Namespaces::HTML,
                                                                      dom::DOMStringAtom::Null(), u8"head",
                                                                      TagName::head, Namespace::HTML));
    InsertElementAtAdjustedInsertionLocation(*_head);
    _openElementStack.Push({TagName::head, Namespace::HTML, *_head, {}});

    _insertionMode = InsertionMode::InHead;
    InHeadMode(krys::move(token));
  }

  void TreeBuilder::InHeadMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::Character:
      {
        if (InsertLeadingWhitespace(token))
        {
          break; // Reprocess remaining characters in the AfterHead insertion mode.
        }

        return;
      }
      case TokenType::Comment:
      {
        InsertComment(token.Comment());
        return;
      }
      case TokenType::ProcessingInstruction:
      {
        InsertProcessingInstruction(token);
        return;
      }
      case TokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case TokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::html:
          {
            InBodyMode(krys::move(token));
            return;
          }
          case TagName::base:
          case TagName::basefont:
          case TagName::bgsound:
          case TagName::link:
          {
            InsertHTMLElement(krys::move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();
            return;
          }
          case TagName::meta:
          {
            InsertHTMLElement(krys::move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();

            // If the active speculative HTML parser is null:
            // If the element has a charset attribute, and getting an encoding from its value results in an
            // encoding, and the confidence is currently tentative, then change the encoding to the resulting
            // encoding.
            // Otherwise, if the element has an http-equiv attribute whose value is an ASCII case-insensitive
            // match for "Content-Type", and the element has a content attribute, and applying the algorithm
            // for extracting a character encoding from a meta element to that attribute's value returns an
            // encoding, and the confidence is currently tentative, then change the encoding to the extracted
            // encoding.
            // (Skipped: documents are UTF-8 only; see DIFFERENCES.md.)

            return;
          }
          case TagName::title:
          {
            ParseGenericRCDATATextElement(krys::move(token));
            return;
          }
          case TagName::noscript:
          {
            if (_scriptingMode == ParserScriptingMode::Disabled)
            {
              InsertHTMLElement(krys::move(token));
              _insertionMode = InsertionMode::InHeadNoscript;
            }
            else
            {
              ParseGenericRawTextElement(krys::move(token));
            }
            return;
          }
          case TagName::noframes:
          case TagName::style:
          {
            ParseGenericRawTextElement(krys::move(token));
            return;
          }
          case TagName::script:
          {
            auto adjustedInsertionLocation = AppropriatePlaceToInsertNode();

            auto element = CreateElement(token.Name(), infra::Namespaces::HTML, token.Attributes(),
                                         *adjustedInsertionLocation.Parent);
            auto &script = Downcast<HTMLScriptElement>(*element);

            if (_scriptingMode != ParserScriptingMode::Fragment)
            {
              script._parserDocument = &_document;
              script._forceAsync = false;
            }

            if (_scriptingMode == ParserScriptingMode::Inert)
            {
              script._alreadyStarted = true;
            }

            // (There's no document.write(), so the script element's already started is never set for it.)

            InsertElementAtAdjustedInsertionLocation(script, adjustedInsertionLocation);
            _openElementStack.Push(
              {TagName::script, Namespace::HTML, script, krys::move(token.Attributes())});

            _tokenizer.State(TokenizerState::ScriptData);
            _originalInsertionMode = _insertionMode;
            _insertionMode = InsertionMode::Text;
            return;
          }
          case TagName::template_:
          {
            InsertTemplateElement(krys::move(token));
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
      case TokenType::EndTag:
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
            if (!_openElementStack.ContainsTemplateElement())
            {
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.GenerateImpliedEndTagsThoroughly();

            auto &currentNode = _openElementStack.Bottom();
            if (!(currentNode.Namespace() == Namespace::HTML && currentNode.TagName() == TagName::template_))
            {
              ParseError(token);
            }

            // 3. Let template be the last template element in the stack of open elements.
            // 4. If template's insertion target is not null:
            auto templateIt = std::ranges::find_if(_openElementStack.rbegin(), _openElementStack.rend(),
                                                   [](const StackItem &item)
                                                   { return Is<HTMLTemplateElement>(item.Element()); });
            krys_debug_assert(templateIt != _openElementStack.rend());
            auto &templateElement = Downcast<HTMLTemplateElement>(templateIt->Element());
            if (templateElement.InsertionTarget())
            {
              // 4a. Let start be template's insertion start marker.
              // 4b. Let end be template's insertion end marker.
              // 4c. Assert: start is not null.
              Ref<dom::Node> start = ShareRef(*templateElement.InsertionStartMarker());
              RefPtr<dom::Node> end = ShareRefPtr(templateElement.InsertionEndMarker());

              // 4d. If start's parent is not null, then remove start.
              if (start->ParentNode())
              {
                (void)dom::MutationAlgorithms::Remove(*start);
              }

              // 4e. If end is not null and end's parent is not null, then remove end.
              if (end && end->ParentNode())
              {
                (void)dom::MutationAlgorithms::Remove(*end);
              }

              // 4f. Set template's insertion target to null.
              // 4g. Set template's insertion start marker to null.
              // 4h. Set template's insertion end marker to null.
              templateElement.ClearInsertionTarget();
            }

            _openElementStack.PopUntilPopped(TagName::template_, Namespace::HTML);
            _activeFormattingElements.ClearUpToLastMarker();

            _templateInsertionModes.pop_back();
            ResetInsertionModeAppropriately();

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
    AfterHeadMode(krys::move(token));
  }

  void TreeBuilder::InHeadNoscriptMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case TokenType::Character:
      {
        if (InsertLeadingWhitespace(token))
        {
          break; // Reprocess remaining characters in the InHead insertion mode.
        }

        return;
      }
      case TokenType::Comment:
      case TokenType::ProcessingInstruction:
      {
        InHeadMode(krys::move(token));
        return;
      }
      case TokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::html:
          {
            InBodyMode(krys::move(token));
            return;
          }
          case TagName::basefont:
          case TagName::bgsound:
          case TagName::link:
          case TagName::meta:
          case TagName::noframes:
          case TagName::style:
          {
            InHeadMode(krys::move(token));
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
      case TokenType::EndTag:
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

        ParseError(token);
        return; // ignore the token
      }
    }

    ParseError(token);
    _openElementStack.Pop();
    _insertionMode = InsertionMode::InHead;
    InHeadMode(krys::move(token));
  }

  void TreeBuilder::AfterHeadMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::Character:
      {
        if (InsertLeadingWhitespace(token))
        {
          break; // Reprocess remaining characters in the InBody insertion mode.
        }

        return;
      }
      case TokenType::Comment:
      {
        InsertComment(token.Comment());
        return;
      }
      case TokenType::ProcessingInstruction:
      {
        InsertProcessingInstruction(token);
        return;
      }
      case TokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case TokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::html:
          {
            InBodyMode(krys::move(token));
            return;
          }
          case TagName::body:
          {
            InsertHTMLElement(krys::move(token));
            _framesetOk = false;

            _insertionMode = InsertionMode::InBody;
            return;
          }
          case TagName::frameset:
          {
            InsertHTMLElement(krys::move(token));

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
            // 1. Parse error.
            ParseError(token);

            // 2. Push the node pointed to by the head element pointer onto the stack of open elements.
            _openElementStack.Push({TagName::head, Namespace::HTML, *_head, {}});

            // 3. Process the token using the rules for the "in head" insertion mode.
            InHeadMode(krys::move(token));

            // 4. Remove the node pointed to by the head element pointer from the stack of open elements. (It
            // might
            //    not be the current node at this point.)
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
      case TokenType::EndTag:
      {
        if (token.Name() == u8"template")
        {
          InHeadMode(krys::move(token));
          return;
        }

        if (token.Name() == u8"body" || token.Name() == u8"html" || token.Name() == u8"br")
        {
          break;
        }

        ParseError(token);
        return; // ignore the token
      }
    }

    auto body = dom::ElementFactory::Create(
      _document, dom::QualifiedName(infra::Namespaces::HTML, dom::DOMStringAtom::Null(), u8"body",
                                    TagName::body, Namespace::HTML));
    InsertElementAtAdjustedInsertionLocation(*body);
    _openElementStack.Push({TagName::body, Namespace::HTML, *body, {}});

    _framesetOk = true;
    _insertionMode = InsertionMode::InBody;
    InBodyMode(krys::move(token));
  }

  void TreeBuilder::InBodyMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::Character:
      {
        if (!RemoveNulls(token))
        {
          return;
        }

        ReconstructActiveFormattingElements();

        if (!InsertLeadingWhitespace(token))
        {
          return;
        }

        InsertCharacters(token.ReleaseCharacters());
        _framesetOk = false;

        return;
      }
      case TokenType::Comment:
      {
        InsertComment(token.Comment());
        return;
      }
      case TokenType::ProcessingInstruction:
      {
        InsertProcessingInstruction(token);
        return;
      }
      case TokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case TokenType::StartTag:
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

            // 1. Let htmlElement be the top element of the stack of open elements.
            // 2. For each attribute on the token: [...] if htmlElement does not have an attribute whose name
            //    is attribute's name, then add attribute's name and its corresponding value to htmlElement.
            krys_debug_assert(Is<HTMLHtmlElement>(_openElementStack.Top().Element()));
            MergeAttributes(_openElementStack.Top().Element(), token.Attributes());
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
            InHeadMode(krys::move(token));
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

            // 1. Set the frameset-ok flag to "not ok".
            _framesetOk = false;

            // 2. Let bodyElement be the body element (the second element) on the stack of open elements.
            // 3. For each attribute on the token: [...] if bodyElement does not have an attribute whose name
            //    is attribute's name, then add attribute's name and its corresponding value to bodyElement.
            krys_debug_assert(Is<HTMLBodyElement>(_openElementStack[1].Element()));
            MergeAttributes(_openElementStack[1].Element(), token.Attributes());
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

            auto &bodyElement = Downcast<HTMLBodyElement>(_openElementStack[1].Element());
            (void)dom::MutationAlgorithms::Remove(bodyElement);
            _openElementStack.PopUntil(_openElementStack.Top().Element());
            InsertHTMLElement(krys::move(token));

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

            InsertHTMLElement(krys::move(token));
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

            InsertHTMLElement(krys::move(token));
            return;
          }
          case TagName::pre:
          case TagName::listing:
          {
            if (_openElementStack.HasElementInButtonScope(TagName::p))
            {
              ClosePElement(token);
            }

            InsertHTMLElement(krys::move(token));

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

            auto element = InsertHTMLElement(krys::move(token));
            if (!containsTemplateElement)
            {
              _form = krys::move(element);
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

            InsertHTMLElement(krys::move(token));
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

            InsertHTMLElement(krys::move(token));
            return;
          }
          case TagName::plaintext:
          {
            if (_openElementStack.HasElementInButtonScope(TagName::p))
            {
              ClosePElement(token);
            }

            InsertHTMLElement(krys::move(token));

            _tokenizer.State(TokenizerState::PLAINTEXT);
            return;
          }
          case TagName::button:
          {
            if (_openElementStack.HasElementInButtonScope(TagName::button))
            {
              ParseError(token);
              _openElementStack.GenerateImpliedEndTags();
              _openElementStack.PopUntilPopped(TagName::button, Namespace::HTML);
            }

            ReconstructActiveFormattingElements();
            InsertHTMLElement(krys::move(token));

            _framesetOk = false;

            return;
          }
          case TagName::a:
          {
            // If an a element is in the formatting list between the end and the last marker, this is a
            // parse error; run the adoption agency algorithm, then remove that element from the list and
            // stack of open elements if the adoption agency algorithm didn't already remove it.
            if (auto *existing = _activeFormattingElements.FindFromLastMarker(TagName::a, Namespace::HTML))
            {
              ParseError(token);

              // Stack item might be removed by AAA so we need to keep a reference to the node here.
              auto &existingNode = existing->Element();

              RunAdoptionAgency(token);

              _activeFormattingElements.Remove(existingNode);
              _openElementStack.Remove(existingNode);
            }

            ReconstructActiveFormattingElements();
            InsertHTMLElement(krys::move(token));

            auto &a = _openElementStack.Bottom();
            _activeFormattingElements.PushElement(
              StackItem(a.TagName(), a.Namespace(), a.Element(), a.Attributes()));

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
            InsertHTMLElement(krys::move(token));

            auto &fmtOpenItem = _openElementStack.Bottom();
            ParsedAttributeList fmtAttrsCopy = fmtOpenItem.Attributes();
            _activeFormattingElements.PushElement(StackItem(fmtOpenItem.TagName(), fmtOpenItem.Namespace(),
                                                            fmtOpenItem.Element(), krys::move(fmtAttrsCopy)));

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

            InsertHTMLElement(krys::move(token));

            auto &nobrOpenItem = _openElementStack.Bottom();
            ParsedAttributeList nobrAttrsCopy = nobrOpenItem.Attributes();
            _activeFormattingElements.PushElement(StackItem(nobrOpenItem.TagName(), nobrOpenItem.Namespace(),
                                                            nobrOpenItem.Element(),
                                                            krys::move(nobrAttrsCopy)));

            return;
          }
          case TagName::applet:
          case TagName::marquee:
          case TagName::object:
          {
            ReconstructActiveFormattingElements();
            InsertHTMLElement(krys::move(token));
            _activeFormattingElements.PushMarker();
            _framesetOk = false;

            return;
          }
          case TagName::table:
          {
            if (_document._quirksMode != dom::QuirksMode::Quirks
                && _openElementStack.HasElementInButtonScope(TagName::p))
            {
              ClosePElement(token);
            }

            InsertHTMLElement(krys::move(token));
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
            InsertHTMLElement(krys::move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();
            _framesetOk = false;

            return;
          }
          case TagName::input:
          {
            // If the parser's fragment context element is a select element (fragment case): parse error;
            // ignore the token.
            if (_context.has_value() && Is<HTMLSelectElement>(_context->Element()))
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
                hasHiddenType =
                  infra::StringAlgorithms::ASCIICaseInsensitiveMatch(attr.ValueView(), u8"hidden");
                break;
              }
            }

            ReconstructActiveFormattingElements();
            InsertHTMLElement(krys::move(token));
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
            InsertHTMLElement(krys::move(token));
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

            InsertHTMLElement(krys::move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();
            _framesetOk = false;

            return;
          }
          case TagName::image:
          {
            ParseError(token);
            token._name = u8"img"; // reprocess the token as an img tag.

            InBodyMode(krys::move(token));
            return;
          }
          case TagName::textarea:
          {
            InsertHTMLElement(krys::move(token));

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

            ParseGenericRawTextElement(krys::move(token));
            return;
          }
          case TagName::iframe:
          {
            _framesetOk = false;

            ParseGenericRawTextElement(krys::move(token));
            return;
          }
          case TagName::noembed:
          {
            ParseGenericRawTextElement(krys::move(token));
            return;
          }
          case TagName::noscript:
          {
            if (_scriptingMode != ParserScriptingMode::Disabled)
            {
              ParseGenericRawTextElement(krys::move(token));
              return;
            }

            // Scripting is disabled: treat as 'any other start tag'.
            ReconstructActiveFormattingElements();
            InsertHTMLElement(krys::move(token));
            return;
          }
          case TagName::select:
          {
            // If the parser's fragment context element is a select element (fragment case): parse error;
            // ignore the token.
            if (_context.has_value() && Is<HTMLSelectElement>(_context->Element()))
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
            InsertHTMLElement(krys::move(token));
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
            InsertHTMLElement(krys::move(token));

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
            InsertHTMLElement(krys::move(token));

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

            InsertHTMLElement(krys::move(token));
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

            InsertHTMLElement(krys::move(token));
            return;
          }
          case TagName::math:
          {
            ReconstructActiveFormattingElements();
            AdjustMathMLAttributes(token);
            AdjustForeignAttributes(token);
            InsertForeignElement(krys::move(token), infra::Namespaces::MathML, false);

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
            InsertForeignElement(krys::move(token), infra::Namespaces::SVG, false);

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
            InsertHTMLElement(krys::move(token));

            return;
          }
        }
      }
      case TokenType::EndTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::template_:
          {
            InHeadMode(krys::move(token));
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
            AfterBodyMode(krys::move(token)); // reprocess the token.

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
              RefPtr<HTMLFormElement> form = krys::move(_form);
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
              auto pElement = dom::ElementFactory::Create(
                _document, dom::QualifiedName(infra::Namespaces::HTML, dom::DOMStringAtom::Null(), u8"p",
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
            if (!_openElementStack.HasElementInScope(tagName))
            {
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.GenerateImpliedEndTags(tagName);

            auto &currentNode = _openElementStack.Bottom();
            if (currentNode.Namespace() != Namespace::HTML || currentNode.TagName() != tagName)
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

            auto &currentNode = _openElementStack.Bottom();
            if (currentNode.Namespace() != Namespace::HTML || currentNode.TagName() != tagName)
            {
              ParseError(token);
            }

            while (true)
            {
              auto tag = _openElementStack.Bottom().TagName();
              auto ns = _openElementStack.Bottom().Namespace();

              _openElementStack.Pop();

              if (ns == Namespace::HTML
                  && (tag == TagName::h1 || tag == TagName::h2 || tag == TagName::h3 || tag == TagName::h4
                      || tag == TagName::h5 || tag == TagName::h6))
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

            // Act as if this were a "br" start tag with no attributes.
            token._attributes.clear();
            ReconstructActiveFormattingElements();
            InsertHTMLElement(krys::move(token));
            _openElementStack.Pop();

            _framesetOk = false;
            return;
          }
          case TagName::applet:
          case TagName::marquee:
          case TagName::object:
          {
            if (!_openElementStack.HasElementInScope(tagName))
            {
              ParseError(token);
              return; // ignore the token
            }

            _openElementStack.GenerateImpliedEndTags();

            auto &bottom = _openElementStack.Bottom();
            if (bottom.Namespace() != Namespace::HTML || bottom.TagName() != tagName)
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
      case TokenType::EndOfFile:
      {
        if (!_templateInsertionModes.empty())
        {
          InTemplateMode(krys::move(token));
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

  void TreeBuilder::TextMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::Character:
      {
        InsertCharacters(token.ReleaseCharacters());
        return;
      }
      case TokenType::EndOfFile:
      {
        ParseError(token);

        if (_openElementStack.Bottom().TagName() == TagName::script)
        {
          Downcast<HTMLScriptElement>(_openElementStack.Bottom().Element())._alreadyStarted = true;
        }

        _openElementStack.Pop();

        _insertionMode = _originalInsertionMode;
        ProcessToken(krys::move(token));

        return;
      }
      case TokenType::EndTag:
      {
        // An end tag whose tag name is "script":
        // (Skipped: there's no scripting, so there's no microtask checkpoint, insertion point or script
        // nesting level, the script isn't prepared and there's never a pending parsing-blocking script. It's
        // popped like any other end tag.)
        _openElementStack.Pop();

        _insertionMode = _originalInsertionMode;
        return;
      }
    }
  }

  void TreeBuilder::InTableMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::Character:
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
            InTableTextMode(krys::move(token));

            return;
          }
        }

        break; // fall through to "anything else"
      }
      case TokenType::Comment:
      {
        InsertComment(token.Comment());
        return;
      }
      case TokenType::ProcessingInstruction:
      {
        InsertProcessingInstruction(token);
        return;
      }
      case TokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case TokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::caption:
          {
            _openElementStack.PopUntilTableContext();
            _activeFormattingElements.PushMarker();
            InsertHTMLElement(krys::move(token));

            _insertionMode = InsertionMode::InCaption;
            return;
          }
          case TagName::colgroup:
          {
            _openElementStack.PopUntilTableContext();
            InsertHTMLElement(krys::move(token));

            _insertionMode = InsertionMode::InColumnGroup;
            return;
          }
          case TagName::col:
          {
            _openElementStack.PopUntilTableContext();

            auto colgroup = dom::ElementFactory::Create(
              _document, dom::QualifiedName(infra::Namespaces::HTML, dom::DOMStringAtom::Null(), u8"colgroup",
                                            TagName::colgroup, Namespace::HTML));
            InsertElementAtAdjustedInsertionLocation(*colgroup);
            _openElementStack.Push({TagName::colgroup, Namespace::HTML, *colgroup, {}});

            _insertionMode = InsertionMode::InColumnGroup;
            ProcessToken(krys::move(token));

            return;
          }
          case TagName::tbody:
          case TagName::tfoot:
          case TagName::thead:
          {
            _openElementStack.PopUntilTableContext();
            InsertHTMLElement(krys::move(token));

            _insertionMode = InsertionMode::InTableBody;
            return;
          }
          case TagName::td:
          case TagName::th:
          case TagName::tr:
          {
            _openElementStack.PopUntilTableContext();

            auto tbody = dom::ElementFactory::Create(
              _document, dom::QualifiedName(infra::Namespaces::HTML, dom::DOMStringAtom::Null(), u8"tbody",
                                            TagName::tbody, Namespace::HTML));
            InsertElementAtAdjustedInsertionLocation(*tbody);
            _openElementStack.Push({TagName::tbody, Namespace::HTML, *tbody, {}});

            _insertionMode = InsertionMode::InTableBody;
            ProcessToken(krys::move(token));

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

            ProcessToken(krys::move(token));
            return;
          }
          case TagName::style:
          case TagName::script:
          case TagName::template_:
          {
            InHeadMode(krys::move(token));
            return;
          }
          case TagName::input:
          {
            auto typeIt = std::ranges::find_if(token.Attributes(),
                                               [](const ParsedAttribute &attr)
                                               {
                                                 return attr.NameView() == u8"type"
                                                        && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(
                                                          attr.ValueView(), u8"hidden");
                                               });

            if (typeIt == token.Attributes().end())
            {
              break; // fall through to "anything else"
            }

            ParseError(token);

            InsertHTMLElement(krys::move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();

            return;
          }
          case TagName::form:
          {
            // Parse error.
            ParseError(token);

            // If the form element pointer is not null, and the parser is not parsing template contents, then
            // ignore the token.
            auto containsTemplateElement = _openElementStack.ContainsTemplateElement();
            if (_form != nullptr && !containsTemplateElement)
            {
              return; // ignore the token
            }

            // Otherwise: Insert an HTML element for the token, and, if the parser is not parsing template
            // contents, set the form element pointer to point to the element created.
            auto element = InsertHTMLElement(krys::move(token));
            if (!containsTemplateElement)
            {
              _form = krys::move(element);
            }

            // Pop that form element off the stack of open elements.
            _openElementStack.Pop();

            return;
          }
        }
        break; // fall through to "anything else"
      }
      case TokenType::EndTag:
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
            InHeadMode(krys::move(token));
            return;
          }
        }
        break; // fall through to "anything else"
      }
      case TokenType::EndOfFile:
      {
        InBodyMode(krys::move(token));
        return;
      }
    }

    ParseError(token);

    _fosterParenting = true;
    InBodyMode(krys::move(token));
    _fosterParenting = false;
  }

  void TreeBuilder::InTableTextMode(TokenAtom &&token) noexcept
  {
    if (token.Type() == TokenType::Character)
    {
      if (!RemoveNulls(token))
      {
        return; // ignore the token
      }

      _pendingTableCharacterTokens.emplace_back(token.ReleaseCharacters());
      return;
    }

    // Anything else: check if any pending token contains non-ASCII-whitespace.
    bool hasNonWhitespace = false;
    for (const auto &chars : _pendingTableCharacterTokens)
    {
      hasNonWhitespace =
        std::ranges::any_of(chars, [](char8 ch) { return !infra::StringAlgorithms::IsASCIIWhitespace(ch); });
      if (hasNonWhitespace)
      {
        break;
      }
    }

    if (hasNonWhitespace)
    {
      ParseError(token);

      _fosterParenting = true;
      {
        for (auto &chars : _pendingTableCharacterTokens)
        {
          ReconstructActiveFormattingElements();
          InsertCharacters(krys::move(chars));
        }
      }
      _fosterParenting = false;

      _framesetOk = false;
    }
    else
    {
      for (auto &chars : _pendingTableCharacterTokens)
      {
        InsertCharacters(krys::move(chars));
      }
    }
    _pendingTableCharacterTokens.clear();

    _insertionMode = _originalInsertionMode;
    ProcessToken(krys::move(token));
  }

  void TreeBuilder::InCaptionMode(TokenAtom &&token) noexcept
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
      case TokenType::StartTag:
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
              ProcessToken(krys::move(token));
            }
            return;
          }
        }

        break;
      }
      case TokenType::EndTag:
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
              ProcessToken(krys::move(token));
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

        break;
      }
    }

    InBodyMode(krys::move(token));
  }

  void TreeBuilder::InColumnGroupMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::Character:
      {
        if (InsertLeadingWhitespace(token))
        {
          break;
        }

        return;
      }
      case TokenType::Comment:
      {
        InsertComment(token.Comment());
        return;
      }
      case TokenType::ProcessingInstruction:
      {
        InsertProcessingInstruction(token);
        return;
      }
      case TokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case TokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::html:
          {
            InBodyMode(krys::move(token));
            return;
          }
          case TagName::col:
          {
            InsertHTMLElement(krys::move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();
            return;
          }
          case TagName::template_:
          {
            InHeadMode(krys::move(token));
            return;
          }
        }
        break;
      }
      case TokenType::EndTag:
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
            InHeadMode(krys::move(token));
            return;
          }
        }
        break;
      }
      case TokenType::EndOfFile:
      {
        InBodyMode(krys::move(token));
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
    InTableMode(krys::move(token));
  }

  void TreeBuilder::InTableBodyMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::tr:
          {
            _openElementStack.PopUntilTableBodyContext();
            InsertHTMLElement(krys::move(token));

            _insertionMode = InsertionMode::InRow;
            return;
          }
          case TagName::th:
          case TagName::td:
          {
            ParseError(token);

            _openElementStack.PopUntilTableBodyContext();

            auto tr = dom::ElementFactory::Create(
              _document, dom::QualifiedName(infra::Namespaces::HTML, dom::DOMStringAtom::Null(), u8"tr",
                                            TagName::tr, Namespace::HTML));
            InsertElementAtAdjustedInsertionLocation(*tr);
            _openElementStack.Push({TagName::tr, Namespace::HTML, *tr, {}});

            _insertionMode = InsertionMode::InRow;
            ProcessToken(krys::move(token));

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
            ProcessToken(krys::move(token));

            return;
          }
        }
        break;
      }
      case TokenType::EndTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::tbody:
          case TagName::tfoot:
          case TagName::thead:
          {
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
            ProcessToken(krys::move(token));

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

    InTableMode(krys::move(token));
  }

  void TreeBuilder::InRowMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::th:
          case TagName::td:
          {
            _openElementStack.PopUntilTableRowContext();

            InsertHTMLElement(krys::move(token));
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
            InTableBodyMode(krys::move(token));

            return;
          }
        }
        break;
      }
      case TokenType::EndTag:
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
            ProcessToken(krys::move(token));

            return;
          }
          case TagName::tbody:
          case TagName::tfoot:
          case TagName::thead:
          {
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
            InTableBodyMode(krys::move(token));

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

    InTableMode(krys::move(token));
  }

  void TreeBuilder::InCellMode(TokenAtom &&token) noexcept
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
      case TokenType::StartTag:
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
            krys_debug_assert(_openElementStack.HasElementInTableScope(TagName::td)
                              || _openElementStack.HasElementInTableScope(TagName::th));
            CloseTheCell();
            ProcessToken(krys::move(token));
            return;
          }
        }
        break;
      }
      case TokenType::EndTag:
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
            ProcessToken(krys::move(token));

            return;
          }
        }
        break;
      }
    }

    InBodyMode(krys::move(token));
  }

  void TreeBuilder::InTemplateMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::Character:
      case TokenType::Comment:
      case TokenType::ProcessingInstruction:
      case TokenType::DOCTYPE:
      {
        InBodyMode(krys::move(token));
        return;
      }
      case TokenType::StartTag:
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
            InHeadMode(krys::move(token));
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
            ProcessToken(krys::move(token));

            return;
          }
          case TagName::col:
          {
            _templateInsertionModes.pop_back();
            _templateInsertionModes.push_back(InsertionMode::InColumnGroup);

            _insertionMode = InsertionMode::InColumnGroup;
            ProcessToken(krys::move(token));

            return;
          }
          case TagName::tr:
          {
            _templateInsertionModes.pop_back();
            _templateInsertionModes.push_back(InsertionMode::InTableBody);

            _insertionMode = InsertionMode::InTableBody;
            ProcessToken(krys::move(token));

            return;
          }
          case TagName::td:
          case TagName::th:
          {
            _templateInsertionModes.pop_back();
            _templateInsertionModes.push_back(InsertionMode::InRow);

            _insertionMode = InsertionMode::InRow;
            ProcessToken(krys::move(token));

            return;
          }
          default:
          {
            _templateInsertionModes.pop_back();
            _templateInsertionModes.push_back(InsertionMode::InBody);

            _insertionMode = InsertionMode::InBody;
            ProcessToken(krys::move(token));

            return;
          }
        }
      }
      case TokenType::EndTag:
      {
        if (token.Name() == u8"template")
        {
          InHeadMode(krys::move(token));
          return;
        }

        ParseError(token);
        return; // ignore the token
      }
      case TokenType::EndOfFile:
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

        ProcessToken(krys::move(token));
        return;
      }
    }
  }

  void TreeBuilder::AfterBodyMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::Character:
      {
        ReconstructActiveFormattingElements();

        if (InsertLeadingWhitespace(token))
        {
          break;
        }

        return;
      }
      case TokenType::Comment:
      {
        InsertComment(token.Comment(), InsertionLocation {&_openElementStack.Top().Element(), nullptr});
        return;
      }
      case TokenType::ProcessingInstruction:
      {
        InsertProcessingInstruction(token, InsertionLocation {&_openElementStack.Top().Element(), nullptr});
        return;
      }
      case TokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case TokenType::StartTag:
      {
        if (token.Name() == u8"html")
        {
          InBodyMode(krys::move(token));
          return;
        }
        break;
      }
      case TokenType::EndTag:
      {
        if (token.Name() == u8"html")
        {
          if (_context.has_value())
          {
            ParseError(token); // fragment case, ignore
            return;
          }

          _insertionMode = InsertionMode::AfterAfterBody;
          return;
        }
        break;
      }
      case TokenType::EndOfFile:
      {
        StopParsing();
        return;
      }
    }

    ParseError(token);

    _insertionMode = InsertionMode::InBody;
    ProcessToken(krys::move(token));
  }

  void TreeBuilder::InFramesetMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::Character:
      {
        if (!RemoveNonWhitespace(token))
        {
          return;
        }

        InsertLeadingWhitespace(token);
        krys_debug_assert(token._data.empty()); // should have been removed or inserted already

        return;
      }
      case TokenType::Comment:
      {
        InsertComment(token.Comment());
        return;
      }
      case TokenType::ProcessingInstruction:
      {
        InsertProcessingInstruction(token);
        return;
      }
      case TokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case TokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::html:
          {
            InBodyMode(krys::move(token));
            return;
          }
          case TagName::frameset:
          {
            InsertHTMLElement(krys::move(token));
            return;
          }
          case TagName::frame:
          {
            InsertHTMLElement(krys::move(token));
            _openElementStack.Pop();
            token.AcknowledgeSelfClosingTag();
            return;
          }
          case TagName::noframes:
          {
            InHeadMode(krys::move(token));
            return;
          }
        }

        break;
      }
      case TokenType::EndTag:
      {
        if (token.Name() == u8"frameset")
        {
          if (&_openElementStack.Bottom() == &_openElementStack.Top())
          {
            ParseError(token);
            return; // fragment case, ignore
          }

          _openElementStack.Pop();

          if (!_context.has_value() && _openElementStack.Bottom().TagName() != TagName::frameset)
          {
            _insertionMode = InsertionMode::AfterFrameset;
          }

          return;
        }

        break;
      }
      case TokenType::EndOfFile:
      {
        if (&_openElementStack.Bottom() != &_openElementStack.Top())
        {
          ParseError(token);
        }

        StopParsing();
        return;
      }
    }

    ParseError(token);
  }

  void TreeBuilder::AfterFramesetMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::Character:
      {
        if (!RemoveNonWhitespace(token))
        {
          return;
        }

        InsertLeadingWhitespace(token);
        krys_debug_assert(token._data.empty()); // should have been removed or inserted already

        return;
      }
      case TokenType::Comment:
      {
        InsertComment(token.Comment());
        return;
      }
      case TokenType::ProcessingInstruction:
      {
        InsertProcessingInstruction(token);
        return;
      }
      case TokenType::DOCTYPE:
      {
        ParseError(token);
        return; // ignore the token
      }
      case TokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::html:
          {
            InBodyMode(krys::move(token));
            return;
          }
          case TagName::noframes:
          {
            InHeadMode(krys::move(token));
            return;
          }
        }
        break;
      }
      case TokenType::EndTag:
      {
        if (token.Name() == u8"html")
        {
          _insertionMode = InsertionMode::AfterAfterFrameset;
          return;
        }

        break;
      }
      case TokenType::EndOfFile:
      {
        StopParsing();
        return;
      }
    }

    ParseError(token);
  }

  void TreeBuilder::AfterAfterBodyMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::Comment:
      {
        AppendCommentToDocument(token.Comment());
        return;
      }
      case TokenType::ProcessingInstruction:
      {
        InsertProcessingInstruction(token, InsertionLocation {&_document, nullptr});
        return;
      }
      case TokenType::DOCTYPE:
      {
        InBodyMode(krys::move(token));
        return;
      }
      case TokenType::Character:
      {
        ReconstructActiveFormattingElements();

        if (InsertLeadingWhitespace(token))
        {
          break;
        }

        return;
      }
      case TokenType::StartTag:
      {
        if (token.Name() == u8"html")
        {
          InBodyMode(krys::move(token));
          return;
        }

        break;
      }
      case TokenType::EndOfFile:
      {
        StopParsing();
        return;
      }
    }

    ParseError(token);

    _insertionMode = InsertionMode::InBody;
    InBodyMode(krys::move(token));
  }

  void TreeBuilder::AfterAfterFramesetMode(TokenAtom &&token) noexcept
  {
    switch (token.Type())
    {
      case TokenType::Comment:
      {
        AppendCommentToDocument(token.Comment());
        return;
      }
      case TokenType::ProcessingInstruction:
      {
        InsertProcessingInstruction(token, InsertionLocation {&_document, nullptr});
        return;
      }
      case TokenType::DOCTYPE:
      {
        InBodyMode(krys::move(token));
        return;
      }
      case TokenType::Character:
      {
        ReconstructActiveFormattingElements();
        if (InsertLeadingWhitespace(token))
        {
          break;
        }

        return;
      }
      case TokenType::StartTag:
      {
        auto tagName = ParseTagName(token.Name().View());
        switch (tagName)
        {
          case TagName::html:
          {
            InBodyMode(krys::move(token));
            return;
          }
          case TagName::noframes:
          {
            InHeadMode(krys::move(token));
            return;
          }
        }
        break;
      }
      case TokenType::EndOfFile:
      {
        StopParsing();
        return;
      }
    }

    ParseError(token);
  }

  void TreeBuilder::InBodyGenericEndTag(const TokenAtom &token, TagName tagName)
  {
    // 1. Initialize node to be the current node (the bottommost node of the stack).
    for (auto nodeIt = _openElementStack.rbegin(); nodeIt != _openElementStack.rend(); ++nodeIt)
    {
      auto &nodeEntry = *nodeIt;
      // 2. Loop: If node is an HTML element with the same tag name as the token, then:
      // (The local name is compared rather than the parsed tag name, which is the same for every unknown
      // element.)
      if (nodeEntry.Namespace() == Namespace::HTML && nodeEntry.Element().LocalName() == token.Name())
      {
        // 2a. Generate implied end tags, except for HTML elements with the same tag name as the token.
        _openElementStack.GenerateImpliedEndTags(tagName);

        // 2b. If node is not the current node, then this is a parse error.
        if (&nodeEntry != &_openElementStack.Bottom())
        {
          ParseError(token);
        }

        // 2c. Pop all the nodes from the current node up to node, including node, then stop these steps.
        _openElementStack.PopUntilPopped(nodeEntry.Element());
        return;
      }

      // 3. Otherwise, if node is in the special category, then this is a parse error; ignore the token, and
      //    return.
      if (IsSpecialElement(nodeEntry.TagName(), nodeEntry.Namespace()))
      {
        ParseError(token);
        return; // ignore the token
      }

      // 4. Set node to the previous entry in the stack of open elements.
      // 5. Return to the step labeled loop.
    }
  }

  void TreeBuilder::StopParsing() noexcept
  {
    // https://html.spec.whatwg.org/#the-end
    // 1. (Skipped: there's no speculative HTML parser.)
    // 2. (Skipped: there's no insertion point.)
    // 4. Pop all the nodes off the stack of open elements.
    _openElementStack.PopAll();

    // 3., 5.-11. The document finishes parsing (a fragment's parser has nothing more to do): it updates its
    // readiness to "interactive", fires DOMContentLoaded and goes on to its load event.
    if (!_context.has_value())
    {
      _document.FinishedParsing();
    }
  }

#pragma endregion

#pragma region Character Insertion

  void TreeBuilder::InsertCharacters(dom::DOMString data) noexcept
  {
    // 1. Let data be the characters passed to the algorithm, or, if no characters were explicitly specified,
    //    the character of the character token being processed.
    // 2. Let (targetParent, referenceChild) be the adjusted insertion location.
    auto [parent, beforeSibling] = AdjustedInsertionLocation();

    // 3. If targetParent is a Document node, then return.
    if (Is<dom::Document>(parent))
    {
      return;
    }

    // 4. Let previousSibling be referenceChild's previous sibling if referenceChild is non-null; otherwise
    //    targetParent's last child.
    auto *previousSibling = beforeSibling ? beforeSibling->PreviousSibling() : parent->LastChild();

    // 5. If previousSibling is a Text node, then append data to previousSibling's data. Otherwise, let text
    // be
    //    the result of creating a text node given targetParent's node document and data, and insert text into
    //    targetParent before referenceChild.
    if (Is<dom::Text>(previousSibling))
    {
      auto &textNode = Downcast<dom::Text>(*previousSibling);
      textNode.AppendData(krys::move(data));
    }
    else
    {
      auto textNode = CreateRef<dom::Text>(parent->NodeDocument(), krys::move(data));
      // We purposely ignore the error here if it happens.
      (void)dom::MutationAlgorithms::Insert(*textNode, *parent, beforeSibling);
    }
  }

  bool TreeBuilder::RemoveNulls(TokenAtom &token) noexcept
  {
    krys_debug_assert(token.Type() == TokenType::Character);

    auto removed = std::erase_if(token._data, [](char8 ch) { return ch == '\0'; });
    if (removed > 0uz)
    {
      ParseError(token);
    }

    return !token.Characters().empty();
  }

  bool TreeBuilder::RemoveNonWhitespace(TokenAtom &token) noexcept
  {
    krys_debug_assert(token.Type() == TokenType::Character);

    auto removed =
      std::erase_if(token._data, [](char8 ch) { return !infra::StringAlgorithms::IsASCIIWhitespace(ch); });
    if (removed > 0uz)
    {
      ParseError(token);
    }

    return !token.Characters().empty();
  }

  bool TreeBuilder::SkipLeadingWhitespace(TokenAtom &token) noexcept
  {
    krys_debug_assert(token.Type() == TokenType::Character);

    auto begin = token.Characters().begin();
    auto position = begin;
    infra::StringAlgorithms::SkipWhitespace(token.Characters(), position);

    token._data.erase(0uz, std::distance(begin, position));
    return !token.Characters().empty();
  }

  bool TreeBuilder::InsertLeadingWhitespace(TokenAtom &token) noexcept
  {
    krys_debug_assert(token.Type() == TokenType::Character);

    auto begin = token.Characters().begin();
    auto position = begin;
    infra::StringAlgorithms::SkipWhitespace(token.Characters(), position);

    if (position != begin)
    {
      InsertCharacters(dom::DOMString(begin, position));
      token._data.erase(0uz, std::distance(begin, position));
    }

    return !token.Characters().empty();
  }

  bool TreeBuilder::InsertLeadingWhitespaceAndNulls(TokenAtom &token) noexcept
  {
    krys_debug_assert(token.Type() == TokenType::Character);

    constexpr static dom::DOMStringView Replacement = u8"\uFFFD";

    auto ReplaceNulls = [&](dom::DOMString &str)
    {
      auto pos = str.find(u8'\0');
      while (pos != dom::DOMString::npos)
      {
        ParseError(token);
        str.replace(pos, 1uz, Replacement);
        pos = str.find(u8'\0', pos + Replacement.length());
      }
    };

    auto begin = token.Characters().begin();
    auto position = begin;
    infra::StringAlgorithms::AdvancePositionWhile(
      token.Characters(), position,
      [](char8 ch) { return ch == '\0' || infra::StringAlgorithms::IsASCIIWhitespace(ch); });

    if (position != begin)
    {
      auto leadingData = dom::DOMString(begin, position);
      ReplaceNulls(leadingData);
      InsertCharacters(krys::move(leadingData));
      token._data.erase(0uz, std::distance(begin, position));
    }

    if (token.Characters().empty())
    {
      return false;
    }

    ReplaceNulls(token._data);
    return true;
  }

#pragma endregion

#pragma region Insertion Algorithms

  namespace
  {
    // https://html.spec.whatwg.org/#appropriate-place-for-inserting-a-node (steps 5 to 8, where targetParent
    // is a template element)
    KRYS_NODISCARD InsertionLocation
      InsertionLocationInTemplate(const HTMLTemplateElement &targetParent) noexcept
    {
      // 5. Let patchInsertionTarget be targetParent's insertion target.
      auto *patchInsertionTarget = targetParent.InsertionTarget();

      // 6. If patchInsertionTarget is null, then return (targetParent's template contents, null).
      if (!patchInsertionTarget)
      {
        return {.Parent = &targetParent.TemplateContents(), .BeforeSibling = nullptr};
      }

      // 7. If targetParent's insertion end marker is not null and its parent is patchInsertionTarget, then
      //    return (patchInsertionTarget, targetParent's insertion end marker).
      auto *endMarker = targetParent.InsertionEndMarker();
      if (endMarker && endMarker->ParentNode() == patchInsertionTarget)
      {
        return {.Parent = patchInsertionTarget, .BeforeSibling = endMarker};
      }

      // 8. Return (patchInsertionTarget, null).
      return {.Parent = patchInsertionTarget, .BeforeSibling = nullptr};
    }
  }

  // https://html.spec.whatwg.org/#appropriate-place-for-inserting-a-node
  InsertionLocation TreeBuilder::AppropriatePlaceToInsertNode(dom::ContainerNode *targetOverride) noexcept
  {
    // 1. Let targetParent be overrideTarget if it is not null; otherwise the current node.
    auto &target = targetOverride ? *targetOverride : CurrentNode();

    // 2. Let referenceChild be null.
    // (The steps below only apply to HTML elements, so other targets are returned as they are.)
    if (!Is<HTMLElement>(target))
    {
      return {.Parent = &target, .BeforeSibling = nullptr};
    }

    auto &htmlTarget = Downcast<HTMLElement>(target);
    auto tagName = htmlTarget.TagQualifiedName().TagName();

    InsertionLocation adjustedInsertionLocation;

    // 3. If foster parenting is enabled and targetParent is a table, tbody, tfoot, thead, or tr element:
    if (_fosterParenting
        && (tagName == TagName::table || tagName == TagName::tbody || tagName == TagName::tfoot
            || tagName == TagName::thead || tagName == TagName::tr))
    {
      // 3a. Let lastTemplateOrTable be the last template or table element in the stack of open elements if
      //     there is one; otherwise null.
      // (The last template and the last table are found, and the later of the two is lastTemplateOrTable.)
      auto lastTemplateIt = _openElementStack.rend();
      auto HasTemplate = [&]()
      {
        return lastTemplateIt != _openElementStack.rend();
      };

      auto lastTableIt = _openElementStack.rend();
      auto HasTable = [&]()
      {
        return lastTableIt != _openElementStack.rend();
      };

      for (auto it = _openElementStack.rbegin(); it != _openElementStack.rend(); ++it)
      {
        if (!Is<HTMLElement>(it->Element()))
        {
          continue;
        }

        auto &element = Downcast<HTMLElement>(it->Element());

        if (!HasTemplate() && Is<HTMLTemplateElement>(element))
        {
          lastTemplateIt = it;
        }

        if (!HasTable() && Is<HTMLTableElement>(element))
        {
          lastTableIt = it;
        }

        if (HasTable() && HasTemplate())
        {
          break;
        }
      }

      // 3c. If lastTemplateOrTable is a template element, then set targetParent to lastTemplateOrTable.
      // (Steps 4 to 8 then give the place in the template.)
      if (HasTemplate() && (!HasTable() || lastTemplateIt < lastTableIt))
      {
        return InsertionLocationInTemplate(Downcast<HTMLTemplateElement>(lastTemplateIt->Element()));
      }

      // 3b. If lastTemplateOrTable is null, then return (the first element in the stack of open elements (the
      //     html element), null).
      if (!HasTable())
      {
        return {.Parent = &_openElementStack.Top().Element()};
      }

      // 3d. Otherwise, if lastTemplateOrTable's parent is not null, then set targetParent to
      //     lastTemplateOrTable's parent and set referenceChild to lastTemplateOrTable.
      // (Its parent isn't a template element, as a template's children go in its template contents.)
      if (lastTableIt->Element().ParentNode() != nullptr)
      {
        return {.Parent = lastTableIt->Element().ParentNode(), .BeforeSibling = &lastTableIt->Element()};
      }

      // 3e. Otherwise, set targetParent to the element immediately above lastTemplateOrTable in the stack of
      //     open elements.
      auto previousElement = std::next(lastTableIt);
      krys_debug_assert(previousElement != _openElementStack.rend());

      adjustedInsertionLocation =
        InsertionLocation {.Parent = &previousElement->Element(), .BeforeSibling = nullptr};
    }
    else
    {
      adjustedInsertionLocation = {.Parent = &target, .BeforeSibling = nullptr};
    }

    // 4. If targetParent is not a template element, then return (targetParent, referenceChild).
    // 5.-8. (InsertionLocationInTemplate.)
    if (auto *templateElement = DynamicDowncast<HTMLTemplateElement>(adjustedInsertionLocation.Parent))
    {
      adjustedInsertionLocation = InsertionLocationInTemplate(*templateElement);
    }

    return adjustedInsertionLocation;
  }

  // https://html.spec.whatwg.org/#adjusted-insertion-location
  InsertionLocation
    TreeBuilder::AdjustedInsertionLocation(Maybe<InsertionLocation> insertionLocation) noexcept
  {
    // 1. Let location be insertionLocation if insertionLocation is non-null; otherwise the appropriate
    //    place for inserting a node.
    auto location = insertionLocation ? *insertionLocation : AppropriatePlaceToInsertNode();

    // 2. Let (targetParent, refChild) be location.
    // 3. If the parser's insertion target redirection map[targetParent] does not exist, then return
    //    location.
    auto redirect = _insertionTargetRedirectionMap.find(location.Parent);
    if (redirect == _insertionTargetRedirectionMap.end())
    {
      return location;
    }

    // 4. Assert: refChild is null.
    krys_debug_assert(!location.BeforeSibling);

    // 5. Return the parser's insertion target redirection map[targetParent].
    return redirect->second;
  }

  void TreeBuilder::RedirectInsertions(dom::Element &element, InsertionLocation location) noexcept
  {
    if (_insertionTargetRedirectionMap.insert_or_assign(&element, location).second)
    {
      _redirectedElements.push_back(ShareRef(element));
    }
  }

  // https://html.spec.whatwg.org/#insert-an-element-at-the-adjusted-insertion-location
  void TreeBuilder::InsertElementAtAdjustedInsertionLocation(dom::Element &element,
                                                             Maybe<InsertionLocation> location,
                                                             bool alreadySanitized) noexcept
  {
    // 1. Let (targetParent, referenceChild) be the adjusted insertion location given overrideLocation.
    auto [parent, beforeSibling] = AdjustedInsertionLocation(location);

    // 2. If any of the following are true: element's parent is non-null; element is a host-including
    //    inclusive ancestor of targetParent; or targetParent is a Document node that already has an element
    //    child, then return.
    if (element.ParentNode() || dom::TreeQueries::IsHostIncludingInclusiveAncestorOf(element, *parent)
        || (Is<dom::Document>(*parent) && dom::TreeQueries::FirstElementChild(*parent)))
    {
      return;
    }

    // 3. If alreadySanitized is false and the parser's parser sanitizer configuration is not null:
    if (!alreadySanitized && _sanitizerConfig)
    {
      // 3a. Let action be the result of getting the sanitizer action for an element name given
      //     element's local name, element's namespace, and the parser's parser sanitizer configuration.
      auto action = SanitizerActionFor(element.LocalName().View(), element.NamespaceURI());

      // 3b. For each attribute of element's attribute list: assert: the result of checking if a
      //     sanitizer config allows an attribute [...] is true.
      //     (Attributes are sanitized when the element is created from the token.)

      // 3c. Assert: if targetParent is a Document, then action is not "Replace with children".
      krys_debug_assert(!Is<dom::Document>(*parent) || action != SanitizerAction::ReplaceWithChildren);

      // 3d. Switch on action:
      switch (action)
      {
        // "Replace with children": set the parser's insertion target redirection map[element] to
        // (targetParent, referenceChild), and return.
        case SanitizerAction::ReplaceWithChildren:
        {
          RedirectInsertions(element, {.Parent = parent, .BeforeSibling = beforeSibling});
          return;
        }
        // "Remove": return.
        case SanitizerAction::Remove:
        {
          return;
        }
        // "Keep": do nothing.
        case SanitizerAction::Keep:
        {
          break;
        }
      }
    }

    // 4. Assert: ensure pre-insert validity given element, targetParent, referenceChild, and « » does
    //    not throw.
    //    (Hosts can move nodes so that this would throw; insertion is skipped then.)
    if (auto result = dom::MutationAlgorithms::EnsurePreInsertValidity(element, *parent, beforeSibling);
        result.HasException())
    {
      return;
    }

    // 5. If the parser's fragment context element is null, then push a new element queue onto element's
    //    relevant agent's custom element reactions stack.
    Maybe<CustomElementReactionStack> reactionStack;
    if (!_context.has_value())
    {
      reactionStack.emplace();
    }

    // 6. Insert element into targetParent before referenceChild.
    (void)dom::MutationAlgorithms::Insert(element, *parent, beforeSibling);

    // 7. If the parser's fragment context element is null, then pop the element queue from element's
    //    relevant agent's custom element reactions stack, and invoke custom element reactions in that queue.
    reactionStack.reset();
  }

  namespace
  {
    // The state of a template's shadowrootmode attribute, null for its None state.
    KRYS_NODISCARD Maybe<dom::ShadowRootMode>
      DeclarativeShadowRootMode(const HTMLTemplateElement &element) noexcept
    {
      auto value = element.GetAttribute(u8"shadowrootmode");
      if (!value)
      {
        return null;
      }
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*value, u8"open"))
      {
        return dom::ShadowRootMode::Open;
      }
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*value, u8"closed"))
      {
        return dom::ShadowRootMode::Closed;
      }
      return null;
    }
  }

  // https://html.spec.whatwg.org/#parsing-main-inhead (a start tag whose tag name is "template")
  void TreeBuilder::InsertTemplateElement(TokenAtom &&token) noexcept
  {
    // 1. Let templateStartTag be the start tag.
    auto &templateStartTag = token;

    // 2. Insert a marker at the end of the list of active formatting elements.
    _activeFormattingElements.PushMarker();

    // 3. Set the frameset-ok flag to "not ok".
    _framesetOk = false;

    // 4. Switch the insertion mode to "in template".
    _insertionMode = InsertionMode::InTemplate;

    // 5. Push "in template" onto the stack of template insertion modes so that it is the new current
    //    template insertion mode.
    _templateInsertionModes.push_back(InsertionMode::InTemplate);

    // 6. Let declarativeShadowHostElement be the current node.
    //    (This is not the adjusted current node, so that in the fragment case a top-level template element
    //    never attaches a shadow root to the parser's fragment context element.)
    Ref<dom::Element> declarativeShadowHostElement = ShareRef(CurrentNode());

    // 7. Let hostIsRoot be true if declarativeShadowHostElement is the topmost element in the stack of
    //    open elements; otherwise false.
    bool hostIsRoot = declarativeShadowHostElement.get() == &_openElementStack.Top().Element();

    // 8. Let template be the result of creating an element for the token given templateStartTag, the HTML
    //    namespace, and the target parent of the appropriate place for inserting a node.
    auto element = CreateElement(templateStartTag.Name(), infra::Namespaces::HTML,
                                 templateStartTag.Attributes(), *AppropriatePlaceToInsertNode().Parent);
    auto &templateElement = Downcast<HTMLTemplateElement>(*element);

    // 9. Let insertionLocation be the appropriate place for inserting a node.
    //    (This is computed after template is created, since creating a customized built-in element can run
    //    code that changes the DOM, and before template is pushed onto the stack of open elements, since
    //    afterwards the appropriate place for inserting a node would be inside template's template contents.)
    auto insertionLocation = AppropriatePlaceToInsertNode();

    // 10. Push template onto the stack of open elements so that it is the new current node.
    element->BeginParsingChildren();
    _openElementStack.Push(
      StackItem(TagName::template_, Namespace::HTML, *element, krys::move(templateStartTag.Attributes())));

    // 11. Let sanitizerAction be "Keep".
    // 12. If the parser's parser sanitizer configuration is not null, then set sanitizerAction to the result
    //     of getting the sanitizer action for an element name given template's local name, template's
    //     namespace, and the parser's parser sanitizer configuration.
    // 13. Switch on sanitizerAction: "Remove", "Replace with children": return.
    //     (A template element has no children (its parsed contents are placed in its template contents
    //     DocumentFragment), so replacing it with its children has the same effect as removing it.)
    if (SanitizerActionFor(templateElement.LocalName().View(), templateElement.NamespaceURI())
        != SanitizerAction::Keep)
    {
      return;
    }

    // "Keep":
    // 13a. If template's shadowrootmode attribute is not in the None state:
    auto mode = DeclarativeShadowRootMode(templateElement);
    if (!mode)
    {
      // 13b. Otherwise, if template has a for attribute:
      if (templateElement.HasAttribute(u8"for"))
      {
        // 13b.i. Let scope be the target parent of the adjusted insertion location given insertionLocation.
        Ref<dom::ContainerNode> scope = ShareRef(*AdjustedInsertionLocation(insertionLocation).Parent);

        // 13b.ii. If scope is the body element of its node document, then set scope to its parent.
        if (auto *body = scope->NodeDocument().BodyOrFrameset(); body == scope.get() && body->ParentNode())
        {
          scope = ShareRef(*body->ParentNode());
        }

        // 13b.iii. Let success be the result of preparing content patching given scope and template.
        // 13b.iv. If success is false, then insert an element at the adjusted insertion location with
        //         template, insertionLocation, and true.
        if (!templateElement.PrepareContentPatching(*scope))
        {
          InsertElementAtAdjustedInsertionLocation(templateElement, insertionLocation, true);
        }
        return;
      }

      // 13c. Otherwise, insert an element at the adjusted insertion location with template,
      //      insertionLocation, and true.
      InsertElementAtAdjustedInsertionLocation(templateElement, insertionLocation, true);
      return;
    }

    // 13a.i. If any of the following are true: the parser's allow declarative shadow roots is false;
    //        hostIsRoot is true; or the parser's insertion target redirection
    //        map[declarativeShadowHostElement] exists, then insert an element at the adjusted insertion
    //        location with template, insertionLocation, and true, and return. (The last condition applies
    //        when declarativeShadowHostElement is replaced with its children as part of sanitization. Since
    //        that element is never inserted, a shadow root attached to it would be lost. Instead, template is
    //        inserted as a regular template element, where the replaced element's children are inserted.)
    if (!_document._allowDeclarativeShadowRoots || hostIsRoot
        || _insertionTargetRedirectionMap.contains(declarativeShadowHostElement.get()))
    {
      InsertElementAtAdjustedInsertionLocation(templateElement, insertionLocation, true);
      return;
    }

    // 13a.ii. Let mode be template's shadowrootmode attribute's value.
    // 13a.iii. Let delegatesFocus be true if template has a shadowrootdelegatesfocus attribute; otherwise
    // false.
    dom::DelegatesFocus delegatesFocus(templateElement.HasAttribute(u8"shadowrootdelegatesfocus"));

    // 13a.iv. Let serializable be true if template has a shadowrootserializable attribute; otherwise false.
    dom::Serialisable serializable(templateElement.HasAttribute(u8"shadowrootserializable"));

    // 13a.v. Let slotAssignment be "named".
    // 13a.vi. If template's shadowrootslotassignment attribute is in the Manual state, then set
    // slotAssignment
    //         to "manual".
    auto slotAssignment = dom::SlotAssignmentMode::Named;
    if (auto value = templateElement.GetAttribute(u8"shadowrootslotassignment");
        value && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*value, u8"manual"))
    {
      slotAssignment = dom::SlotAssignmentMode::Manual;
    }

    // 13a.vii. Let clonable be true if template has a shadowrootclonable attribute; otherwise false.
    dom::Clonable clonable(templateElement.HasAttribute(u8"shadowrootclonable"));

    // 13a.viii. If declarativeShadowHostElement is a shadow host, then insert an element at the adjusted
    //           insertion location with template, insertionLocation, and true.
    if (dom::TreeQueries::IsShadowHost(*declarativeShadowHostElement))
    {
      InsertElementAtAdjustedInsertionLocation(templateElement, insertionLocation, true);
      return;
    }

    // 13a.ix.1. Let registry be null if template has a shadowrootcustomelementregistry attribute; otherwise
    //           declarativeShadowHostElement's node document's custom element registry.
    bool keepRegistryNull = templateElement.HasAttribute(u8"shadowrootcustomelementregistry");
    RefPtr<CustomElementRegistry> registry =
      keepRegistryNull ? nullptr : declarativeShadowHostElement->NodeDocument().CustomElementRegistry();

    // 13a.ix.2. Attach a shadow root with declarativeShadowHostElement, mode, delegatesFocus, serializable,
    //           slotAssignment, clonable, and registry. If an exception is thrown, then catch it and: insert
    //           an element at the adjusted insertion location with template, insertionLocation, and true; the
    //           user agent may report an error to the developer console; return.
    if (auto result = dom::ElementAlgorithms::AttachShadowRoot(*declarativeShadowHostElement, *mode, clonable,
                                                               serializable, delegatesFocus, slotAssignment,
                                                               registry.get());
        result.HasException())
    {
      InsertElementAtAdjustedInsertionLocation(templateElement, insertionLocation, true);
      return;
    }

    // 13a.ix.3. Let shadow be declarativeShadowHostElement's shadow root.
    auto *shadow = dom::TreeQueries::GetShadowRoot(*declarativeShadowHostElement);

    // 13a.ix.4. Set shadow's available to element internals to true.
    shadow->_availableToElementInternals = true;

    // 13a.ix.5. Set shadow's declarative to true.
    shadow->_declarative = true;

    // 13a.ix.6. Set template's template contents to shadow.
    templateElement.SetTemplateContents(*shadow);

    // 13a.ix.7. If template has a shadowrootcustomelementregistry attribute, then set shadow's keep custom
    //           element registry null to true.
    //           (Attributes on template were sanitized when the element was created from the token.)
    if (keepRegistryNull)
    {
      shadow->_keepCustomElementRegistryNull = true;
    }
  }

  SanitizerAction TreeBuilder::SanitizerActionFor(dom::DOMStringView localName,
                                                  dom::DOMStringAtom namespaceURI) const noexcept
  {
    return _sanitizerConfig
             ? SanitizerAlgorithms::ActionForElementName(localName, namespaceURI, *_sanitizerConfig)
             : SanitizerAction::Keep;
  }

  void TreeBuilder::MergeAttributes(dom::Element &element, const ParsedAttributeList &attributes) noexcept
  {
    // For each attribute on the token:
    for (auto &attr : attributes)
    {
      // If the parser's parser sanitizer configuration is not null and the result of checking if a
      // sanitizer config allows an attribute given attribute's name, null, attribute's value, element's local
      // name, element's namespace, and the parser's parser sanitizer configuration is false, then continue.
      if (_sanitizerConfig
          && !SanitizerAlgorithms::AllowsAttribute(attr.NameView(), dom::DOMStringAtom::Null(),
                                                   attr.ValueView(), element.LocalName().View(),
                                                   element.NamespaceURI(), *_sanitizerConfig))
      {
        continue;
      }

      // If element does not have an attribute whose name is attribute's name, then add attribute's name and
      // its corresponding value to element.
      if (!element.HasAttribute(attr.NameView()))
      {
        element.SetAttribute(attr.NameView(), dom::DOMString(attr.ValueView()));
      }
    }
  }

  Ref<dom::Element> TreeBuilder::InsertForeignElement(TokenAtom &&token, dom::DOMStringAtom namespaceURI,
                                                      bool onlyAddToElementStack) noexcept
  {
    // 1. Let element be the result of creating an element for the token given token, namespace, and the
    // target
    //    parent of the appropriate place for inserting a node.
    auto [parent, beforeSibling] = AppropriatePlaceToInsertNode();
    auto element = CreateElement(token.Name(), namespaceURI, token.Attributes(), *parent);

    // The element is about to be pushed onto the stack of open elements, which begins parsing its
    // children; begin before inserting it, so that its insertion steps see that its children are still to
    // come.
    element->BeginParsingChildren();

    // 2. If onlyAddToElementStack is false, then run insert an element at the adjusted insertion location
    // with
    //    element.
    if (!onlyAddToElementStack)
    {
      InsertElementAtAdjustedInsertionLocation(*element);
    }

    // 3. Push element onto the stack of open elements so that it is the new current node.
    auto tagName = ParseTagName(token.Name().View());
    auto tagNamespace = ParseNamespace(namespaceURI.View());
    StackItem stackItem(tagName, tagNamespace, *element, krys::move(token.Attributes()));

    _openElementStack.Push(krys::move(stackItem));

    // 4. Return element.
    return element;
  }

  Ref<dom::Element> TreeBuilder::InsertHTMLElement(TokenAtom &&token) noexcept
  {
    return InsertForeignElement(krys::move(token), infra::Namespaces::HTML, false);
  }

  void TreeBuilder::InsertComment(dom::DOMStringView data, Maybe<InsertionLocation> position) noexcept
  {
    // 1. If the parser's parser sanitizer configuration is non-null and the parser's parser sanitizer
    //    configuration does not allow comments, then return.
    if (_sanitizerConfig && !SanitizerAlgorithms::AllowsComments(*_sanitizerConfig))
    {
      return;
    }

    // 2. Let data be the data given in the comment token being processed.
    // 3. Let (targetParent, referenceChild) be the adjusted insertion location given insertionLocation.
    auto [parent, beforeSibling] = AdjustedInsertionLocation(position);

    // 4. Let comment be the result of creating a comment node given targetParent's node document and data.
    auto commentNode = CreateRef<dom::Comment>(parent->NodeDocument(), dom::DOMString(data));

    // 5. Insert comment into targetParent before referenceChild.
    // We purposely ignore the error here if it happens.
    (void)dom::MutationAlgorithms::Insert(*commentNode, *parent, beforeSibling);
  }

  // https://html.spec.whatwg.org/#insert-a-processing-instruction
  void TreeBuilder::InsertProcessingInstruction(const TokenAtom &token,
                                                Maybe<InsertionLocation> position) noexcept
  {
    // 1. Let piTarget be the target given in the processing instruction token being processed.
    auto piTarget = token.ProcessingInstructionTarget();

    // 2. If the parser's parser sanitizer configuration is non-null and the parser's parser sanitizer
    //    configuration does not allow processing instruction target piTarget, then return.
    if (_sanitizerConfig
        && !SanitizerAlgorithms::AllowsProcessingInstructionTarget(piTarget.View(), *_sanitizerConfig))
    {
      return;
    }

    // 3. Let data be the data given in the processing instruction token being processed.
    // 4. Let (targetParent, referenceChild) be the adjusted insertion location given insertionLocation.
    auto [parent, beforeSibling] = AdjustedInsertionLocation(position);

    // 5. Let pi be the result of creating a processing instruction node given targetParent's node document,
    //    piTarget, and data.
    // (The tokenizer only makes targets that are XML names, and data without "?>", so this can't throw.)
    auto pi = dom::ProcessingInstruction::Create(parent->NodeDocument(), dom::DOMString(piTarget.View()),
                                                 dom::DOMString(token.ProcessingInstructionData()))
                .ReleaseValue();

    // 6. Insert pi into targetParent before referenceChild.
    (void)dom::MutationAlgorithms::Insert(*pi, *parent, beforeSibling);
  }

  void TreeBuilder::AppendCommentToDocument(dom::DOMStringView data) noexcept
  {
    InsertComment(data, InsertionLocation {&_document, nullptr});
  }

  void TreeBuilder::ParseGenericRawTextElement(TokenAtom &&token) noexcept
  {
    // 1. Insert an HTML element for the token.
    InsertHTMLElement(krys::move(token));

    // 2. If the algorithm that was invoked is the generic raw text element parsing algorithm, switch the
    //    tokenizer to the RAWTEXT state; otherwise the algorithm invoked was the generic RCDATA element
    //    parsing algorithm, switch the tokenizer to the RCDATA state.
    _tokenizer.State(TokenizerState::RAWTEXT);

    // 3. Set the original insertion mode to the current insertion mode.
    _originalInsertionMode = _insertionMode;

    // 4. Then, switch the insertion mode to "text".
    _insertionMode = InsertionMode::Text;
  }

  void TreeBuilder::ParseGenericRCDATATextElement(TokenAtom &&token) noexcept
  {
    // 1. Insert an HTML element for the token.
    InsertHTMLElement(krys::move(token));

    // 2. If the algorithm that was invoked is the generic raw text element parsing algorithm, switch the
    //    tokenizer to the RAWTEXT state; otherwise the algorithm invoked was the generic RCDATA element
    //    parsing algorithm, switch the tokenizer to the RCDATA state.
    _tokenizer.State(TokenizerState::RCDATA);

    // 3. Set the original insertion mode to the current insertion mode.
    _originalInsertionMode = _insertionMode;

    // 4. Then, switch the insertion mode to "text".
    _insertionMode = InsertionMode::Text;
  }

  Ref<dom::Element> TreeBuilder::CreateElement(dom::DOMStringAtom name, dom::DOMStringAtom namespaceURI,
                                               const ParsedAttributeList &attributes,
                                               dom::ContainerNode &intendedParent) noexcept
  {
    // https://html.spec.whatwg.org/#create-an-element-for-the-token
    // 1.-2. (There's no speculative parser.)
    // 3. Let document be intendedParent's node document.
    auto &document = intendedParent.NodeDocument();

    // 4. Let localName be token's tag name.
    // 5. Let sanitizerConfig be the parser's parser sanitizer configuration.
    // 6. Let sanitizerAction be "Keep".
    // 7. If sanitizerConfig is not null, then set sanitizerAction to the result of getting the sanitizer
    // action for an element name given localName, namespace, and
    //    sanitizerConfig.
    auto sanitizerAction = SanitizerActionFor(name.View(), namespaceURI);

    // 8. Let is be the value of the "is" attribute in token, if such an attribute exists; otherwise null.
    auto is = [&]() -> dom::DOMStringAtom
    {
      auto it = std::ranges::find_if(attributes,
                                     [](const ParsedAttribute &attr) { return attr.NameView() == u8"is"; });

      return it != std::ranges::end(attributes) ? dom::DOMStringAtom(it->ValueView())
                                                : dom::DOMStringAtom::Null();
    }();

    // 9. If is is non-null, sanitizerConfig is non-null, and the result of checking if a sanitizer config
    //    allows an attribute given "is", null, is, localName, namespace, and sanitizerConfig is false, then
    //    set is to null.
    if (is != dom::DOMStringAtom::Null() && _sanitizerConfig
        && !SanitizerAlgorithms::AllowsAttribute(u8"is", dom::DOMStringAtom::Null(), is.View(), name.View(),
                                                 namespaceURI, *_sanitizerConfig))
    {
      is = dom::DOMStringAtom::Null();
    }

    // 10. Let registry be the result of looking up a custom element registry given intendedParent.
    RefPtr<CustomElementRegistry> registry =
      ShareRefPtr(CustomElementAlgorithms::LookUpCustomElementRegistry(intendedParent));

    // 11. Let creationRegistry be registry if sanitizerAction is "Keep"; otherwise null.
    // (Elements that the sanitizer will remove or replace with their children are created with a null custom
    // element registry, so creating an element finds no custom element definition and no custom element
    // constructor runs for them.)
    auto creationRegistry = sanitizerAction == SanitizerAction::Keep ? registry : nullptr;

    // 12. Let definition be null if sanitizerAction is not "Keep"; otherwise the result of looking up a
    //     custom element definition given registry, namespace, localName, and is.
    // (Customized built-in elements aren't supported, so is isn't used.)
    auto *definition =
      sanitizerAction == SanitizerAction::Keep
        ? CustomElementAlgorithms::LookupCustomElementDefinition(registry.get(), namespaceURI, name)
        : nullptr;

    // 13. Let willExecuteScript be true if definition is non-null and the parser's fragment context element
    //     is null; otherwise false.
    bool willExecuteScript = definition != nullptr && !_context.has_value();

    Maybe<CustomElementReactionStack> reactionStack;
    // 14. If willExecuteScript is true:
    if (willExecuteScript)
    {
      // 14a. Increment document's throw-on-dynamic-markup-insertion counter.
      // 14b. If the JavaScript execution context stack is empty, then perform a microtask checkpoint.
      EventLoop::Current().PerformMicrotaskCheckpoint();
      // 14c. Push a new element queue onto document's relevant agent's custom element reactions stack.
      reactionStack.emplace();
    }

    // 15. Let element be the result of creating an element given document, localName, namespace, prefix, is,
    //     willExecuteScript, and creationRegistry.
    auto element = dom::ElementFactory::Create(document, {namespaceURI, dom::DOMStringAtom::Null(), name}, is,
                                               willExecuteScript, creationRegistry);

    // 16. If sanitizerAction is "Replace with children", then set element's custom element registry to
    //     registry.
    if (sanitizerAction == SanitizerAction::ReplaceWithChildren)
    {
      element->SetCustomElementRegistry(krys::move(registry));
    }
    // 17. For each attribute of token's attributes:
    for (auto &attr : attributes)
    {
      if (attr.Flags == ParsedAttributeFlags::None)
      {
        // 17a. If sanitizerConfig is not null: [...] if the result of checking if a sanitizer config allows
        // an
        //     attribute given attrLocalName, attrNamespace, attribute's value, localName, namespace, and
        //     sanitizerConfig is false, then continue.
        if (_sanitizerConfig
            && !SanitizerAlgorithms::AllowsAttribute(attr.NameView(), dom::DOMStringAtom::Null(),
                                                     attr.ValueView(), name.View(), namespaceURI,
                                                     *_sanitizerConfig))
        {
          continue;
        }

        // 17b. Append attribute to element.
        dom::ElementAlgorithms::SetAttributeValue(*element, attr.NameView(),
                                                  dom::DOMString(attr.ValueView()));
      }
      else
      {
        auto namespaceUri = dom::DOMStringAtom::Null();
        auto prefix = dom::DOMStringAtom::Null();
        size_t prefixLength = 0uz;

        if (HasFlag(attr.Flags, ParsedAttributeFlags::IsXML))
        {
          namespaceUri = infra::Namespaces::XML;
          prefix = infra::NamespacePrefixes::XML;
          prefixLength = 4uz; // xml + :
        }

        if (HasFlag(attr.Flags, ParsedAttributeFlags::IsXLink))
        {
          namespaceUri = infra::Namespaces::XLink;
          prefix = infra::NamespacePrefixes::XLink;
          prefixLength = 6uz; // xlink + :
        }

        if (HasFlag(attr.Flags, ParsedAttributeFlags::IsXMLNS))
        {
          namespaceUri = infra::Namespaces::XMLNS;

          // xmlns:xlink has the xmlns prefix; xmlns has none.
          if (HasFlag(attr.Flags, ParsedAttributeFlags::HasPrefix))
          {
            prefix = infra::NamespacePrefixes::XMLNS;
            prefixLength = 6uz; // xmlns + :
          }
        }

        auto localName = attr.NameView().substr(prefixLength, attr.NameView().size() - prefixLength);
        if (_sanitizerConfig
            && !SanitizerAlgorithms::AllowsAttribute(localName, namespaceUri, attr.ValueView(), name.View(),
                                                     namespaceURI, *_sanitizerConfig))
        {
          continue;
        }
        dom::ElementAlgorithms::SetAttributeValue(*element, localName, dom::DOMString(attr.ValueView()),
                                                  prefix, namespaceUri);
      }
    }

    // 18. If willExecuteScript is true: let queue be the result of popping from document's relevant
    //     agent's custom element reactions stack, invoke custom element reactions in queue, and decrement
    //     document's throw-on-dynamic-markup-insertion counter.
    reactionStack.reset();

    // 19. If element has an xmlns attribute in the XMLNS namespace whose value is not exactly the same as the
    //     element's namespace, that is a parse error. Similarly, if element has an xmlns:xlink attribute in
    //     the XMLNS namespace whose value is not the XLink Namespace, that is a parse error.
    // (Skipped: parse errors aren't reported.)

    // 20. If element is a resettable element and not a form-associated custom element, then invoke its reset
    //     algorithm.
    // (Setting the attributes in step 17 has already put a resettable element in its reset state; resetting
    // it again here would also undo what its creation set up, such as a text field's selection.)

    // 21. If element is a form-associated element and not a form-associated custom element, the form element
    //     pointer is not null, the parser is not parsing template contents, the parser's fragment context
    //     element is null, element is either not listed or doesn't have a form attribute, the intendedParent
    //     is in the same tree as the element pointed to by the form element pointer, and the parser's
    //     insertion target redirection map[the element pointed to by the form element pointer] does not
    //     exist, then associate element with the form element pointed to by the form element pointer and set
    //     element's parser inserted flag.
    // FIXME: Associating the form owner (see FormAssociatedElement::SetFormSetByParser).

    // 22. Return element.
    return element;
  }

  Ref<dom::Element> TreeBuilder::CreateElement(const StackItem &item,
                                               dom::ContainerNode *intendedParent) noexcept
  {
    auto &node = item.Element();
    auto &parent = intendedParent != nullptr ? *intendedParent : *node.ParentNode();
    return CreateElement(node.LocalName(), node.NamespaceURI(), item.Attributes(), parent);
  }

  void TreeBuilder::ReconstructActiveFormattingElements() noexcept
  {
    // 1. If there are no entries in the list of active formatting elements, then there is nothing to
    //    reconstruct; stop this algorithm.
    if (_activeFormattingElements.IsEmpty())
    {
      return;
    }

    // 2. If the last (most recently added) entry in the list of active formatting elements is a marker,
    //    or if it is an element that is in the stack of open elements, then there is nothing to reconstruct;
    //    stop this algorithm.
    auto last = _activeFormattingElements.Last();
    if (last->IsMarker() || _openElementStack.Contains(last->Item().Element()))
    {
      return;
    }

    // 3. Let entry be the last (most recently added) element in the list of active formatting elements.
    decltype(last) entry = last;

    // Only follows the Create steps of the initial Advance loop if true.
    bool skipInitialAdvance = false;

    // Rewind
    while (true)
    {
      // 4. Rewind: If there are no entries before entry in the list of active formatting elements, then
      //    jump to the step labeled create.
      if (entry == _activeFormattingElements.begin())
      {
        skipInitialAdvance = true;
        break;
      }

      // 5. Let entry be the entry one earlier than entry in the list of active formatting elements.
      entry = std::prev(entry);

      // 6. If entry is neither a marker nor an element that is also in the stack of open elements, go
      //    to the step labeled rewind.
      if (!entry->IsMarker() && !_openElementStack.Contains(entry->Item().Element()))
      {
        continue;
      }

      break;
    }

    // Advance
    while (true)
    {
      // 7. Advance: Let entry be the element one later than entry in the list of active formatting
      //    elements.
      if (!skipInitialAdvance || entry->IsMarker())
      {
        entry = std::next(entry);
      }
      skipInitialAdvance = false;

      // 8. Create: Insert an HTML element for the token for which the element entry was created, to
      //    obtain new element.
      auto &item = entry->Item();
      auto newElement = CreateElement(item);
      InsertElementAtAdjustedInsertionLocation(*newElement);
      _openElementStack.Push({item.TagName(), item.Namespace(), *newElement, item.Attributes()});

      // 9. Replace the entry for entry in the list with an entry for new element.
      entry->ReplaceItem(_openElementStack.Bottom());

      // 10. If the entry for new element in the list of active formatting elements is not the last
      //     entry in the list, return to the step labeled advance.
      if (entry != last)
      {
        continue;
      }

      break;
    }
  }

  void TreeBuilder::ClosePElement(const TokenAtom &token) noexcept
  {
    // 1. Generate implied end tags, except for p elements.
    _openElementStack.GenerateImpliedEndTags(TagName::p);

    // 2. If the current node is not a p element, then this is a parse error.
    if (_openElementStack.Bottom().TagName() != TagName::p)
    {
      ParseError(token);
    }

    // 3. Pop elements from the stack of open elements until a p element has been popped from the stack.
    _openElementStack.PopUntilPopped(TagName::p, Namespace::HTML);
  }

#pragma endregion

#pragma region Adjust Attribute Algorithms

  void TreeBuilder::AdjustForeignAttributes(TokenAtom &token) noexcept
  {
    constexpr static Array<Pair<dom::DOMStringView, ParsedAttributeFlags>, 11uz> attributesToAdjust = {
      Pair {u8"xlink:actuate", ParsedAttributeFlags::HasPrefix | ParsedAttributeFlags::IsXLink},
      Pair {u8"xlink:arcrole", ParsedAttributeFlags::HasPrefix | ParsedAttributeFlags::IsXLink},
      Pair {u8"xlink:href", ParsedAttributeFlags::HasPrefix | ParsedAttributeFlags::IsXLink},
      Pair {u8"xlink:role", ParsedAttributeFlags::HasPrefix | ParsedAttributeFlags::IsXLink},
      Pair {u8"xlink:show", ParsedAttributeFlags::HasPrefix | ParsedAttributeFlags::IsXLink},
      Pair {u8"xlink:title", ParsedAttributeFlags::HasPrefix | ParsedAttributeFlags::IsXLink},
      Pair {u8"xlink:type", ParsedAttributeFlags::HasPrefix | ParsedAttributeFlags::IsXLink},
      Pair {u8"xml:lang", ParsedAttributeFlags::HasPrefix | ParsedAttributeFlags::IsXML},
      Pair {u8"xml:space", ParsedAttributeFlags::HasPrefix | ParsedAttributeFlags::IsXML},
      Pair {u8"xmlns", ParsedAttributeFlags::IsXMLNS},
      Pair {u8"xmlns:xlink", ParsedAttributeFlags::HasPrefix | ParsedAttributeFlags::IsXMLNS},
    };

    for (auto &attr : token.Attributes())
    {
      for (auto &[name, flags] : attributesToAdjust)
      {
        if (attr.NameView() == name)
        {
          attr.Flags = flags;
          break;
        }
      }
    }
  }

  void TreeBuilder::AdjustSVGAttributes(TokenAtom &token) noexcept
  {
    constexpr static Array<Array<dom::DOMStringView, 2uz>, 58uz> attributesToAdjust = {
      Array<dom::DOMStringView, 2> {u8"attributename", u8"attributeName"},
      Array<dom::DOMStringView, 2> {u8"attributetype", u8"attributeType"},
      Array<dom::DOMStringView, 2> {u8"basefrequency", u8"baseFrequency"},
      Array<dom::DOMStringView, 2> {u8"baseprofile", u8"baseProfile"},
      Array<dom::DOMStringView, 2> {u8"calcmode", u8"calcMode"},
      Array<dom::DOMStringView, 2> {u8"clippathunits", u8"clipPathUnits"},
      Array<dom::DOMStringView, 2> {u8"diffuseconstant", u8"diffuseConstant"},
      Array<dom::DOMStringView, 2> {u8"edgemode", u8"edgeMode"},
      Array<dom::DOMStringView, 2> {u8"filterunits", u8"filterUnits"},
      Array<dom::DOMStringView, 2> {u8"glyphref", u8"glyphRef"},
      Array<dom::DOMStringView, 2> {u8"gradienttransform", u8"gradientTransform"},
      Array<dom::DOMStringView, 2> {u8"gradientunits", u8"gradientUnits"},
      Array<dom::DOMStringView, 2> {u8"kernelmatrix", u8"kernelMatrix"},
      Array<dom::DOMStringView, 2> {u8"kernelunitlength", u8"kernelUnitLength"},
      Array<dom::DOMStringView, 2> {u8"keypoints", u8"keyPoints"},
      Array<dom::DOMStringView, 2> {u8"keysplines", u8"keySplines"},
      Array<dom::DOMStringView, 2> {u8"keytimes", u8"keyTimes"},
      Array<dom::DOMStringView, 2> {u8"lengthadjust", u8"lengthAdjust"},
      Array<dom::DOMStringView, 2> {u8"limitingconeangle", u8"limitingConeAngle"},
      Array<dom::DOMStringView, 2> {u8"markerheight", u8"markerHeight"},
      Array<dom::DOMStringView, 2> {u8"markerunits", u8"markerUnits"},
      Array<dom::DOMStringView, 2> {u8"markerwidth", u8"markerWidth"},
      Array<dom::DOMStringView, 2> {u8"maskcontentunits", u8"maskContentUnits"},
      Array<dom::DOMStringView, 2> {u8"maskunits", u8"maskUnits"},
      Array<dom::DOMStringView, 2> {u8"numoctaves", u8"numOctaves"},
      Array<dom::DOMStringView, 2> {u8"pathlength", u8"pathLength"},
      Array<dom::DOMStringView, 2> {u8"patterncontentunits", u8"patternContentUnits"},
      Array<dom::DOMStringView, 2> {u8"patterntransform", u8"patternTransform"},
      Array<dom::DOMStringView, 2> {u8"patternunits", u8"patternUnits"},
      Array<dom::DOMStringView, 2> {u8"pointsatx", u8"pointsAtX"},
      Array<dom::DOMStringView, 2> {u8"pointsaty", u8"pointsAtY"},
      Array<dom::DOMStringView, 2> {u8"pointsatz", u8"pointsAtZ"},
      Array<dom::DOMStringView, 2> {u8"preservealpha", u8"preserveAlpha"},
      Array<dom::DOMStringView, 2> {u8"preserveaspectratio", u8"preserveAspectRatio"},
      Array<dom::DOMStringView, 2> {u8"primitiveunits", u8"primitiveUnits"},
      Array<dom::DOMStringView, 2> {u8"refx", u8"refX"},
      Array<dom::DOMStringView, 2> {u8"refy", u8"refY"},
      Array<dom::DOMStringView, 2> {u8"repeatcount", u8"repeatCount"},
      Array<dom::DOMStringView, 2> {u8"repeatdur", u8"repeatDur"},
      Array<dom::DOMStringView, 2> {u8"requiredextensions", u8"requiredExtensions"},
      Array<dom::DOMStringView, 2> {u8"requiredfeatures", u8"requiredFeatures"},
      Array<dom::DOMStringView, 2> {u8"specularconstant", u8"specularConstant"},
      Array<dom::DOMStringView, 2> {u8"specularexponent", u8"specularExponent"},
      Array<dom::DOMStringView, 2> {u8"spreadmethod", u8"spreadMethod"},
      Array<dom::DOMStringView, 2> {u8"startoffset", u8"startOffset"},
      Array<dom::DOMStringView, 2> {u8"stddeviation", u8"stdDeviation"},
      Array<dom::DOMStringView, 2> {u8"stitchtiles", u8"stitchTiles"},
      Array<dom::DOMStringView, 2> {u8"surfacescale", u8"surfaceScale"},
      Array<dom::DOMStringView, 2> {u8"systemlanguage", u8"systemLanguage"},
      Array<dom::DOMStringView, 2> {u8"tablevalues", u8"tableValues"},
      Array<dom::DOMStringView, 2> {u8"targetx", u8"targetX"},
      Array<dom::DOMStringView, 2> {u8"targety", u8"targetY"},
      Array<dom::DOMStringView, 2> {u8"textlength", u8"textLength"},
      Array<dom::DOMStringView, 2> {u8"viewbox", u8"viewBox"},
      Array<dom::DOMStringView, 2> {u8"viewtarget", u8"viewTarget"},
      Array<dom::DOMStringView, 2> {u8"xchannelselector", u8"xChannelSelector"},
      Array<dom::DOMStringView, 2> {u8"ychannelselector", u8"yChannelSelector"},
      Array<dom::DOMStringView, 2> {u8"zoomandpan", u8"zoomAndPan"},
    };

    for (auto &attr : token.Attributes())
    {
      for (auto &[name, adjustedName] : attributesToAdjust)
      {
        if (attr.NameView() == name)
        {
          attr.SetName(adjustedName);
          break;
        }
      }
    }
  }

  void TreeBuilder::AdjustMathMLAttributes(TokenAtom &token) noexcept
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

  bool TreeBuilder::IsQuirksModeDOCTYPE(const TokenAtom &token) const noexcept
  {
    krys_debug_assert(token.Type() == TokenType::DOCTYPE);

    if (token.IsForceQuirks() || token.Name() != u8"html")
    {
      return true;
    }

    auto &data = *token.DOCTYPEData();

    if (data.HasSystemIdentifier)
    {
      auto systemIdentifier = dom::DOMStringView(data.SystemIdentifier.begin(), data.SystemIdentifier.end());
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(
            systemIdentifier, u8"http://www.ibm.com/data/dtd/v11/ibmxhtml1-transitional.dtd"))
      {
        return true;
      }
    }

    if (data.HasPublicIdentifier)
    {
      auto publicIdentifier = dom::DOMStringView(data.PublicIdentifier.begin(), data.PublicIdentifier.end());

      auto Matches = [publicIdentifier](dom::DOMStringView identifier)
      {
        return infra::StringAlgorithms::ASCIICaseInsensitiveMatch(publicIdentifier, identifier);
      };

      auto StartsWith = [publicIdentifier](dom::DOMStringView identifier)
      {
        if (publicIdentifier.size() < identifier.size())
        {
          return false;
        }

        auto publicIdentifierPrefix = publicIdentifier.substr(0uz, identifier.size());
        return infra::StringAlgorithms::ASCIICaseInsensitiveMatch(publicIdentifierPrefix, identifier);
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

  bool TreeBuilder::IsLimitedQuirksModeDOCTYPE(const TokenAtom &token) const noexcept
  {
    krys_debug_assert(token.Type() == TokenType::DOCTYPE);

    auto &data = *token.DOCTYPEData();

    if (data.HasPublicIdentifier)
    {
      auto publicIdentifier = dom::DOMStringView(data.PublicIdentifier.begin(), data.PublicIdentifier.end());

      auto StartsWith = [publicIdentifier](dom::DOMStringView identifier)
      {
        if (publicIdentifier.size() < identifier.size())
        {
          return false;
        }

        auto publicIdentifierPrefix = publicIdentifier.substr(0uz, identifier.size());
        return infra::StringAlgorithms::ASCIICaseInsensitiveMatch(publicIdentifierPrefix, identifier);
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

  void TreeBuilder::RunAdoptionAgency(TokenAtom &token) noexcept
  {
    // 1. Let subject be token's tag name.
    auto subject = ParseTagName(token.Name().View());

    // 2. If the current node is an HTML element whose tag name is subject, and the current node is not
    //    in the list of active formatting elements, then pop the current node off the stack of open elements
    //    and return.
    auto &currentNode = _openElementStack.Bottom();
    if (currentNode.Namespace() == Namespace::HTML && currentNode.TagName() == subject
        && !_activeFormattingElements.Contains(currentNode.Element()))
    {
      _openElementStack.Pop();
      return;
    }

    // 3. Let outerLoopCounter be 0.
    size_t outerLoopCounter = 0uz;

    // 4. While true:
    while (true)
    {
      // 4a. If outerLoopCounter is greater than or equal to 8, then return.
      if (outerLoopCounter >= 8uz)
      {
        return;
      }

      // 4b. Increment outerLoopCounter by 1.
      outerLoopCounter++;

      // 4c. Let formattingElement be the last element in the list of active formatting elements that:
      //     - is between the end of the list and the last marker in the list, if any, or the start of the
      //     list otherwise
      //     - has the tag name subject.
      //     If there is no such element, then act as described in the "any other end tag" entry above and
      //     return.
      StackItem *formattingElement = _activeFormattingElements.FindFromLastMarker(subject, Namespace::HTML);

      if (formattingElement == nullptr)
      {
        InBodyGenericEndTag(token, subject);
        return;
      }

      // This is just for convenience; the stack item will only be destroyed right before the function
      // returns so the reference will remain valid regardless.
      auto &formattingElementNode = formattingElement->Element();

      // 4d. If formattingElement is not in the stack of open elements, then this is a parse error;
      //     remove the element from the list, and return.
      if (!_openElementStack.Contains(formattingElementNode))
      {
        ParseError(token);
        _activeFormattingElements.Remove(formattingElementNode);
        return;
      }

      // 4e. If formattingElement is in the stack of open elements, but the element is not in scope,
      //     then this is a parse error; return.
      if (!_openElementStack.HasElementInScope(formattingElementNode))
      {
        ParseError(token);
        return;
      }

      // 4f. If formattingElement is not the current node, this is a parse error. (But do not return.)
      if (&_openElementStack.Bottom().Element() != &formattingElementNode)
      {
        ParseError(token);
      }

      // 4g. Let furthestBlock be the topmost node in the stack of open elements that is lower in the
      //     stack than formattingElement, and is an element in the special category. There might not be one.
      StackItem *furthestBlock = FurthestSpecialElementBlock(formattingElementNode);

      // 4h. If there is no furthestBlock, then the UA must first pop all the nodes from the bottom of
      //     the stack of open elements, from the current node up to and including formattingElement, then
      //     remove formattingElement from the list of active formatting elements, and finally return.
      if (furthestBlock == nullptr)
      {
        _openElementStack.PopUntilPopped(formattingElementNode);
        _activeFormattingElements.Remove(formattingElementNode);
        return;
      }

      // Save a reference to the node now in case the stack item gets destroyed. The node will still be
      // alive.
      auto &furthestBlockNode = furthestBlock->Element();

      // 4i. Let commonAncestor be the element immediately above formattingElement in the stack of open
      //     elements.
      StackItem *commonAncestor = _openElementStack.EntryBefore(formattingElementNode);

      // 4j. Let a bookmark note the position of formattingElement in the list of active formatting
      //     elements relative to the elements on either side of it in the list.
      auto bookmark = _activeFormattingElements.BookmarkFor(formattingElementNode);

      // 4k. Let node and lastNode be furthestBlock.
      dom::Element *node = &furthestBlockNode;
      dom::Element *lastNode = node;

      // 4l. Let pendingRedirects be an empty list.
      // (An element that is replaced with its children is not itself in the tree, so its insertion target
      // redirection map entry has to be updated when this algorithm changes where it would have been
      // inserted. The new location is only known once a later element is not itself replaced with its
      // children, so such elements are collected in pendingRedirects until then.)
      List<Ref<dom::Element>> pendingRedirects;

      // 4m. If the parser's parser sanitizer configuration is not null: let action be the result of getting
      //     the sanitizer action for an element name given lastNode's local name, lastNode's namespace, and
      //     the parser's parser sanitizer configuration. If action is not "Keep", then set lastNode to null.
      //     If action is "Replace with children", then append furthestBlock to pendingRedirects.
      auto furthestBlockAction =
        SanitizerActionFor(furthestBlockNode.LocalName().View(), furthestBlockNode.NamespaceURI());
      if (furthestBlockAction != SanitizerAction::Keep)
      {
        lastNode = nullptr;
      }
      if (furthestBlockAction == SanitizerAction::ReplaceWithChildren)
      {
        pendingRedirects.push_back(ShareRef(furthestBlockNode));
      }

      // 4n. Let innerLoopCounter be 0.
      size_t innerLoopCounter = 0uz;

      // 4o. Let bookmarkMoved be false.
      bool bookmarkMoved = false;

      auto ElementAbove = [&](dom::Element &node) -> dom::Element *
      {
        StackItem *entry = _openElementStack.EntryBefore(node);
        return entry != nullptr ? &entry->Element() : nullptr;
      };
      dom::Element *immediatelyAbove = ElementAbove(*node);

      // 4p. While true:
      while (true)
      {
        // 4p.i. Increment innerLoopCounter by 1.
        ++innerLoopCounter;

        // 4p.ii. Let node be the element immediately above node in the stack of open elements, or if
        //        node is no longer in the stack of open elements (e.g. because it got removed by this
        //        algorithm), the element that was immediately above node in the stack of open elements before
        //        node was removed.
        node = immediatelyAbove;
        krys_debug_assert(node != nullptr);

        // Fetch the next node now in case the stack item gets destroyed.
        immediatelyAbove = ElementAbove(*node);

        // 4p.iii. If node is formattingElement, then break.
        if (node == &formattingElementNode)
        {
          break;
        }

        // 4p.iv. If innerLoopCounter is greater than 3 and node is in the list of active formatting
        //        elements, then remove node from the list of active formatting elements.
        if (innerLoopCounter > 3uz && _activeFormattingElements.Contains(*node))
        {
          _activeFormattingElements.RemoveAndUpdateBookmark(*node, bookmark);
        }

        // 4p.v. If node is not in the list of active formatting elements, then remove node from the
        //       stack of open elements and continue.
        // We retrieve the entry for node here in case the previous step removes it.
        FormattingListEntry *nodeFormattingEntry = _activeFormattingElements.Find(*node);
        if (nodeFormattingEntry == nullptr)
        {
          _openElementStack.Remove(*node);
          continue;
        }

        // 4p.vi. Create an element for the token for which the element node was created, in the HTML
        //        namespace, with commonAncestor as the intended parent; replace the entry for node in the
        //        list of active formatting elements with an entry for the new element, replace the entry for
        //        node in the stack of open elements with an entry for the new element, and set node to the
        //        new element.
        auto newElement = CreateElement(nodeFormattingEntry->Item(), &commonAncestor->Element());
        nodeFormattingEntry->Item().UpdateElement(*newElement);
        _openElementStack.Find(*node)->UpdateElement(*newElement);
        node = newElement.get();

        // 4p.vii. If bookmarkMoved is false, then move the aforementioned bookmark to be immediately after
        // the
        //         new node in the list of active formatting elements, and set bookmarkMoved to true.
        if (!bookmarkMoved)
        {
          _activeFormattingElements.MoveBookmarkAfter(bookmark, *nodeFormattingEntry);
          bookmarkMoved = true;
        }

        // 4p.viii.-4p.ix. Let nodeAction be "Keep". If the parser's parser sanitizer configuration is not
        // null, then
        //                 set nodeAction to the result of getting the sanitizer action for an element name
        //                 given node's local name, node's namespace, and the parser's parser sanitizer
        //                 configuration.
        auto nodeAction = SanitizerActionFor(node->LocalName().View(), node->NamespaceURI());

        // 4p.x. If nodeAction is not "Replace with children": for each replacedElement of pendingRedirects:
        //       set the parser's insertion target redirection map[replacedElement] to (node, null); empty
        //       pendingRedirects.
        if (nodeAction != SanitizerAction::ReplaceWithChildren)
        {
          for (auto &replacedElement : pendingRedirects)
          {
            RedirectInsertions(*replacedElement, {.Parent = node});
          }
          pendingRedirects.clear();
        }

        // 4p.xi. Switch on nodeAction:
        switch (nodeAction)
        {
          // "Keep": if lastNode is non-null, then append lastNode to node. Set lastNode to node.
          case SanitizerAction::Keep:
          {
            if (lastNode)
            {
              (void)dom::MutationAlgorithms::Append(*lastNode, *node);
            }
            lastNode = node;
            break;
          }
          // "Remove": if lastNode is non-null and lastNode's parent is non-null, then remove lastNode.
          // Set lastNode to null.
          case SanitizerAction::Remove:
          {
            if (lastNode && lastNode->ParentNode())
            {
              (void)dom::MutationAlgorithms::Remove(*lastNode);
            }
            lastNode = nullptr;
            break;
          }
          // "Replace with children": append node to pendingRedirects.
          case SanitizerAction::ReplaceWithChildren:
          {
            pendingRedirects.push_back(ShareRef(*node));
            break;
          }
        }
      }

      // 4q. Let insertionLocation be the appropriate place for inserting a node given commonAncestor.
      auto insertionLocation = AppropriatePlaceToInsertNode(&commonAncestor->Element());

      // 4r. Let (target, refNode) be the adjusted insertion location given insertionLocation.
      auto [target, refNode] = AdjustedInsertionLocation(insertionLocation);

      // 4s. For each replacedElement of pendingRedirects: set the parser's insertion target redirection
      //     map[replacedElement] to (target, refNode).
      for (auto &replacedElement : pendingRedirects)
      {
        RedirectInsertions(*replacedElement, {.Parent = target, .BeforeSibling = refNode});
      }

      // 4t. If lastNode is non-null and lastNode's parent is non-null, then remove lastNode.
      if (lastNode && lastNode->ParentNode())
      {
        (void)dom::MutationAlgorithms::Remove(*lastNode);
      }

      // 4u. If all of the following are true: lastNode is non-null; lastNode's parent is null; lastNode is
      //     not a host-including inclusive ancestor of target; target is not a Document node, or it does not
      //     have an element child; and refNode is null or its parent is target, then insert lastNode into
      //     target before refNode.
      if (lastNode && !lastNode->ParentNode()
          && !dom::TreeQueries::IsHostIncludingInclusiveAncestorOf(*lastNode, *target)
          && (!Is<dom::Document>(*target) || !dom::TreeQueries::FirstElementChild(*target))
          && (!refNode || refNode->ParentNode() == target))
      {
        (void)dom::MutationAlgorithms::Insert(*lastNode, *target, refNode);
      }

      // 4v. Let newElement be the result of creating an element for the token for which formattingElement
      //     was created, in the HTML namespace, with furthestBlock as the intended parent.
      auto newElement = CreateElement(*formattingElement, &furthestBlockNode);

      auto newTagName = formattingElement->TagName();
      auto newTagNamespace = formattingElement->Namespace();
      auto newTagAttributes = ParsedAttributeList(formattingElement->Attributes());
      auto newFormattingElementItem = StackItem(newTagName, newTagNamespace, *newElement, newTagAttributes);
      auto newStackItem = StackItem(newTagName, newTagNamespace, *newElement, krys::move(newTagAttributes));

      // 4w. Remove formattingElement from the list of active formatting elements, and insert newElement
      //     into the list of active formatting elements at the position of the aforementioned bookmark.
      _activeFormattingElements.RemoveAndUpdateBookmark(formattingElementNode, bookmark);
      _activeFormattingElements.InsertAtBookmark(krys::move(newFormattingElementItem), bookmark);

      // 4x. Remove formattingElement from the stack of open elements, and insert newElement into the stack
      //     of open elements immediately below the position of furthestBlock in that stack.
      _openElementStack.Remove(formattingElementNode);
      _openElementStack.InsertBelow(krys::move(newStackItem), furthestBlockNode);

      // 4y.-4z. Let sanitizerAction be "Keep". If the parser's parser sanitizer configuration is not null,
      // then
      //         set sanitizerAction to the result of getting the sanitizer action for an element name given
      //         newElement's local name, newElement's namespace, and the parser's parser sanitizer
      //         configuration.
      // 4aa. Switch on sanitizerAction:
      switch (SanitizerActionFor(newElement->LocalName().View(), newElement->NamespaceURI()))
      {
        // "Replace with children": set the parser's insertion target redirection map[newElement] to the
        // adjusted insertion location given (furthestBlock, null).
        case SanitizerAction::ReplaceWithChildren:
        {
          RedirectInsertions(*newElement,
                             AdjustedInsertionLocation(InsertionLocation {.Parent = &furthestBlockNode}));
          break;
        }
        // "Remove": take all of the child nodes of furthestBlock and append them to newElement.
        case SanitizerAction::Remove:
        {
          while (auto *child = furthestBlockNode.FirstChild())
          {
            (void)dom::MutationAlgorithms::Append(*child, *newElement);
          }
          break;
        }
        // "Keep": take all of the child nodes of furthestBlock and append them to newElement. Let
        // (newElementParent, newElementRefNode) be the adjusted insertion location given (furthestBlock,
        // null). Insert newElement into newElementParent before newElementRefNode.
        case SanitizerAction::Keep:
        {
          while (auto *child = furthestBlockNode.FirstChild())
          {
            (void)dom::MutationAlgorithms::Append(*child, *newElement);
          }
          auto [newElementParent, newElementRefNode] =
            AdjustedInsertionLocation(InsertionLocation {.Parent = &furthestBlockNode});
          (void)dom::MutationAlgorithms::Insert(*newElement, *newElementParent, newElementRefNode);
          break;
        }
      }

      // 4ab. Let belowNode be the element immediately below newElement in the stack of open elements, if
      //      any; otherwise null.
      // 4ac. While belowNode is non-null and the parser's insertion target redirection map[belowNode] exists:
      //      set the parser's insertion target redirection map[belowNode] to the adjusted insertion location
      //      given (newElement, null); set belowNode to the element immediately below belowNode in the stack
      //      of open elements, if any; otherwise null.
      // (Elements below furthestBlock in the stack that were replaced with their children now have newElement
      // between them and furthestBlock.)
      for (auto *below = _openElementStack.EntryAfter(*newElement);
           below && _insertionTargetRedirectionMap.contains(&below->Element());
           below = _openElementStack.EntryAfter(below->Element()))
      {
        RedirectInsertions(below->Element(),
                           AdjustedInsertionLocation(InsertionLocation {.Parent = newElement.get()}));
      }
    }
  }

  StackItem *TreeBuilder::FurthestSpecialElementBlock(const dom::Element &formattingElement) noexcept
  {
    // Get the index of the formatting element in the open element stack.
    size_t formattingIndex = std::distance(
      _openElementStack.begin(), std::ranges::find_if(_openElementStack, [&](const auto &item)
                                                      { return &item.Element() == &formattingElement; }));

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

  bool TreeBuilder::IsMathMLTextIntegrationPoint(const dom::Element &element) const noexcept
  {
    if (element.NamespaceURI() == infra::Namespaces::MathML)
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

  bool TreeBuilder::IsHTMLIntegrationPoint(const dom::Element &element) const noexcept
  {
    if (element.NamespaceURI() == infra::Namespaces::MathML)
    {
      if (element.LocalName() == u8"annotation-xml")
      {
        auto attr = element.GetAttribute(u8"encoding");
        if (attr.has_value())
        {
          auto &encoding = attr.value();
          if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(encoding, u8"text/html")
              || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(encoding, u8"application/xhtml+xml"))
          {
            return true;
          }
        }

        return false;
      }
    }
    else if (element.NamespaceURI() == infra::Namespaces::SVG)
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
