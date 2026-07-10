#include "Krystal.HTML/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.HTML/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Internals/ElementFactory.hpp"

namespace Krys::HTML
{
  HTMLDocumentParser::HTMLDocumentParser(HTMLDocument &document) noexcept
      : _document(document), _input(),
        _tokenizer(_input, [&]() { return _treeBuilder.IsCDATASectionAllowedInCurrentContext(); }),
        _treeBuilder(document, _tokenizer, _fragmentParsingContext)
  {
  }

  SmallNodeList HTMLDocumentParser::ParseFragment(Element &context, utf32_string &&input,
                                                  bool allowDeclarativeShadowRoots,
                                                  ParserScriptingMode scriptingMode) noexcept
  {
    // FPA(1): Assert: scriptingMode is either Inert or Fragment.
    assert(scriptingMode == ParserScriptingMode::Inert || scriptingMode == ParserScriptingMode::Fragment);

    // FPA(2): Let document be a Document node whose type is "html".
    auto document = CreateRef<HTMLDocument>();

    // FPA(3): Let contextDocument be context's node document.
    auto &contextDocument = context.NodeDocument();

    // FPA(4): If contextDocument is in quirks mode, then set document's mode to "quirks".
    if (contextDocument._quirksMode == QuirksMode::Quirks)
    {
      document->_quirksMode = QuirksMode::Quirks;
    }

    // FPA(5): Otherwise, if context's node document is in limited-quirks mode, then set document's mode to
    // "limited-quirks".
    else if (contextDocument._quirksMode == QuirksMode::LimitedQuirks)
    {
      document->_quirksMode = QuirksMode::LimitedQuirks;
    }

    // FPA(6): If allowDeclarativeShadowRoots is true, then set document's allow declarative shadow roots to
    // true.
    if (allowDeclarativeShadowRoots)
    {
      document->_allowDeclarativeShadowRoots = true;
    }

    // FPA(7): Create a new HTML parser, and associate it with document.
    auto parser = CreateRef<HTMLDocumentParser>(*document);

    // FPA(8): If contextDocument's scripting is disabled, then set scriptingMode to Disabled.
    if (contextDocument.IsScriptingDisabled())
    {
      scriptingMode = ParserScriptingMode::Disabled;
    }

    // FPA(9): Set the parser's scripting mode to scriptingMode.
    parser->ScriptingMode(scriptingMode);

    // FPA(10): Set the state of the HTML parser's tokenization stage as follows, switching on the context
    // element:
    auto contextTagName = ParseTagName(context.LocalName().View());
    auto contextNamespace = ParseNamespace(context.NamespaceURI().View());
    switch (contextTagName)
    {
      // Switch the tokenizer to the RCDATA state.
      case TagName::title:
      case TagName::textarea:
      {
        if (contextNamespace == Namespace::HTML)
        {
          parser->TokenizerState(TokenizerState::RCDATA);
        }

        break;
      }
      // Switch the tokenizer to the RAWTEXT state.
      case TagName::style:
      case TagName::xmp:
      case TagName::iframe:
      case TagName::noembed:
      case TagName::noframes:
      {
        if (contextNamespace == Namespace::HTML)
        {
          parser->TokenizerState(TokenizerState::RAWTEXT);
        }

        break;
      }
      // Switch the tokenizer to the script data state.
      case TagName::script:
      {
        if (contextNamespace == Namespace::HTML)
        {
          parser->TokenizerState(TokenizerState::ScriptData);
        }

        break;
      }
      // If scripting mode is not Disabled, switch the tokenizer to the RAWTEXT state. Otherwise, leave the
      // tokenizer in the data state.
      case TagName::noscript:
      {
        if (contextNamespace == Namespace::HTML && parser->ScriptingMode() != ParserScriptingMode::Disabled)
        {
          parser->TokenizerState(TokenizerState::RAWTEXT);
        }

        break;
      }
      // Switch the tokenizer to the PLAINTEXT state.
      case TagName::plaintext:
      {
        if (contextNamespace == Namespace::HTML)
        {
          parser->TokenizerState(TokenizerState::PLAINTEXT);
        }

        break;
      }
    }

    // FPA(11): Let root be the result of creating an element given document, "html", the HTML namespace,
    // null, null, false, and context's custom element registry.
    QualifiedName name {Namespaces::HTML, DOMStringAtom::Null(), u8"html", TagName::html, Namespace::HTML};
    auto root = ElementFactory::Create(*document, name, DOMStringAtom::Null(), false,
                                       contextDocument.CustomElementRegistry());

    // FPA(12): Append root to document.
    (void)MutationAlgorithms::Append(*root, *document);

    // FPA(13): Set up the HTML parser's stack of open elements so that it contains just the single element
    // root.
    parser->OpenElementStack().Push(HTMLStackItem(TagName::html, Namespace::HTML, *root, {}));

    // FPA(14): If context is a template element, then push "in template" onto the stack of template insertion
    // modes so that it is the new current template insertion mode.
    if (Is<HTMLTemplateElement>(context))
    {
      parser->_treeBuilder.PushTemplateInsertionMode(InsertionMode::InTemplate);
    }

    // FPA(15): Create a start tag token whose name is the local name of context and whose attributes are the
    // attributes of context.
    // Let this start tag token be the start tag token of context; e.g. for the purposes of
    // determining if it is an HTML integration point.
    ParsedAttributeList attributes;
    for (const auto &attrName : context.GetAttributeNames())
    {
      ParsedAttribute attribute {};
      attribute.Name.assign(attrName.begin(), attrName.end());

      auto attrValue = context.GetAttribute(attrName);
      if (attrValue.has_value())
      {
        attribute.Value.assign(attrValue->begin(), attrValue->end());
      }
    }
    parser->_fragmentParsingContext.emplace(contextTagName, contextNamespace, context,
                                            Krys::Move(attributes));

    // FPA(16): Reset the parser's insertion mode appropriately.
    parser->ResetInsertionModeAppropriately();

    // FPA(17): Set the HTML parser's form element pointer to the nearest node to context that is a form
    // element (going straight up the ancestor chain, and including the element itself, if it is a form
    // element), if any. (If there is no such form element, the form element pointer keeps its initial value,
    // null.)
    auto range = InclusiveAncestorRange(context);
    auto form = FirstOfType<HTMLFormElement>(range);
    if (form != range.end())
    {
      parser->FormElement(Downcast<HTMLFormElement>(*form));
    }

    // FPA(18): Place the input into the input stream for the HTML parser just created. The encoding
    // confidence is irrelevant.
    parser->InputStream().Append(Krys::Move(input), IsEOF(true));

    // FPA(19): Start the HTML parser and let it run until it has consumed all the characters just inserted
    // into the input stream.
    parser->PumpTokenizer();

    // FPA(20): Return root's children, in tree order.
    SmallNodeList result;
    for (Node &child : ChildNodeRange(*root))
    {
      result.emplace_back(ShareRef(child));
    }
    return result;
  }

  void HTMLDocumentParser::PumpTokenizer() noexcept
  {
    while (!IsStopped())
    {
      NextTokenPtr token = _tokenizer.NextToken();
      if (!token)
      {
        return;
      }

      HTMLTokenAtom tokenAtom(*token);
      _treeBuilder.ProcessToken(Krys::Move(tokenAtom));
    }

    return;
  }
}