#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/NodeIterator.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"

namespace krys::boo::dom
{
  bool DocumentAlgorithms::IsXMLTypeDocument(const Document &document) noexcept
  {
    // We use flags to set the derived document type (i.e a HTMLDocument sets the 'IsHTMLDocument' flag) but
    // the base Document class also has a public constructor. In that case 'IsXMLDocument' is not set but it
    // should be treated as an 'xml' doc as per the spec. A 'html' type document will always be created via
    // the HTMLDocument class.
    return document.HasDocumentFlag(DocumentFlags::IsXMLDocument) || document._flags == DocumentFlags::None;
  }

  ExceptionOr<ElementCreationOptions>
    DocumentAlgorithms::FlattenElementCreationOptions(const ElementCreationOptionsOrString &options,
                                                      Document &document) noexcept
  {
    // 1. Let registry be the result of looking up a custom element registry given document.
    // 2. Let is be null.
    ElementCreationOptions result {.CustomElementRegistry = document.CustomElementRegistry(),
                                   .Is = DOMStringAtom::Null()};

    // 3. If options is a dictionary:
    if (std::holds_alternative<ElementCreationOptions>(options))
    {
      auto &dict = std::get<ElementCreationOptions>(options);

      // 3a. If options["is"] exists, then set is to it.
      result.Is = dict.Is;

      // 3b. If options["customElementRegistry"] exists:
      // (A null registry is taken as one not given.)
      if (dict.CustomElementRegistry != nullptr)
      {
        // 3b.i. If is is non-null, then throw a "NotSupportedError" DOMException.
        if (result.Is != DOMStringAtom::Null())
        {
          return Exception {ExceptionCode::NotSupportedError};
        }

        // 3b.ii. Set registry to options["customElementRegistry"].
        result.CustomElementRegistry = dict.CustomElementRegistry;
      }

      // 3c. If registry is non-null, registry's is scoped is false, and registry is not document's custom
      // element
      //     registry, then throw a "NotSupportedError" DOMException.
      if (result.CustomElementRegistry != nullptr && !result.CustomElementRegistry->IsScoped()
          && result.CustomElementRegistry != document.CustomElementRegistry())
      {
        return Exception {ExceptionCode::NotSupportedError};
      }
    }

    // 4. Return registry and is.
    return result;
  }

  ExceptionOr<Ref<Element>>
    DocumentAlgorithms::InternalCreateElementNS(Document &document, DOMStringAtom namespaceUri,
                                                DOMStringAtom qualifiedName,
                                                const ElementCreationOptionsOrString &options) noexcept
  {
    // 1. Let (namespace, prefix, localName) be the result of validating and extracting namespace and
    //    qualifiedName given "element".
    auto nameResult =
      NameValidation::ValidateAndExtract(namespaceUri, qualifiedName, ValidateAndExtractContext::Element);
    if (nameResult.HasException())
    {
      return nameResult.ReleaseException();
    }

    // 2. Let registry and is be the result of flattening element creation options given options and this.
    auto creationOptions = DocumentAlgorithms::FlattenElementCreationOptions(options, document);
    if (creationOptions.HasException())
    {
      return creationOptions.ReleaseException();
    }

    // 3. Return the result of creating an element given document, localName, namespace, prefix, is, true, and
    //    registry.
    auto &qName = *nameResult;
    return ElementFactory::Create(
      document, {qName.NamespaceURI(), qName.NamespacePrefix(), qName.LocalName()},
      creationOptions.Value().Is, true, creationOptions.Value().CustomElementRegistry);
  }

  ExceptionOr<void> DocumentAlgorithms::AdoptNode(Node &node, Document &document) noexcept
  {
    // 1. Let oldDocument be node's node document.
    auto &oldDocument = node.NodeDocument();

    // 2. If node's parent is non-null, then remove node.
    if (node.ParentNode() != nullptr)
    {
      if (auto result = MutationAlgorithms::Remove(node); result.HasException())
      {
        return result.ReleaseException();
      }
    }

    // 3. If document is not oldDocument, then for each inclusiveDescendant of node's shadow-including
    // inclusive
    //    descendants, in shadow-including tree order:
    // (Steps 3a to 3c run for every node first, then 3c.iii, then 3d, so the adopting steps see the whole
    // subtree moved.)
    if (&document != &oldDocument)
    {
      // The old document may have only the moving nodes to keep it.
      auto protectedOldDocument = ShareRef(oldDocument);
      auto documentEffectiveGlobalRegistry =
        ShareRefPtr(html::CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(document));

      for (auto &inclusiveDescendant : InclusiveShadowIncludingDescendantRange(node))
      {
        // 3a. Set inclusiveDescendant's node document to document.
        inclusiveDescendant.SetNodeDocument(document);

        // 3b. If inclusiveDescendant is a shadow root and if any of the following are true:
        //     - inclusiveDescendant's custom element registry is null and inclusiveDescendant's keep custom
        //       element registry null is false; or
        //     - inclusiveDescendant's custom element registry is a global custom element registry,
        //     then set inclusiveDescendant's custom element registry to document's custom element registry's
        //     effective global custom element registry.
        if (Is<ShadowRoot>(inclusiveDescendant))
        {
          auto &shadowRootInclusiveDescendant = Downcast<ShadowRoot>(inclusiveDescendant);
          auto registry = shadowRootInclusiveDescendant.CustomElementRegistry();
          if ((registry == nullptr && !shadowRootInclusiveDescendant._keepCustomElementRegistryNull)
              || html::CustomElementAlgorithms::IsGlobalCustomElementRegistry(registry.get()))
          {
            shadowRootInclusiveDescendant._customElementRegistry = documentEffectiveGlobalRegistry;
          }
        }
        // 3c. Otherwise, if inclusiveDescendant is an element:
        else if (Is<Element>(inclusiveDescendant))
        {
          auto &elementInclusiveDescendant = Downcast<Element>(inclusiveDescendant);

          // 3c.i. Set the node document of each attribute in inclusiveDescendant's attribute list to
          // document.
          for (auto &attr : elementInclusiveDescendant._attributes)
          {
            attr->SetNodeDocument(document);
          }

          // 3c.ii. If inclusiveDescendant's custom element registry is null or inclusiveDescendant's custom
          //        element registry's is scoped is false:
          auto registry = elementInclusiveDescendant.CustomElementRegistry();
          if (registry == nullptr || !registry->IsScoped())
          {
            // 3c.ii.A. Let registry be null.
            html::CustomElementRegistry *newRegistry = nullptr;

            // 3c.ii.B. If inclusiveDescendant's custom element registry is non-null, inclusiveDescendant's
            // parent
            //          is null, or inclusiveDescendant's parent is an exclusive DocumentFragment node, then
            //          set registry to document's custom element registry.
            auto *parent = elementInclusiveDescendant.ParentNode();
            if (registry != nullptr || parent == nullptr
                || TreeQueries::IsExclusiveDocumentFragmentNode(*parent))
            {
              newRegistry = document.CustomElementRegistry().get();
            }
            // 3c.ii.C. Otherwise, set registry to the result of looking up a custom element registry given
            //          inclusiveDescendant's parent.
            else
            {
              newRegistry = html::CustomElementAlgorithms::LookUpCustomElementRegistry(*parent);
            }

            // 3c.ii.D. Set inclusiveDescendant's custom element registry to registry's effective global
            // custom
            //          element registry.
            elementInclusiveDescendant._customElementRegistry =
              ShareRefPtr(html::CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(newRegistry));
          }
        }
      }

      // Node iterators whose root moved go with it.
      std::erase_if(oldDocument.NodeIterators(),
                    [&](NodeIterator *iterator)
                    {
                      if (&iterator->Root().NodeDocument() != &document)
                      {
                        return false;
                      }
                      document.NodeIterators().push_back(iterator);
                      return true;
                    });

      // 3c.iii. If inclusiveDescendant is custom, then enqueue a custom element callback reaction with
      //         inclusiveDescendant, callback name "adoptedCallback", and « oldDocument, document ».
      for (auto &inclusiveDescendant : InclusiveShadowIncludingDescendantRange(node))
      {
        if (auto *element = DynamicDowncast<Element>(inclusiveDescendant);
            element && element->IsDefinedCustomElement())
        {
          html::CustomElementReactionQueue::EnqueueAdoptedCallbackIfNeeded(*element, oldDocument, document);
        }
      }

      // 3d. Run the adopting steps with inclusiveDescendant and oldDocument.
      for (auto &inclusiveDescendant : InclusiveShadowIncludingDescendantRange(node))
      {
        ExtensibilityHooks::NodeAdopted(inclusiveDescendant, oldDocument);
      }
    }

    return {};
  }

  void DocumentAlgorithms::UpdateCurrentDocumentReadiness(Document &document,
                                                          html::DocumentReadyState readiness) noexcept
  {
    if (document._currentDocumentReadiness == readiness)
    {
      return;
    }

    document._currentDocumentReadiness = readiness;

    // If document is associated with an HTML parser (a document a frame loads, with a window):
    if (auto *window = document.Window(); window && document.Frame())
    {
      // Let now be the current high resolution time given document's relevant global object.
      auto now = window->NowTimestamp();
      // If readinessValue is "complete", and document's load timing info's DOM complete time is 0, then set
      // document's load timing info's DOM complete time to now.
      // Otherwise, if readinessValue is "interactive", and document's load timing info's DOM interactive time
      // is 0, then set document's load timing info's DOM interactive time to now.
      if (readiness == html::DocumentReadyState::Complete && !document._loadTiming.DOMCompleteTime)
      {
        document._loadTiming.DOMCompleteTime = now;
      }
      else if (readiness == html::DocumentReadyState::Interactive && !document._loadTiming.DOMInteractiveTime)
      {
        document._loadTiming.DOMInteractiveTime = now;
      }
    }

    EventDispatcher::FireEvent(EventNames::ReadyStateChange, document);
  }

  bool DocumentAlgorithms::IsIFrameSrcdocDocument(const Document &document) noexcept
  {
    // FIXME: This is a Document whose URL matches about:srcdoc.
    (void)document;
    return false;
  }
}