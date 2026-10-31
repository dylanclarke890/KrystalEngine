#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/Page/Settings.hpp"

namespace krys::boo::html
{
  HTMLDocumentParser::HTMLDocumentParser(dom::HTMLDocument &document) noexcept
      : _document(document), _input(),
        _tokenizer(_input, [&]() { return _treeBuilder.IsCDATASectionAllowedInCurrentContext(); }),
        _treeBuilder(document, _tokenizer, _fragmentParsingContext)
  {
  }

  Ref<dom::DocumentFragment>
    HTMLDocumentParser::ParseFragment(dom::ContainerNode &target, utf8_string &&input,
                                      bool allowDeclarativeShadowRoots, ParserScriptingMode scriptingMode,
                                      Maybe<SanitizerConfig> &&configuration) noexcept
  {
    // FPA(1): Assert: scriptingMode is either Inert or Fragment.
    krys_debug_assert(scriptingMode == ParserScriptingMode::Inert
                      || scriptingMode == ParserScriptingMode::Fragment);

    // FPA: Let context be target if target is an Element; otherwise target's host.
    // FPA: Assert: context is non-null.
    auto *contextElement = DynamicDowncast<dom::Element>(target);
    if (!contextElement)
    {
      contextElement = Downcast<dom::DocumentFragment>(target).Host();
    }
    krys_assert(contextElement);
    auto &context = *contextElement;

    // FPA(2): Let document be a Document node whose type is "html".
    auto document = CreateRef<dom::HTMLDocument>();

    // FPA(3): Let contextDocument be context's node document.
    auto &contextDocument = context.NodeDocument();
    document->SetSettings(ShareRef(contextDocument.GetSettings()));

    // FPA(4): If contextDocument is in quirks mode, then set document's mode to "quirks".
    if (contextDocument._quirksMode == dom::QuirksMode::Quirks)
    {
      document->SetQuirksMode(dom::QuirksMode::Quirks);
    }

    // FPA(5): Otherwise, if context's node document is in limited-quirks mode, then set document's mode to
    // "limited-quirks".
    else if (contextDocument._quirksMode == dom::QuirksMode::LimitedQuirks)
    {
      document->SetQuirksMode(dom::QuirksMode::LimitedQuirks);
    }

    // FPA(6): If allowDeclarativeShadowRoots is true, then set document's allow declarative shadow roots to
    // true.
    if (allowDeclarativeShadowRoots)
    {
      document->_allowDeclarativeShadowRoots = true;
    }

    // FPA(7): Create a new HTML parser whose parser sanitizer configuration is configuration [...], and
    // associate it with document.
    auto parser = CreateRef<HTMLDocumentParser>(*document);
    bool sanitizing = configuration.has_value();
    parser->SanitizerConfiguration(krys::move(configuration));

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
    // null, null, false, and the result of looking up a custom element registry given target.
    dom::QualifiedName name {infra::Namespaces::HTML, dom::DOMStringAtom::Null(), u8"html", TagName::html,
                             Namespace::HTML};
    auto root =
      dom::ElementFactory::Create(*document, name, dom::DOMStringAtom::Null(), false,
                                  ShareRefPtr(CustomElementAlgorithms::LookUpCustomElementRegistry(target)));

    // FPA(12): Append root to document.
    (void)dom::MutationAlgorithms::Append(*root, *document);

    // FPA(13): Set up the HTML parser's stack of open elements so that it contains just the single element
    // root.
    parser->OpenElementStack().Push(HTMLStackItem(TagName::html, Namespace::HTML, *root, {}));

    // FPA: Let fragment be the result of creating a document fragment given document if configuration is
    // non-null; otherwise target's node document.
    // NOTE: When sanitizing, the DocumentFragment is created as part of the inert document, so that elements
    // that the sanitizer removes are never created in target's node document. It is adopted into target's
    // node document after parsing completes.
    auto fragment =
      sanitizing ? document->CreateDocumentFragment() : target.NodeDocument().CreateDocumentFragment();

    // FPA: Set the parser's insertion target redirection map[root] to (fragment, null).
    parser->_treeBuilder.RedirectInsertions(*root, {.Parent = fragment.get()});

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
      attributes.emplace_back(krys::move(attribute));
    }
    parser->_fragmentParsingContext.emplace(contextTagName, contextNamespace, context,
                                            krys::move(attributes));

    // FPA(16): Reset the parser's insertion mode appropriately.
    parser->ResetInsertionModeAppropriately();

    // FPA(17): Set the HTML parser's form element pointer to the nearest node to context that is a form
    // element (going straight up the ancestor chain, and including the element itself, if it is a form
    // element), if any. (If there is no such form element, the form element pointer keeps its initial value,
    // null.)
    auto range = dom::InclusiveAncestorRange(context);
    auto form = dom::FirstOfType<HTMLFormElement>(range);
    if (form != range.end())
    {
      parser->FormElement(Downcast<HTMLFormElement>(*form));
    }

    // FPA(18): Place the input into the input stream for the HTML parser just created. The encoding
    // confidence is irrelevant.
    parser->InputStream().Append(krys::move(input), IsEOF(true));

    // FPA(19): Start the HTML parser and let it run until it has consumed all the characters just inserted
    // into the input stream.
    parser->PumpTokenizer();

    // FPA: Adopt fragment into target's node document.
    if (&fragment->NodeDocument() != &target.NodeDocument())
    {
      (void)dom::DocumentAlgorithms::AdoptNode(*fragment, target.NodeDocument());
    }

    // FPA: Return fragment.
    return fragment;
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
      _treeBuilder.ProcessToken(krys::move(tokenAtom));
    }

    return;
  }
}