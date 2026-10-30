#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementInterface.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Node;
}

namespace krys::boo::html
{
  /// @brief The custom element definitions of a document (its global registry) or of the trees that use a
  /// scoped registry, defined from C++.
  /// @see https://html.spec.whatwg.org/#customelementregistry
  class CustomElementRegistry : public RefCounted<CustomElementRegistry>
  {
  public:
    using WhenDefinedCallback = Func<void(CustomElementInterface &)>;

  private:
    /// @brief The document whose global registry this is; null for a scoped registry.
    WeakPtr<dom::Document> _document;
    bool _isScoped {false};
    /// @see https://html.spec.whatwg.org/#element-definition-is-running
    bool _elementDefinitionIsRunning {false};
    HashMap<dom::DOMStringAtom, Ref<CustomElementInterface>> _nameMap;
    /// @see https://html.spec.whatwg.org/#when-defined-promise-map
    HashMap<dom::DOMStringAtom, List<WhenDefinedCallback>> _whenDefinedMap;
    /// @see https://html.spec.whatwg.org/#customelementregistry-scoped-document-set
    List<WeakPtr<dom::Document>> _scopedDocumentSet;

  public:
    /// @brief A document's global registry.
    explicit CustomElementRegistry(dom::Document &document) noexcept;
    /// @brief A scoped registry.
    CustomElementRegistry() noexcept;
    ~CustomElementRegistry() noexcept;

    KRYS_NODISCARD static Ref<CustomElementRegistry> Create(dom::Document &document) noexcept
    {
      return CreateRef<CustomElementRegistry>(document);
    }

    /// @see https://html.spec.whatwg.org/#dom-customelementregistry
    KRYS_NODISCARD static Ref<CustomElementRegistry> CreateScoped() noexcept
    {
      return CreateRef<CustomElementRegistry>();
    }

    /// @see https://html.spec.whatwg.org/#is-scoped
    KRYS_NODISCARD bool IsScoped() const noexcept
    {
      return _isScoped;
    }

    /// @brief The document whose global registry this is; null for a scoped registry.
    KRYS_NODISCARD dom::Document *Document() const noexcept;

    /// @brief Defines @p name as a custom element whose behaviour @p factory creates, and upgrades the
    /// elements already waiting for it.
    /// @see https://html.spec.whatwg.org/#dom-customelementregistry-define
    dom::ExceptionOr<void> Define(dom::DOMStringAtom name, CustomElementFactory &&factory,
                                  CustomElementOptions &&options = {}) noexcept;

    /// @brief The definition of @p name; null if there's none.
    /// @see https://html.spec.whatwg.org/#dom-customelementregistry-get
    KRYS_NODISCARD CustomElementInterface *Get(dom::DOMStringAtom name) const noexcept;

    /// @brief The name @p elementInterface is defined as here; null if it isn't one of this registry's.
    /// @see https://html.spec.whatwg.org/#dom-customelementregistry-getname
    KRYS_NODISCARD dom::DOMStringAtom GetName(const CustomElementInterface &elementInterface) const noexcept;

    /// @brief Calls @p callback (in a microtask) once @p name is defined, or now if it already is.
    /// @see https://html.spec.whatwg.org/#dom-customelementregistry-whendefined
    dom::ExceptionOr<void> WhenDefined(dom::DOMStringAtom name, WhenDefinedCallback &&callback) noexcept;

    /// @brief Tries to upgrade @p root and its shadow-including descendants.
    /// @see https://html.spec.whatwg.org/#dom-customelementregistry-upgrade
    void Upgrade(dom::Node &root) noexcept;

    /// @brief Gives @p root and its descendants that have a null registry this one, and upgrades them.
    /// @see https://html.spec.whatwg.org/#dom-customelementregistry-initialize
    dom::ExceptionOr<void> Initialize(dom::Node &root) noexcept;

    /// @brief Remembers that @p document has trees using this (scoped) registry, so that they upgrade when
    /// a definition is added.
    void AppendToScopedDocumentSet(dom::Document &document) noexcept;

    KRYS_NODISCARD CustomElementInterface *FindInterface(dom::DOMStringAtom name) const noexcept;
    KRYS_NODISCARD CustomElementInterface *FindInterface(const dom::QualifiedName &name) const noexcept;

  private:
    void AddElementDefinition(Ref<CustomElementInterface> &&elementInterface) noexcept;
  };
}
