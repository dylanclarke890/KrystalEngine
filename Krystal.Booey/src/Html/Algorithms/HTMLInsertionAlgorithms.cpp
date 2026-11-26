#include "Krystal.Booey/Html/Algorithms/HTMLInsertionAlgorithms.hpp"
#include "Krystal.Booey/Dom/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/Dom/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/Html/Elements/HTMLHtmlElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTemplateElement.hpp"
#include "Krystal.Booey/Html/Parser/DocumentParser.hpp"
#include "Krystal.Booey/Html/Sanitizer/SanitizerAlgorithms.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::html
{
  namespace
  {
    // https://html.spec.whatwg.org/#html-fragment-parsing-algorithm-steps
    KRYS_NODISCARD ExceptionOr<Ref<DocumentFragment>>
      RunFragmentParsingAlgorithmSteps(ContainerNode &target, DOMStringView markup,
                                       const SanitizerInput *sanitizer, FragmentParserMode mode) noexcept
    {
      // 1. If mode is Legacy and target's node document is an XML document, then return the result of
      //    invoking the XML fragment parsing algorithm given target and markup.
      // (Legacy parsing doesn't come through here.)

      // 2. Let safe be true if mode is Safe; otherwise false.
      bool safe = mode == FragmentParserMode::Safe;

      // 3. Let sanitizerConfig be the result of getting a sanitizer config from options given options and
      //    safe.
      auto sanitizerConfig = SanitizerAlgorithms::GetSanitizerConfigFromOptions(sanitizer, safe);
      if (sanitizerConfig.HasException())
      {
        return sanitizerConfig.ReleaseException();
      }

      // 4. Let scriptingMode be Inert.
      // 5. Let runScripts be options's runScripts if options is a TrustedHTMLParserOptions; otherwise
      //    options["runScripts"] with default false.
      // 6. If runScripts is true: assert mode is not Safe, and set scriptingMode to Fragment.
      // (There's no scripting, so no runScripts.)

      // 7. Let allowDeclarativeShadowRoots be false if mode is Legacy; otherwise true.
      // 8. Return the result of the HTML fragment parsing algorithm given target, markup,
      //    allowDeclarativeShadowRoots, scriptingMode, and sanitizerConfig.
      return DocumentParser::ParseFragment(target, utf8_string(markup), true, ParserScriptingMode::Inert,
                                           sanitizerConfig.ReleaseValue());
    }
  }

  ContainerNode &HTMLInsertionAlgorithms::TargetForHTMLInsertion(Element &element) noexcept
  {
    // 1. If element is a template element, then return element's template contents.
    if (auto *templateElement = DynamicDowncast<HTMLTemplateElement>(element))
    {
      return templateElement->TemplateContents();
    }

    // 2. Return element.
    return element;
  }

  ExceptionOr<ContainerNode *> HTMLInsertionAlgorithms::ParentForHTMLInsertion(Node &node) noexcept
  {
    // 1. Let parent be node's parent.
    auto *parent = node.ParentNode();

    // 2. If parent is null, then return null.
    if (!parent)
    {
      return static_cast<ContainerNode *>(nullptr);
    }

    // 3. If parent is a template element, then throw a "HierarchyRequestError" DOMException.
    if (Is<HTMLTemplateElement>(*parent))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    // 4. If parent is an Element, or a DocumentFragment with a non-null host, then return parent.
    if (Is<Element>(*parent))
    {
      return parent;
    }
    if (auto *fragment = DynamicDowncast<DocumentFragment>(*parent); fragment && fragment->Host())
    {
      return parent;
    }

    // 5. Throw a "HierarchyRequestError" DOMException.
    return Exception {ExceptionCode::HierarchyRequestError};
  }

  ExceptionOr<void> HTMLInsertionAlgorithms::FilterAndSetHTML(ContainerNode &target, DOMStringView html,
                                                              const SanitizerInput *sanitizer,
                                                              FragmentParserMode mode) noexcept
  {
    // 1. Let fragment be the result of parsing HTML for insertion given target, html, options, and mode.
    auto fragment = ParseHTMLForInsertion(target, html, sanitizer, mode);
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }

    // 2. If fragment is non-null, then replace all with fragment within target.
    if (auto parsed = fragment.ReleaseValue())
    {
      return dom::MutationAlgorithms::ReplaceAll(parsed.get(), target);
    }
    return {};
  }

  ExceptionOr<void> HTMLInsertionAlgorithms::FilterAndPreInsertHTML(ContainerNode &parent, Node *child,
                                                                    DOMStringView html,
                                                                    const SanitizerInput *sanitizer,
                                                                    FragmentParserMode mode) noexcept
  {
    // 1. If parent is an html element, then throw a "HierarchyRequestError" DOMException.
    if (Is<HTMLHtmlElement>(parent))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    // 2. Let fragment be the result of parsing HTML for insertion given parent, html, options, and mode.
    auto fragment = ParseHTMLForInsertion(parent, html, sanitizer, mode);
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }

    // 3. If fragment is non-null, then pre-insert fragment into parent before child.
    if (auto parsed = fragment.ReleaseValue())
    {
      if (auto result = dom::MutationAlgorithms::PreInsert(*parsed, parent, child); result.HasException())
      {
        return result.ReleaseException();
      }
    }
    return {};
  }

  ExceptionOr<void> HTMLInsertionAlgorithms::FilterAndReplaceWithHTML(ContainerNode &parent, Node &node,
                                                                      DOMStringView html,
                                                                      const SanitizerInput *sanitizer,
                                                                      FragmentParserMode mode) noexcept
  {
    // 1. If parent is an html element, then throw a "HierarchyRequestError" DOMException.
    if (Is<HTMLHtmlElement>(parent))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    // 2. Let fragment be the result of parsing HTML for insertion given parent, html, options, and mode.
    auto fragment = ParseHTMLForInsertion(parent, html, sanitizer, mode);
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }

    // 3. If fragment is non-null, then replace node with fragment within parent.
    if (auto parsed = fragment.ReleaseValue())
    {
      if (auto result = dom::MutationAlgorithms::Replace(node, *parsed, parent); result.HasException())
      {
        return result.ReleaseException();
      }
    }
    return {};
  }

  ExceptionOr<RefPtr<DocumentFragment>>
    HTMLInsertionAlgorithms::ParseHTMLForInsertion(ContainerNode &target, DOMStringView html,
                                                   const SanitizerInput *sanitizer,
                                                   FragmentParserMode mode) noexcept
  {
    // 1. If mode is Safe:
    if (mode == FragmentParserMode::Safe)
    {
      // 1a. Let context be target if target is an Element; otherwise target's host.
      auto *context = DynamicDowncast<Element>(target);
      if (!context)
      {
        context = Downcast<DocumentFragment>(target).Host();
      }

      // 1b. Assert: context is non-null.
      krys_assert(context);

      // 1c. If all of the following are true: context's local name is "script"; and context's namespace is
      //     the HTML namespace or the SVG namespace, then return null.
      if (context->LocalName() == u8"script"
          && (context->NamespaceURI() == infra::namespaces::HTML
              || context->NamespaceURI() == infra::namespaces::SVG))
      {
        return RefPtr<DocumentFragment> {};
      }
    }

    // 2. Return the result of invoking the fragment parsing algorithm steps given target, html, options,
    //    and mode.
    auto fragment = RunFragmentParsingAlgorithmSteps(target, html, sanitizer, mode);
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }
    return RefPtr<DocumentFragment> {fragment.ReleaseValue()};
  }

  ExceptionOr<void> HTMLInsertionAlgorithms::BeforeHTML(Node &node, DOMStringView html,
                                                        const SanitizerInput *sanitizer,
                                                        FragmentParserMode mode) noexcept
  {
    // 1. Let parent be the parent for HTML insertion given this.
    auto parent = ParentForHTMLInsertion(node);
    if (parent.HasException())
    {
      return parent.ReleaseException();
    }

    // 2. If parent is null, then return.
    if (!parent.Value())
    {
      return {};
    }

    // 3. Filter and pre-insert HTML given parent, this, html, options, and mode.
    return FilterAndPreInsertHTML(*parent.Value(), &node, html, sanitizer, mode);
  }

  ExceptionOr<void> HTMLInsertionAlgorithms::AfterHTML(Node &node, DOMStringView html,
                                                       const SanitizerInput *sanitizer,
                                                       FragmentParserMode mode) noexcept
  {
    // 1. Let parent be the parent for HTML insertion given this.
    auto parent = ParentForHTMLInsertion(node);
    if (parent.HasException())
    {
      return parent.ReleaseException();
    }

    // 2. If parent is null, then return.
    if (!parent.Value())
    {
      return {};
    }

    // 3. Filter and pre-insert HTML given parent, this's next sibling, html, options, and mode.
    return FilterAndPreInsertHTML(*parent.Value(), node.NextSibling(), html, sanitizer, mode);
  }

  ExceptionOr<void> HTMLInsertionAlgorithms::ReplaceWithHTML(Node &node, DOMStringView html,
                                                             const SanitizerInput *sanitizer,
                                                             FragmentParserMode mode) noexcept
  {
    // 1. Let parent be the parent for HTML insertion given this.
    auto parent = ParentForHTMLInsertion(node);
    if (parent.HasException())
    {
      return parent.ReleaseException();
    }

    // 2. If parent is null, then return.
    if (!parent.Value())
    {
      return {};
    }

    // 3. Filter and replace with HTML given parent, this, html, options, and mode.
    return FilterAndReplaceWithHTML(*parent.Value(), node, html, sanitizer, mode);
  }
}
