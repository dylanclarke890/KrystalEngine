/*
 * Copyright (C) 2015, 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementInterface.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo
{
  class Document;
  class Node;
}

namespace krys::boo::html
{
  /// @brief The custom element definitions of a document (its global registry) or of the trees that use a
  /// scoped registry, defined from C++.
  /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#customelementregistry
  class CustomElementRegistry : public RefCounted<CustomElementRegistry>
  {
  public:
    using WhenDefinedCallback = Func<void(CustomElementInterface &)>;

  private:
    /// @brief The document whose global registry this is; null for a scoped registry.
    WeakPtr<boo::Document> _document;
    bool _isScoped {false};
    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#element-definition-is-running
    bool _elementDefinitionIsRunning {false};
    HashMap<DOMStringAtom, Ref<CustomElementInterface>> _nameMap;
    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#when-defined-promise-map
    HashMap<DOMStringAtom, List<WhenDefinedCallback>> _whenDefinedMap;
    /// @see https://html.spec.whatwg.org/#customelementregistry-scoped-document-set
    List<WeakPtr<boo::Document>> _scopedDocumentSet;

  public:
    /// @brief A document's global registry.
    explicit CustomElementRegistry(boo::Document &document) noexcept;

    /// @brief A scoped registry.
    CustomElementRegistry() noexcept;

    ~CustomElementRegistry() noexcept;

    KRYS_NODISCARD static Ref<CustomElementRegistry> Create(boo::Document &document) noexcept
    {
      return CreateRef<CustomElementRegistry>(document);
    }

    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#dom-customelementregistry
    KRYS_NODISCARD static Ref<CustomElementRegistry> CreateScoped() noexcept
    {
      return CreateRef<CustomElementRegistry>();
    }

    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#is-scoped
    KRYS_NODISCARD bool IsScoped() const noexcept
    {
      return _isScoped;
    }

    /// @brief The document whose global registry this is; null for a scoped registry.
    KRYS_NODISCARD boo::Document *Document() const noexcept;

    /// @brief Defines @p name as a custom element whose behaviour @p factory creates, and upgrades the
    /// elements already waiting for it.
    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#dom-customelementregistry-define
    ExceptionOr<void> Define(const DOMStringAtom &name, CustomElementFactory &&factory,
                             CustomElementOptions &&options = {}) noexcept;

    /// @brief The definition of @p name; null if there's none.
    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#dom-customelementregistry-get
    KRYS_NODISCARD CustomElementInterface *Get(DOMStringAtom name) const noexcept;

    /// @brief The name @p elementInterface is defined as here; null if it isn't one of this registry's.
    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#dom-customelementregistry-getname
    KRYS_NODISCARD DOMStringAtom GetName(const CustomElementInterface &elementInterface) const noexcept;

    /// @brief Calls @p callback (in a microtask) once @p name is defined, or now if it already is.
    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#dom-customelementregistry-whendefined
    ExceptionOr<void> WhenDefined(const DOMStringAtom &name, WhenDefinedCallback &&callback) noexcept;

    /// @brief Tries to upgrade @p root and its shadow-including descendants.
    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#dom-customelementregistry-upgrade
    void Upgrade(Node &root) noexcept;

    /// @brief Gives @p root and its descendants that have a null registry this one, and upgrades them.
    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#dom-customelementregistry-initialize
    ExceptionOr<void> Initialize(Node &root) noexcept;

    /// @brief Remembers that @p document has trees using this (scoped) registry, so that they upgrade when
    /// a definition is added.
    void AppendToScopedDocumentSet(boo::Document &document) noexcept;

    KRYS_NODISCARD CustomElementInterface *FindInterface(const DOMStringAtom &name) const noexcept;

    KRYS_NODISCARD CustomElementInterface *FindInterface(const QualifiedName &name) const noexcept;

  private:
    void AddElementDefinition(const Ref<CustomElementInterface> &elementInterface) noexcept;
  };
}
