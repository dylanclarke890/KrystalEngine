#include "Krystal.Booey/HTML/Algorithms/HTMLInsertionAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/HTMLTemplateElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/HTML/Sanitizer/SanitizerAlgorithms.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::html
{
  namespace
  {
    // https://html.spec.whatwg.org/#html-fragment-parsing-algorithm-steps
    KRYS_NODISCARD dom::ExceptionOr<Ref<dom::DocumentFragment>>
      RunFragmentParsingAlgorithmSteps(dom::ContainerNode &target, dom::DOMStringView markup,
                                       const SanitizerInput *sanitizer, FragmentParserMode mode) noexcept
    {
      // SPEC: If mode is Legacy and target's node document is an XML document, then return the result of
      // invoking the XML fragment parsing algorithm given target and markup.
      // NOTE: Legacy parsing doesn't come through here.

      // SPEC: Let safe be true if mode is Safe; otherwise false.
      bool safe = mode == FragmentParserMode::Safe;

      // SPEC: Let sanitizerConfig be the result of getting a sanitizer config from options given options and
      // safe.
      auto sanitizerConfig = SanitizerAlgorithms::GetSanitizerConfigFromOptions(sanitizer, safe);
      if (sanitizerConfig.HasException())
      {
        return sanitizerConfig.ReleaseException();
      }

      // SPEC: Let scriptingMode be Inert. Let runScripts be [...] options["runScripts"] with default false.
      // If runScripts is true: [...] set scriptingMode to Fragment. NOTE: There's no scripting, so no
      // runScripts.

      // SPEC: Let allowDeclarativeShadowRoots be false if mode is Legacy; otherwise true.
      // SPEC: Return the result of the HTML fragment parsing algorithm given target, markup,
      // allowDeclarativeShadowRoots, scriptingMode, and sanitizerConfig.
      return HTMLDocumentParser::ParseFragment(target, utf8_string(markup), true, ParserScriptingMode::Inert,
                                               sanitizerConfig.ReleaseValue());
    }
  }

  dom::ContainerNode &HTMLInsertionAlgorithms::TargetForHTMLInsertion(dom::Element &element) noexcept
  {
    // SPEC: If element is a template element, then return element's template contents.
    if (auto *templateElement = DynamicDowncast<HTMLTemplateElement>(element))
    {
      return templateElement->TemplateContents();
    }

    // SPEC: Return element.
    return element;
  }

  dom::ExceptionOr<dom::ContainerNode *>
    HTMLInsertionAlgorithms::ParentForHTMLInsertion(dom::Node &node) noexcept
  {
    // SPEC: Let parent be node's parent.
    auto *parent = node.ParentNode();

    // SPEC: If parent is null, then return null.
    if (!parent)
    {
      return static_cast<dom::ContainerNode *>(nullptr);
    }

    // SPEC: If parent is a template element, then throw a "HierarchyRequestError" DOMException.
    if (Is<HTMLTemplateElement>(*parent))
    {
      return dom::Exception {dom::ExceptionCode::HierarchyRequestError};
    }

    // SPEC: If parent is an Element, or a DocumentFragment with a non-null host, then return parent.
    if (Is<dom::Element>(*parent))
    {
      return parent;
    }
    if (auto *fragment = DynamicDowncast<dom::DocumentFragment>(*parent); fragment && fragment->Host())
    {
      return parent;
    }

    // SPEC: Throw a "HierarchyRequestError" DOMException.
    return dom::Exception {dom::ExceptionCode::HierarchyRequestError};
  }

  dom::ExceptionOr<void> HTMLInsertionAlgorithms::FilterAndSetHTML(dom::ContainerNode &target,
                                                                   dom::DOMStringView html,
                                                                   const SanitizerInput *sanitizer,
                                                                   FragmentParserMode mode) noexcept
  {
    // SPEC: Let fragment be the result of parsing HTML for insertion given target, html, options, and mode.
    auto fragment = ParseHTMLForInsertion(target, html, sanitizer, mode);
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }

    // SPEC: If fragment is non-null, then replace all with fragment within target.
    if (auto parsed = fragment.ReleaseValue())
    {
      return dom::MutationAlgorithms::ReplaceAll(parsed.get(), target);
    }
    return {};
  }

  dom::ExceptionOr<void> HTMLInsertionAlgorithms::FilterAndPreInsertHTML(dom::ContainerNode &parent,
                                                                         dom::Node *child,
                                                                         dom::DOMStringView html,
                                                                         const SanitizerInput *sanitizer,
                                                                         FragmentParserMode mode) noexcept
  {
    // SPEC: If parent is an html element, then throw a "HierarchyRequestError" DOMException.
    if (Is<HTMLHtmlElement>(parent))
    {
      return dom::Exception {dom::ExceptionCode::HierarchyRequestError};
    }

    // SPEC: Let fragment be the result of parsing HTML for insertion given parent, html, options, and mode.
    auto fragment = ParseHTMLForInsertion(parent, html, sanitizer, mode);
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }

    // SPEC: If fragment is non-null, then pre-insert fragment into parent before child.
    if (auto parsed = fragment.ReleaseValue())
    {
      if (auto result = dom::MutationAlgorithms::PreInsert(*parsed, parent, child); result.HasException())
      {
        return result.ReleaseException();
      }
    }
    return {};
  }

  dom::ExceptionOr<void> HTMLInsertionAlgorithms::FilterAndReplaceWithHTML(dom::ContainerNode &parent,
                                                                           dom::Node &node,
                                                                           dom::DOMStringView html,
                                                                           const SanitizerInput *sanitizer,
                                                                           FragmentParserMode mode) noexcept
  {
    // SPEC: If parent is an html element, then throw a "HierarchyRequestError" DOMException.
    if (Is<HTMLHtmlElement>(parent))
    {
      return dom::Exception {dom::ExceptionCode::HierarchyRequestError};
    }

    // SPEC: Let fragment be the result of parsing HTML for insertion given parent, html, options, and mode.
    auto fragment = ParseHTMLForInsertion(parent, html, sanitizer, mode);
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }

    // SPEC: If fragment is non-null, then replace node with fragment within parent.
    if (auto parsed = fragment.ReleaseValue())
    {
      if (auto result = dom::MutationAlgorithms::Replace(node, *parsed, parent); result.HasException())
      {
        return result.ReleaseException();
      }
    }
    return {};
  }

  dom::ExceptionOr<RefPtr<dom::DocumentFragment>>
    HTMLInsertionAlgorithms::ParseHTMLForInsertion(dom::ContainerNode &target, dom::DOMStringView html,
                                                   const SanitizerInput *sanitizer,
                                                   FragmentParserMode mode) noexcept
  {
    // SPEC: If mode is Safe:
    if (mode == FragmentParserMode::Safe)
    {
      // SPEC: Let context be target if target is an Element; otherwise target's host.
      auto *context = DynamicDowncast<dom::Element>(target);
      if (!context)
      {
        context = Downcast<dom::DocumentFragment>(target).Host();
      }

      // SPEC: Assert: context is non-null.
      krys_assert(context);

      // SPEC: If all of the following are true: context's local name is "script"; and context's namespace is
      // the HTML namespace or the SVG namespace, then return null.
      if (context->LocalName() == u8"script"
          && (context->NamespaceURI() == dom::DOMStringAtom(infra::Namespaces::HTML)
              || context->NamespaceURI() == dom::DOMStringAtom(infra::Namespaces::SVG)))
      {
        return RefPtr<dom::DocumentFragment> {};
      }
    }

    // SPEC: Return the result of invoking the fragment parsing algorithm steps given target, html, options,
    // and mode.
    auto fragment = RunFragmentParsingAlgorithmSteps(target, html, sanitizer, mode);
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }
    return RefPtr<dom::DocumentFragment> {fragment.ReleaseValue()};
  }

  dom::ExceptionOr<void> HTMLInsertionAlgorithms::BeforeHTML(dom::Node &node, dom::DOMStringView html,
                                                             const SanitizerInput *sanitizer,
                                                             FragmentParserMode mode) noexcept
  {
    // SPEC: Let parent be the parent for HTML insertion given this.
    auto parent = ParentForHTMLInsertion(node);
    if (parent.HasException())
    {
      return parent.ReleaseException();
    }

    // SPEC: If parent is null, then return.
    if (!parent.Value())
    {
      return {};
    }

    // SPEC: Filter and pre-insert HTML given parent, this, html, options, and mode.
    return FilterAndPreInsertHTML(*parent.Value(), &node, html, sanitizer, mode);
  }

  dom::ExceptionOr<void> HTMLInsertionAlgorithms::AfterHTML(dom::Node &node, dom::DOMStringView html,
                                                            const SanitizerInput *sanitizer,
                                                            FragmentParserMode mode) noexcept
  {
    // SPEC: Let parent be the parent for HTML insertion given this.
    auto parent = ParentForHTMLInsertion(node);
    if (parent.HasException())
    {
      return parent.ReleaseException();
    }

    // SPEC: If parent is null, then return.
    if (!parent.Value())
    {
      return {};
    }

    // SPEC: Filter and pre-insert HTML given parent, this's next sibling, html, options, and mode.
    return FilterAndPreInsertHTML(*parent.Value(), node.NextSibling(), html, sanitizer, mode);
  }

  dom::ExceptionOr<void> HTMLInsertionAlgorithms::ReplaceWithHTML(dom::Node &node, dom::DOMStringView html,
                                                                  const SanitizerInput *sanitizer,
                                                                  FragmentParserMode mode) noexcept
  {
    // SPEC: Let parent be the parent for HTML insertion given this.
    auto parent = ParentForHTMLInsertion(node);
    if (parent.HasException())
    {
      return parent.ReleaseException();
    }

    // SPEC: If parent is null, then return.
    if (!parent.Value())
    {
      return {};
    }

    // SPEC: Filter and replace with HTML given parent, this, html, options, and mode.
    return FilterAndReplaceWithHTML(*parent.Value(), node, html, sanitizer, mode);
  }
}
