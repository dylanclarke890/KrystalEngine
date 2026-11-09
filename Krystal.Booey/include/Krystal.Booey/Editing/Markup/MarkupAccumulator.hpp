/*
 * Copyright (C) 2004-2024 Apple Inc. All rights reserved.
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
#include "Krystal.Booey/Editing/Markup/Markup.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Attr;
  class Element;
  class Node;
  class QualifiedName;
  class ShadowRoot;
  class Text;
}

namespace krys::boo
{
  /// @brief Prefixes by namespace and namespaces by prefix (the default namespace's prefix is empty), as in
  /// scope while serializing XML.
  using Namespaces = HashMap<dom::DOMStringAtom, dom::DOMStringAtom>;

  enum class EntityMask : uint8
  {
    Amp,
    Lt,
    Gt,
    Quot,
    Nbsp,
    Tab,
    LineFeed,
    CarriageReturn,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::EntityMask, 8uz);

namespace krys::boo
{
  inline constexpr EnumSet<EntityMask> EntityMaskInCDATA {};
  inline constexpr EnumSet<EntityMask> EntityMaskInPCDATA {EntityMask::Amp, EntityMask::Lt, EntityMask::Gt};
  // Non-breaking space needs to be escaped in innerHTML for compatibility reason. See
  // http://trac.webkit.org/changeset/32879 However, we cannot do this in a XML document because it does not
  // have the entity reference defined (See the bug 19215).
  inline constexpr EnumSet<EntityMask> EntityMaskInHTMLPCDATA {EntityMask::Amp, EntityMask::Lt,
                                                               EntityMask::Gt, EntityMask::Nbsp};
  inline constexpr EnumSet<EntityMask> EntityMaskInAttributeValue {
    EntityMask::Amp,           EntityMask::Lt,  EntityMask::Gt,
    EntityMask::Quot,          EntityMask::Tab, EntityMask::LineFeed,
    EntityMask::CarriageReturn};
  inline constexpr EnumSet<EntityMask> EntityMaskInHTMLAttributeValue {
    EntityMask::Amp, EntityMask::Lt, EntityMask::Gt, EntityMask::Quot, EntityMask::Nbsp};
  inline constexpr EnumSet<EntityMask> EntityMaskInHTMLLegacyAttributeValue {
    EntityMask::Amp, EntityMask::Quot, EntityMask::Nbsp};

  /// @brief Serializes nodes as HTML or XML.
  // FIXME: Replacing URLs as a web archive is saved (enableURLReplacement and the exclusion rules), with web
  // archives.
  class MarkupAccumulator
  {
    KRYS_NON_COPYABLE(MarkupAccumulator);

  protected:
    List<Ref<dom::Node>> *const _nodes;

  private:
    utf8_string _markup;
    const ResolveURLs _resolveURLs;
    const SerializationSyntax _serializationSyntax;
    uint32 _prefixLevel {0};
    SerializeShadowRoots _serializeShadowRoots;
    List<Ref<dom::ShadowRoot>> _explicitShadowRoots;
    const RequireWellFormed _requireWellFormed;
    bool _wellFormed {true};

  public:
    MarkupAccumulator(List<Ref<dom::Node>> *nodes, ResolveURLs resolveURLs,
                      SerializationSyntax serializationSyntax,
                      SerializeShadowRoots serializeShadowRoots = SerializeShadowRoots::Explicit,
                      List<Ref<dom::ShadowRoot>> &&explicitShadowRoots = {},
                      RequireWellFormed requireWellFormed = RequireWellFormed::No) noexcept;
    virtual ~MarkupAccumulator() noexcept;

    KRYS_NODISCARD utf8_string SerializeNodes(dom::Node &targetNode, SerializedNodes root) noexcept;

    /// @brief False if the serialization required well-formed XML and met one of the error cases.
    /// @see https://html.spec.whatwg.org/#xml-fragment-serialisation-algorithm
    KRYS_NODISCARD bool IsWellFormed() const noexcept
    {
      return _wellFormed;
    }

    static void AppendCharactersReplacingEntities(utf8_string &result, utf8_stringview source,
                                                  EnumSet<EntityMask> entityMask) noexcept;

    KRYS_NODISCARD static SerializationSyntax GetSerializationSyntax(dom::Document &document) noexcept;

  protected:
    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _markup.size();
    }
    KRYS_NODISCARD bool ContainsOnlyASCII() const noexcept;

    KRYS_NODISCARD utf8_string TakeMarkup() noexcept;

    template <typename... TStrings>
    void Append(TStrings &&...strings) noexcept
    {
      (_markup.append(std::forward<TStrings>(strings)), ...);
    }

    void StartAppendingNode(const dom::Node &node, Namespaces *namespaces = nullptr) noexcept;
    void EndAppendingNode(const dom::Node &node) noexcept;

    virtual void AppendStartTag(utf8_string &result, const dom::Element &element,
                                Namespaces *namespaces) noexcept;
    virtual void AppendEndTag(utf8_string &result, const dom::Element &element) noexcept;
    virtual void AppendCustomAttributes(utf8_string &result, const dom::Element &element,
                                        Namespaces *namespaces) noexcept;
    virtual void AppendText(utf8_string &result, const dom::Text &text) noexcept;
    virtual bool AppendContentsForNode(utf8_string &result, const dom::Node &node) noexcept;

    void AppendOpenTag(utf8_string &result, const dom::Element &element, Namespaces *namespaces) noexcept;
    void AppendCloseTag(utf8_string &result, const dom::Element &element) noexcept;

    void AppendNonElementNode(utf8_string &result, const dom::Node &node, Namespaces *namespaces) noexcept;

    void AppendAttributeValue(utf8_string &result, utf8_stringview attribute) noexcept;
    bool AppendAttribute(utf8_string &result, const dom::Element &element, const dom::Attr &attribute,
                         Namespaces *namespaces) noexcept;

    KRYS_NODISCARD EnumSet<EntityMask> EntityMaskForText(const dom::Text &text) const noexcept;

  private:
    void AppendNamespace(utf8_string &result, const dom::DOMStringAtom &prefix,
                         const dom::DOMStringAtom &namespaceURI, Namespaces &namespaces,
                         bool allowEmptyDefaultNS = false) noexcept;
    KRYS_NODISCARD utf8_string ResolveURLIfNeeded(const dom::Element &element,
                                                  utf8_stringview urlString) const noexcept;
    KRYS_NODISCARD bool ShouldIncludeShadowRoots() const noexcept;
    KRYS_NODISCARD bool IncludeShadowRoot(const dom::ShadowRoot &shadowRoot) const noexcept;
    void SerializeNodesWithNamespaces(dom::Node &targetNode, SerializedNodes root,
                                      const Namespaces *namespaces) noexcept;
    KRYS_NODISCARD bool InXMLFragmentSerialization() const noexcept
    {
      return _serializationSyntax == SerializationSyntax::XML;
    }
    /// @brief Records that the serialization isn't well-formed unless @p wellFormed returns true, when it
    /// must be (only the XML serialization has error cases).
    template <Predicate TPredicate>
    void CheckWellFormed(TPredicate &&wellFormed) noexcept
    {
      if (_wellFormed && _requireWellFormed == RequireWellFormed::Yes && InXMLFragmentSerialization())
      {
        _wellFormed = wellFormed();
      }
    }
    KRYS_NODISCARD dom::DOMStringAtom GenerateUniquePrefix(const Namespaces &namespaces) noexcept;
    KRYS_NODISCARD utf8_string XMLAttributeSerialization(const dom::Attr &attribute, Namespaces *namespaces,
                                                         dom::DOMStringAtom &prefix) noexcept;
    KRYS_NODISCARD const dom::ShadowRoot *SuitableShadowRoot(const dom::Node &node) const noexcept;
  };
}
