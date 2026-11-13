/*
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2009-2022 Google Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Editing/Markup/MarkupAccumulator.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/CDATASection.hpp"
#include "Krystal.Booey/DOM/Nodes/Comment.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentType.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTemplateElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include <algorithm>
#include <string>

namespace krys::boo
{
  using dom::TreeQueries;

  namespace
  {
    const dom::DOMStringAtom &EmptyAtom() noexcept
    {
      static const dom::DOMStringAtom atom(u8"");
      return atom;
    }

    const dom::DOMStringAtom &XMLNSAtom() noexcept
    {
      static const dom::DOMStringAtom atom(infra::NamespacePrefixes::XMLNS);
      return atom;
    }

    const dom::DOMStringAtom &XMLAtom() noexcept
    {
      static const dom::DOMStringAtom atom(infra::NamespacePrefixes::XML);
      return atom;
    }

    const dom::DOMStringAtom &IsAtom() noexcept
    {
      static const dom::DOMStringAtom atom(u8"is");
      return atom;
    }

    KRYS_NODISCARD bool ElementCannotHaveEndTag(const dom::Node &node) noexcept
    {
      auto *element = DynamicDowncast<html::HTMLElement>(node);
      if (!element)
      {
        return false;
      }

      switch (element->TagQualifiedName().TagName())
      {
        // https://html.spec.whatwg.org/#void-elements
        case html::TagName::area:
        case html::TagName::base:
        case html::TagName::br:
        case html::TagName::col:
        case html::TagName::embed:
        case html::TagName::hr:
        case html::TagName::img:
        case html::TagName::input:
        case html::TagName::link:
        case html::TagName::meta:
        case html::TagName::source:
        case html::TagName::track:
        case html::TagName::wbr:
        // https://html.spec.whatwg.org/#serializes-as-void
        case html::TagName::basefont:
        case html::TagName::bgsound:
        case html::TagName::frame:
        case html::TagName::keygen:
        case html::TagName::param:    return true;
        default:                      break;
      }

      return false;
    }

    // Rules of self-closure
    // 1. No elements in HTML documents use the self-closing syntax.
    // 2. Elements w/ children never self-close because they use a separate end tag.
    // 3. HTML elements which do not have a "forbidden" end tag will close with a separate end tag.
    // 4. Other elements self-close.
    KRYS_NODISCARD bool ShouldSelfClose(const dom::Element &element, SerializationSyntax syntax) noexcept
    {
      if (syntax != SerializationSyntax::XML)
      {
        return false;
      }
      if (element.HasChildNodes())
      {
        return false;
      }
      if (Is<html::HTMLElement>(element) && !ElementCannotHaveEndTag(element))
      {
        return false;
      }
      return true;
    }

    // https://html.spec.whatwg.org/#html-fragment-serialisation-algorithm
    KRYS_NODISCARD utf8_string TagQName(const dom::Element &element, bool inXMLFragmentSerialization) noexcept
    {
      // If current node is an element in the HTML namespace, the MathML namespace, or the SVG namespace, then
      // let tagname be current node's local name. Otherwise, let tagname be current node's qualified name.
      auto namespaceURI = element.NamespaceURI().View();
      if (!inXMLFragmentSerialization
          && (namespaceURI == infra::Namespaces::HTML || namespaceURI == infra::Namespaces::MathML
              || namespaceURI == infra::Namespaces::SVG))
      {
        return utf8_string(element.LocalName().View());
      }
      return element.TagQualifiedName().Name();
    }

    // https://html.spec.whatwg.org/#attribute's-serialised-name
    KRYS_NODISCARD utf8_string HTMLAttributeSerialization(const dom::Attr &attribute) noexcept
    {
      auto namespaceURI = attribute.NamespaceURI().View();
      if (namespaceURI.empty())
      {
        return utf8_string(attribute.LocalName().View());
      }

      utf8_stringview prefix;
      if (namespaceURI == infra::Namespaces::XML)
      {
        prefix = infra::NamespacePrefixes::XML;
      }
      else if (namespaceURI == infra::Namespaces::XMLNS)
      {
        if (attribute.LocalName() == XMLNSAtom())
        {
          return utf8_string(infra::NamespacePrefixes::XMLNS);
        }
        prefix = infra::NamespacePrefixes::XMLNS;
      }
      else if (namespaceURI == infra::Namespaces::XLink)
      {
        prefix = infra::NamespacePrefixes::XLink;
      }
      else
      {
        return attribute.Name();
      }

      utf8_string result(prefix);
      result.push_back(u8':');
      result.append(attribute.LocalName().View());
      return result;
    }

    KRYS_NODISCARD bool ShouldAddNamespaceElement(const dom::Element &element) noexcept
    {
      // Don't add namespace attribute if it is already defined for this elem.
      auto prefix = element.Prefix().View();
      if (prefix.empty())
      {
        return !element.HasAttribute(XMLNSAtom());
      }
      utf8_string attributeName(u8"xmlns:");
      attributeName.append(prefix);
      return !element.HasAttribute(dom::DOMStringAtom(attributeName));
    }

    KRYS_NODISCARD bool ShouldAddNamespaceAttribute(const dom::Attr &attribute,
                                                    Namespaces &namespaces) noexcept
    {
      // Don't add namespace attributes twice
      // HTML Parser will create xmlns attributes without namespace for HTML elements, allow those as well.
      auto namespaceURI = attribute.NamespaceURI().View();
      if (attribute.LocalName() == XMLNSAtom()
          && (namespaceURI.empty() || namespaceURI == infra::Namespaces::XMLNS))
      {
        namespaces.insert_or_assign(EmptyAtom(), dom::DOMStringAtom(attribute.Value()));
        return false;
      }

      if (attribute.Prefix() == XMLNSAtom() && namespaceURI == infra::Namespaces::XMLNS)
      {
        namespaces.insert_or_assign(attribute.LocalName(), dom::DOMStringAtom(attribute.Value()));
        namespaces.insert_or_assign(dom::DOMStringAtom(attribute.Value()), attribute.LocalName());
        return false;
      }

      return true;
    }

    // There's no scripting, so <noscript>'s text is escaped like any other.
    KRYS_NODISCARD bool IsScriptEnabled(const dom::Node &) noexcept
    {
      return false;
    }

    void AppendXMLDeclaration(utf8_string &result, const dom::Document &document) noexcept
    {
      if (!document.HasXMLDeclaration())
      {
        return;
      }

      const auto &encoding = document.XMLEncoding();
      bool isStandaloneSpecified =
        document.XMLStandaloneStatus() != dom::Document::StandaloneStatus::Unspecified;

      result.append(u8"<?xml version=\"");
      result.append(document.XMLVersion());
      if (!encoding.empty())
      {
        result.append(u8"\" encoding=\"");
        result.append(encoding);
      }
      if (isStandaloneSpecified)
      {
        result.append(document.XMLStandalone() ? u8"\" standalone=\"yes" : u8"\" standalone=\"no");
      }
      result.append(u8"\"?>");
    }

    /// @brief Whether @p data has only characters the XML `Char` production matches.
    KRYS_NODISCARD constexpr bool IsXMLCharacterData(utf8_stringview data) noexcept
    {
      for (size_t i = 0; i < data.size(); ++i)
      {
        auto byte = static_cast<uint8>(data[i]);
        if (byte < 0x20 && byte != u8'\t' && byte != u8'\n' && byte != u8'\r')
        {
          return false;
        }
        // U+FFFE and U+FFFF (EF BF BE and EF BF BF). Strings hold no surrogates.
        if (byte == 0xEF && i + 2 < data.size() && static_cast<uint8>(data[i + 1]) == 0xBF
            && (static_cast<uint8>(data[i + 2]) & 0xFE) == 0xBE)
        {
          return false;
        }
      }
      return true;
    }

    /// @brief Whether @p id has only characters the XML `PubidChar` production matches.
    KRYS_NODISCARD constexpr bool IsXMLPublicIdentifier(utf8_stringview id) noexcept
    {
      for (auto c : id)
      {
        if (!text::IsASCIIAlphanumeric(c)
            && utf8_stringview(u8" \r\n-'()+,./:=?;!*#@$_%").find(c) == utf8_stringview::npos)
        {
          return false;
        }
      }
      return true;
    }

    /// @brief Whether a node's @p localName can be serialized: an XML `Name` without a colon.
    KRYS_NODISCARD bool IsWellFormedLocalName(utf8_stringview localName) noexcept
    {
      return localName.find(u8':') == utf8_stringview::npos && dom::NameValidation::IsValidXMLName(localName);
    }

    void AppendDocumentType(utf8_string &result, const dom::DocumentType &documentType) noexcept
    {
      if (documentType.Name().empty())
      {
        return;
      }

      const auto &publicId = documentType.PublicId();
      const auto &systemId = documentType.SystemId();
      result.append(u8"<!DOCTYPE ");
      result.append(documentType.Name());
      if (!publicId.empty())
      {
        result.append(u8" PUBLIC \"");
        result.append(publicId);
        result.append(u8"\"");
      }
      if (!systemId.empty())
      {
        result.append(publicId.empty() ? u8" SYSTEM \"" : u8" \"");
        result.append(systemId);
        result.append(u8"\">");
      }
      else
      {
        result.append(u8">");
      }
    }
  }

  void MarkupAccumulator::AppendCharactersReplacingEntities(utf8_string &result, utf8_stringview source,
                                                            EnumSet<EntityMask> entityMask) noexcept
  {
    if (source.empty())
    {
      return;
    }

    size_t positionAfterLastEntity = 0;
    for (size_t i = 0; i < source.size(); ++i)
    {
      auto character = static_cast<uint8>(source[i]);
      utf8_stringview replacement;
      size_t length = 1;
      switch (character)
      {
        case '&':  replacement = entityMask.Contains(EntityMask::Amp) ? u8"&amp;" : u8""; break;
        case '<':  replacement = entityMask.Contains(EntityMask::Lt) ? u8"&lt;" : u8""; break;
        case '>':  replacement = entityMask.Contains(EntityMask::Gt) ? u8"&gt;" : u8""; break;
        case '"':  replacement = entityMask.Contains(EntityMask::Quot) ? u8"&quot;" : u8""; break;
        case '\t': replacement = entityMask.Contains(EntityMask::Tab) ? u8"&#9;" : u8""; break;
        case '\n': replacement = entityMask.Contains(EntityMask::LineFeed) ? u8"&#10;" : u8""; break;
        case '\r': replacement = entityMask.Contains(EntityMask::CarriageReturn) ? u8"&#13;" : u8""; break;
        case 0xC2:
          // U+00A0 NO-BREAK SPACE, which is two bytes in UTF-8.
          if (i + 1 < source.size() && static_cast<uint8>(source[i + 1]) == 0xA0
              && entityMask.Contains(EntityMask::Nbsp))
          {
            replacement = u8"&nbsp;";
            length = 2;
          }
          break;
        default: break;
      }
      if (!replacement.empty()) KRYS_UNLIKELY
      {
        result.append(source.substr(positionAfterLastEntity, i - positionAfterLastEntity));
        result.append(replacement);
        positionAfterLastEntity = i + length;
        i += length - 1;
      }
    }
    result.append(source.substr(positionAfterLastEntity));
  }

  MarkupAccumulator::MarkupAccumulator(List<Ref<dom::Node>> *nodes, ResolveURLs resolveURLs,
                                       SerializationSyntax serializationSyntax,
                                       SerializeShadowRoots serializeShadowRoots,
                                       List<Ref<dom::ShadowRoot>> &&explicitShadowRoots,
                                       RequireWellFormed requireWellFormed) noexcept
      : _nodes(nodes), _resolveURLs(resolveURLs), _serializationSyntax(serializationSyntax),
        _serializeShadowRoots(serializeShadowRoots), _explicitShadowRoots(krys::move(explicitShadowRoots)),
        _requireWellFormed(requireWellFormed)
  {
    krys_debug_assert(serializeShadowRoots != SerializeShadowRoots::AllForInterchange
                      || _explicitShadowRoots.empty());
  }

  MarkupAccumulator::~MarkupAccumulator() noexcept = default;

  utf8_string MarkupAccumulator::SerializeNodes(dom::Node &targetNode, SerializedNodes root) noexcept
  {
    SerializeNodesWithNamespaces(targetNode, root, nullptr);
    return _markup;
  }

  // FIXME: Writing a style sheet's text with its URLs replaced, with web archives.
  bool MarkupAccumulator::AppendContentsForNode(utf8_string &, const dom::Node &) noexcept
  {
    return false;
  }

  bool MarkupAccumulator::ShouldIncludeShadowRoots() const noexcept
  {
    return _serializeShadowRoots != SerializeShadowRoots::Explicit || !_explicitShadowRoots.empty();
  }

  bool MarkupAccumulator::IncludeShadowRoot(const dom::ShadowRoot &shadowRoot) const noexcept
  {
    if (shadowRoot.Mode() == dom::ShadowRootMode::UserAgent)
    {
      return false;
    }

    if (_serializeShadowRoots == SerializeShadowRoots::AllForInterchange)
    {
      return true;
    }

    if (_serializeShadowRoots == SerializeShadowRoots::Serializable && shadowRoot.Serialisable())
    {
      return true;
    }

    return std::ranges::any_of(_explicitShadowRoots,
                               [&shadowRoot](const auto &item) { return item.get() == &shadowRoot; });
  }

  void MarkupAccumulator::SerializeNodesWithNamespaces(dom::Node &targetNode, SerializedNodes root,
                                                       const Namespaces *namespaces) noexcept
  {
    List<Namespaces> namespaceStack;
    if (namespaces)
    {
      namespaceStack.push_back(*namespaces);
    }
    else if (InXMLFragmentSerialization())
    {
      // Make sure xml prefix and namespace are always known to uphold the constraints listed at
      // http://www.w3.org/TR/xml-names11/#xmlReserved.
      Namespaces namespaceHash;
      namespaceHash.insert_or_assign(XMLAtom(), dom::DOMStringAtom(infra::Namespaces::XML));
      namespaceHash.insert_or_assign(dom::DOMStringAtom(infra::Namespaces::XML), XMLAtom());

      // An element's children are serialized in the default namespace in scope on it, so that children in no
      // namespace declare the empty default namespace.
      if (auto *element = DynamicDowncast<dom::Element>(targetNode);
          element && root != SerializedNodes::SubtreeIncludingNode)
      {
        auto defaultNamespace = element->LookupNamespaceURI(dom::DOMStringAtom::Null());
        if (!defaultNamespace.View().empty())
        {
          namespaceHash.insert_or_assign(EmptyAtom(), defaultNamespace);
        }
      }

      namespaceStack.push_back(krys::move(namespaceHash));
    }
    else
    {
      namespaceStack.emplace_back();
    }

    // The templates whose contents are being serialized, innermost last, so the walk back up leaves a
    // template's contents for the template.
    List<const html::HTMLTemplateElement *> templates;

    auto firstChild = [&](const dom::Node &current) -> dom::Node *
    {
      bool shouldSkipChidren = AppendContentsForNode(_markup, current);
      if (shouldSkipChidren)
      {
        return nullptr;
      }
      if (auto *currentTemplate = DynamicDowncast<html::HTMLTemplateElement>(current))
      {
        auto content = currentTemplate->Content();
        if (auto *child = content ? content->FirstChild() : nullptr)
        {
          templates.push_back(currentTemplate);
          return child;
        }
        return nullptr;
      }
      return current.FirstChild();
    };

    auto shouldEmitCloseTagFor = [&](const dom::Node &node)
    {
      return _serializationSyntax == SerializationSyntax::XML
             || !(Is<dom::HTMLDocument>(targetNode.NodeDocument()) && ElementCannotHaveEndTag(node));
    };

    RefPtr<const dom::Node> current = ShareRefPtr(&targetNode);
    do
    {
      bool shouldSkipNode = false;

      bool shouldAppendNode =
        !shouldSkipNode && !(current == &targetNode && root != SerializedNodes::SubtreeIncludingNode);
      if (shouldAppendNode)
      {
        StartAppendingNode(*current, &namespaceStack.back());
      }

      bool shouldEmitCloseTag = shouldEmitCloseTagFor(*current);
      shouldSkipNode = shouldSkipNode || !shouldEmitCloseTag;
      if (!shouldSkipNode)
      {
        if (ShouldIncludeShadowRoots())
        {
          auto *element = DynamicDowncast<dom::Element>(*current);
          auto *shadowRoot = element ? TreeQueries::GetShadowRoot(*element) : nullptr;
          if (shadowRoot && IncludeShadowRoot(*shadowRoot))
          {
            current = ShareRefPtr<const dom::Node>(shadowRoot);
            namespaceStack.push_back(namespaceStack.back());
            continue;
          }
        }

        if (auto *child = firstChild(*current))
        {
          current = ShareRefPtr<const dom::Node>(child);
          namespaceStack.push_back(namespaceStack.back());
          continue;
        }
      }

      if (shouldAppendNode && shouldEmitCloseTag)
      {
        EndAppendingNode(*current);
      }

      while (current != &targetNode)
      {
        if (auto *nextSibling = current->NextSibling())
        {
          current = ShareRefPtr<const dom::Node>(nextSibling);
          namespaceStack.pop_back();
          namespaceStack.push_back(namespaceStack.back());
          break;
        }

        if (ShouldIncludeShadowRoots() && Is<dom::ShadowRoot>(*current))
        {
          current = ShareRefPtr<const dom::Node>(Downcast<dom::ShadowRoot>(*current).Host());
          if (_serializeShadowRoots != SerializeShadowRoots::AllForInterchange)
          {
            if (auto *child = firstChild(*current))
            {
              current = ShareRefPtr<const dom::Node>(child);
              namespaceStack.push_back(namespaceStack.back());
              break;
            }
          }
        }
        else
        {
          current = ShareRefPtr<const dom::Node>(current->ParentNode());
        }

        namespaceStack.pop_back();
        if (!templates.empty() && Is<dom::DocumentFragment>(current.get())
            && templates.back()->Content().get() == current.get())
        {
          if (current != &targetNode)
          {
            current = ShareRefPtr<const dom::Node>(templates.back());
          }
          templates.pop_back();
        }

        krys_debug_assert(current);
        if (!current)
        {
          break;
        }

        shouldAppendNode = !(current == &targetNode && root != SerializedNodes::SubtreeIncludingNode);
        shouldEmitCloseTag = shouldEmitCloseTagFor(*current);
        if (shouldAppendNode && shouldEmitCloseTag)
        {
          EndAppendingNode(*current);
        }
      }
    } while (current != &targetNode);
  }

  // FIXME: Resolving URL attributes against the document's base URL (ResolveURLs::Yes and its privacy
  // variants), with URL attributes.
  utf8_string MarkupAccumulator::ResolveURLIfNeeded(const dom::Element &,
                                                    utf8_stringview urlString) const noexcept
  {
    (void)_resolveURLs;
    return utf8_string(urlString);
  }

  const dom::ShadowRoot *MarkupAccumulator::SuitableShadowRoot(const dom::Node &node) const noexcept
  {
    if (!ShouldIncludeShadowRoots())
    {
      return nullptr;
    }

    auto *shadowRoot = DynamicDowncast<dom::ShadowRoot>(node);
    if (!shadowRoot || !IncludeShadowRoot(*shadowRoot))
    {
      return nullptr;
    }
    return shadowRoot;
  }

  // FIXME: The shadow root's custom element registry (shadowrootcustomelementregistry), with custom element
  // registries.
  void MarkupAccumulator::StartAppendingNode(const dom::Node &node, Namespaces *namespaces) noexcept
  {
    if (auto *element = DynamicDowncast<dom::Element>(node))
    {
      AppendStartTag(_markup, *element, namespaces);
    }
    else if (auto *shadowRoot = SuitableShadowRoot(node))
    {
      _markup.append(u8"<template shadowrootmode=\"");
      switch (shadowRoot->Mode())
      {
        case dom::ShadowRootMode::Open:      _markup.append(u8"open"); break;
        case dom::ShadowRootMode::Closed:    _markup.append(u8"closed"); break;
        case dom::ShadowRootMode::UserAgent: krys_unreachable();
      }
      _markup.push_back(u8'"');
      if (shadowRoot->DelegatesFocus())
      {
        _markup.append(u8" shadowrootdelegatesfocus=\"\"");
      }
      if (shadowRoot->Serialisable())
      {
        _markup.append(u8" shadowrootserializable=\"\"");
      }
      if (shadowRoot->SlotAssignment() == dom::SlotAssignmentMode::Manual)
      {
        _markup.append(u8" shadowrootslotassignment=\"manual\"");
      }
      if (shadowRoot->Clonable())
      {
        _markup.append(u8" shadowrootclonable=\"\"");
      }
      // Escaped as attribute values are, which the specification's steps leave out.
      if (auto &referenceTarget = shadowRoot->ReferenceTarget())
      {
        _markup.append(u8" shadowrootreferencetarget=\"");
        AppendAttributeValue(_markup, *referenceTarget);
        _markup.push_back(u8'"');
      }
      _markup.push_back(u8'>');
    }
    else
    {
      AppendNonElementNode(_markup, node, namespaces);
    }

    if (_nodes)
    {
      _nodes->push_back(ShareRef(const_cast<dom::Node &>(node)));
    }
  }

  void MarkupAccumulator::EndAppendingNode(const dom::Node &node) noexcept
  {
    if (auto *element = DynamicDowncast<dom::Element>(node))
    {
      AppendEndTag(_markup, *element);
    }
    else if (SuitableShadowRoot(node))
    {
      _markup.append(u8"</template>");
    }
  }

  void MarkupAccumulator::AppendEndTag(utf8_string &result, const dom::Element &element) noexcept
  {
    if (ShouldSelfClose(element, _serializationSyntax)
        || (!element.HasChildNodes() && ElementCannotHaveEndTag(element)))
    {
      return;
    }
    result.append(u8"</");
    result.append(TagQName(element, InXMLFragmentSerialization()));
    result.push_back(u8'>');
  }

  utf8_string MarkupAccumulator::TakeMarkup() noexcept
  {
    return std::exchange(_markup, {});
  }

  bool MarkupAccumulator::ContainsOnlyASCII() const noexcept
  {
    return std::ranges::all_of(_markup, [](char8_t c) { return static_cast<uint8>(c) < 0x80; });
  }

  void MarkupAccumulator::AppendAttributeValue(utf8_string &result, utf8_stringview attribute) noexcept
  {
    auto entityMask = [&]() -> EnumSet<EntityMask>
    {
      switch (_serializationSyntax)
      {
        case SerializationSyntax::XML:                      return EntityMaskInAttributeValue;
        case SerializationSyntax::HTML:                     return EntityMaskInHTMLAttributeValue;
        case SerializationSyntax::HTMLLegacyAttributeValue: return EntityMaskInHTMLLegacyAttributeValue;
      }
      krys_debug_assert(false);
      return EntityMaskInAttributeValue;
    }();
    AppendCharactersReplacingEntities(result, attribute, entityMask);
  }

  void MarkupAccumulator::AppendCustomAttributes(utf8_string &, const dom::Element &, Namespaces *) noexcept
  {
  }

  void MarkupAccumulator::AppendNamespace(utf8_string &result, const dom::DOMStringAtom &prefix,
                                          const dom::DOMStringAtom &namespaceURI, Namespaces &namespaces,
                                          bool allowEmptyDefaultNS) noexcept
  {
    if (namespaceURI.View().empty())
    {
      // http://www.whatwg.org/specs/web-apps/current-work/multipage/the-xhtml-syntax.html#xml-fragment-serialization-algorithm
      if (allowEmptyDefaultNS && namespaces.contains(EmptyAtom())
          && !namespaces.at(EmptyAtom()).View().empty())
      {
        result.append(u8" xmlns=\"\"");
      }
      return;
    }

    auto [iterator, isNewEntry] = namespaces.try_emplace(prefix, namespaceURI);
    if (!isNewEntry)
    {
      if (iterator->second == namespaceURI)
      {
        return;
      }
      iterator->second = namespaceURI;
    }

    // Add namespace to prefix pair so we can do constraint checking later.
    if (InXMLFragmentSerialization() && !prefix.View().empty())
    {
      namespaces.insert_or_assign(namespaceURI, prefix);
    }

    // Make sure xml prefix and namespace are always known to uphold the constraints listed at
    // http://www.w3.org/TR/xml-names11/#xmlReserved.
    if (namespaceURI.View() == infra::Namespaces::XML)
    {
      return;
    }

    result.append(u8" xmlns");
    if (!prefix.View().empty())
    {
      result.push_back(u8':');
      result.append(prefix.View());
    }
    result.append(u8"=\"");
    AppendAttributeValue(result, namespaceURI.View());
    result.push_back(u8'"');
  }

  EnumSet<EntityMask> MarkupAccumulator::EntityMaskForText(const dom::Text &text) const noexcept
  {
    if (InXMLFragmentSerialization())
    {
      return EntityMaskInPCDATA;
    }

    if (auto *element = DynamicDowncast<html::HTMLElement>(text.ParentElement()))
    {
      switch (element->TagQualifiedName().TagName())
      {
        case html::TagName::noscript:
          if (!IsScriptEnabled(*element))
          {
            break;
          }
          KRYS_FALLTHROUGH;
        case html::TagName::iframe:
        case html::TagName::noembed:
        case html::TagName::noframes:
        case html::TagName::plaintext:
        case html::TagName::script:
        case html::TagName::style:
        case html::TagName::xmp:       return EntityMaskInCDATA;
        default:                       break;
      }
    }
    return EntityMaskInHTMLPCDATA;
  }

  void MarkupAccumulator::AppendText(utf8_string &result, const dom::Text &text) noexcept
  {
    AppendCharactersReplacingEntities(result, text.Data(), EntityMaskForText(text));
  }

  void MarkupAccumulator::AppendStartTag(utf8_string &result, const dom::Element &element,
                                         Namespaces *namespaces) noexcept
  {
    AppendOpenTag(result, element, namespaces);

    for (const auto &attribute : element.AttributeList())
    {
      (void)AppendAttribute(result, element, *attribute, namespaces);
    }

    // If current node's is value is not null, and the element does not have an is content attribute in its
    // attribute list, then append the string " is="", followed by current node's is value escaped as
    // described below in attribute mode, followed by a U+0022 QUOTATION MARK character (").
    if (!InXMLFragmentSerialization() && element.IsValue() != dom::DOMStringAtom::Null()
        && !element.HasAttribute(IsAtom()))
    {
      result.append(u8" is=\"");
      AppendAttributeValue(result, element.IsValue().View());
      result.push_back(u8'"');
    }

    // Give an opportunity to subclasses to add their own attributes.
    AppendCustomAttributes(result, element, namespaces);

    AppendCloseTag(result, element);
  }

  void MarkupAccumulator::AppendOpenTag(utf8_string &result, const dom::Element &element,
                                        Namespaces *namespaces) noexcept
  {
    // Error cases: a node with a local name containing a U+003A COLON (:), or that does not match the XML
    // Name production. (An element can't have two attributes with the same local name and namespace.)
    CheckWellFormed([&] { return IsWellFormedLocalName(element.LocalName().View()); });

    result.push_back(u8'<');
    if (InXMLFragmentSerialization() && namespaces && element.Prefix().View().empty())
    {
      // According to http://www.w3.org/TR/DOM-Level-3-Core/namespaces-algorithms.html#normalizeDocumentAlgo
      // we now should create a default namespace declaration to make this namespace well-formed. However,
      // http://www.w3.org/TR/xml-names11/#xmlReserved states "The prefix xml MUST NOT be declared as the
      // default namespace.", so use the xml prefix explicitly.
      if (element.NamespaceURI().View() == infra::Namespaces::XML)
      {
        result.append(infra::NamespacePrefixes::XML);
        result.push_back(u8':');
      }
    }
    result.append(TagQName(element, InXMLFragmentSerialization()));
    if (InXMLFragmentSerialization() && namespaces && ShouldAddNamespaceElement(element))
    {
      AppendNamespace(result, element.Prefix(), element.NamespaceURI(), *namespaces,
                      InXMLFragmentSerialization());
    }
  }

  void MarkupAccumulator::AppendCloseTag(utf8_string &result, const dom::Element &element) noexcept
  {
    if (ShouldSelfClose(element, _serializationSyntax))
    {
      if (Is<html::HTMLElement>(element))
      {
        result.push_back(u8' '); // XHTML 1.0 <-> HTML compatibility.
      }
      result.push_back(u8'/');
    }
    result.push_back(u8'>');
  }

  dom::DOMStringAtom MarkupAccumulator::GenerateUniquePrefix(const Namespaces &namespaces) noexcept
  {
    // http://www.w3.org/TR/DOM-Level-3-Core/namespaces-algorithms.html#normalizeDocumentAlgo
    // Find a prefix following the pattern "NS" + index (starting at 1) and make sure this
    // prefix is not declared in the current scope.
    dom::DOMStringAtom name = EmptyAtom();
    do
    {
      auto number = std::to_string(++_prefixLevel);
      utf8_string prefix(u8"ns");
      prefix.append(reinterpret_cast<const char8_t *>(number.data()), number.size());
      name = dom::DOMStringAtom(prefix);
    } while (namespaces.contains(name));
    return name;
  }

  // https://w3c.github.io/DOM-Parsing/#dfn-xml-serialization-of-the-attributes
  utf8_string MarkupAccumulator::XMLAttributeSerialization(const dom::Attr &attribute, Namespaces *namespaces,
                                                           dom::DOMStringAtom &prefix) noexcept
  {
    prefix = attribute.Prefix();
    auto namespaceURI = attribute.NamespaceURI();
    if (!namespaceURI.View().empty())
    {
      if (namespaceURI.View() == infra::Namespaces::XML)
      {
        // Always use xml as prefix if the namespace is the XML namespace.
        prefix = XMLAtom();
      }
      else
      {
        const dom::DOMStringAtom *foundNS = nullptr;
        if (namespaces && !attribute.Prefix().View().empty())
        {
          if (auto found = namespaces->find(attribute.Prefix()); found != namespaces->end())
          {
            foundNS = &found->second;
          }
        }
        bool prefixIsAlreadyMappedToOtherNS = foundNS && *foundNS != namespaceURI;
        if (attribute.Prefix().View().empty() || !foundNS || prefixIsAlreadyMappedToOtherNS)
        {
          auto existingPrefix = namespaces ? namespaces->find(namespaceURI) : Namespaces::iterator {};
          if (namespaces && existingPrefix != namespaces->end())
          {
            prefix = existingPrefix->second;
          }
          else
          {
            bool shouldBeDeclaredUsingAppendNamespace = !attribute.Prefix().View().empty() && !foundNS;
            if (!shouldBeDeclaredUsingAppendNamespace && attribute.LocalName() != XMLNSAtom() && namespaces)
            {
              prefix = GenerateUniquePrefix(*namespaces);
            }
          }
        }
      }
    }

    if (prefix.View().empty())
    {
      return utf8_string(attribute.LocalName().View());
    }
    utf8_string name(prefix.View());
    name.push_back(u8':');
    name.append(attribute.LocalName().View());
    return name;
  }

  // FIXME: Resolving URL attributes (isURLAttribute) when ResolveURLs asks for it, with URL attributes.
  bool MarkupAccumulator::AppendAttribute(utf8_string &result, const dom::Element &element,
                                          const dom::Attr &attribute, Namespaces *namespaces) noexcept
  {
    bool isSerializingHTML = !InXMLFragmentSerialization();

    utf8_string effectiveXMLName;

    // Per https://w3c.github.io/DOM-Parsing/#dfn-xml-serialization-of-the-attributes the xmlns attribute is
    // serialized first
    if (!isSerializingHTML)
    {
      // Error cases: an Attr node whose local name contains a colon or isn't an XML Name, an Attr node with
      // no namespace whose local name is "xmlns", and an Attr node whose data contains characters that aren't
      // XML Chars.
      CheckWellFormed(
        [&]
        {
          auto localName = attribute.LocalName().View();
          return IsWellFormedLocalName(localName)
                 && !(attribute.NamespaceURI().View().empty() && localName == u8"xmlns")
                 && IsXMLCharacterData(attribute.Value());
        });

      dom::DOMStringAtom prefix = EmptyAtom();
      effectiveXMLName = XMLAttributeSerialization(attribute, namespaces, prefix);
      if (namespaces && ShouldAddNamespaceAttribute(attribute, *namespaces))
      {
        AppendNamespace(result, prefix, attribute.NamespaceURI(), *namespaces);
      }
    }

    result.push_back(u8' ');

    if (isSerializingHTML)
    {
      result.append(HTMLAttributeSerialization(attribute));
    }
    else
    {
      result.append(effectiveXMLName);
    }

    result.append(u8"=\"");
    (void)element;
    AppendAttributeValue(result, attribute.Value());
    result.push_back(u8'"');

    return false;
  }

  void MarkupAccumulator::AppendNonElementNode(utf8_string &result, const dom::Node &node,
                                               Namespaces *) noexcept
  {
    switch (node.NodeType())
    {
      case dom::NodeType::TEXT_NODE:
      {
        auto &text = Downcast<dom::Text>(node);
        // Error case: a Text node whose data contains characters that aren't XML Chars.
        CheckWellFormed([&] { return IsXMLCharacterData(text.Data()); });
        AppendText(result, text);
        break;
      }
      case dom::NodeType::COMMENT_NODE:
      {
        // The data isn't escaped: in XML, the error cases rule out data that would end the comment.
        auto &data = Downcast<dom::Comment>(node).Data();
        // Error cases: a Comment node whose data contains characters that aren't XML Chars, or two adjacent
        // U+002D HYPHEN-MINUS characters (-), or ends with one.
        CheckWellFormed(
          [&]
          {
            return IsXMLCharacterData(data) && data.find(u8"--") == utf8_string::npos
                   && !data.ends_with(u8'-');
          });
        result.append(u8"<!--");
        result.append(data);
        result.append(u8"-->");
        break;
      }
      case dom::NodeType::DOCUMENT_NODE:
        // Error case: a Document node with no child element nodes.
        CheckWellFormed([&] { return static_cast<bool>(Downcast<dom::Document>(node).DocumentElement()); });
        AppendXMLDeclaration(result, Downcast<dom::Document>(node));
        break;
      case dom::NodeType::DOCUMENT_FRAGMENT_NODE: break;
      case dom::NodeType::DOCUMENT_TYPE_NODE:
      {
        auto &documentType = Downcast<dom::DocumentType>(node);
        // Error cases: a DocumentType node whose public identifier contains characters that aren't XML
        // PubidChars, or whose system identifier contains both a U+0022 QUOTATION MARK (") and a U+0027
        // APOSTROPHE ('), or characters that aren't XML Chars.
        CheckWellFormed(
          [&]
          {
            auto &systemId = documentType.SystemId();
            return IsXMLPublicIdentifier(documentType.PublicId())
                   && (systemId.find(u8'"') == utf8_string::npos
                       || systemId.find(u8'\'') == utf8_string::npos)
                   && IsXMLCharacterData(systemId);
          });
        AppendDocumentType(result, documentType);
        break;
      }
      case dom::NodeType::PROCESSING_INSTRUCTION_NODE:
      {
        auto &instruction = Downcast<dom::ProcessingInstruction>(node);
        // The data isn't escaped: in XML, the error cases rule out data that would end the instruction.
        // Error cases: a ProcessingInstruction node whose target is an ASCII case-insensitive match for "xml"
        // or contains a U+003A COLON (:), or whose data contains characters that aren't XML Chars, or "?>".
        CheckWellFormed(
          [&]
          {
            auto &target = instruction.Target();
            auto &data = instruction.Data();
            return !infra::StringAlgorithms::ASCIICaseInsensitiveMatch(target, u8"xml")
                   && target.find(u8':') == utf8_string::npos && IsXMLCharacterData(data)
                   && data.find(u8"?>") == utf8_string::npos;
          });
        result.append(u8"<?");
        result.append(instruction.Target());
        result.push_back(u8' ');
        result.append(instruction.Data());
        // The HTML syntax ends a processing instruction with ">".
        result.append(InXMLFragmentSerialization() ? u8"?>" : u8">");
        break;
      }
      case dom::NodeType::ELEMENT_NODE: krys_debug_assert(false); break;
      case dom::NodeType::CDATA_SECTION_NODE:
        if (InXMLFragmentSerialization())
        {
          // A CDATASection is a Text node, so the data must be XML Chars. The data isn't escaped, and there's
          // no error case for data containing "]]>" (which createCDATASection() refuses, but the data setter
          // doesn't).
          CheckWellFormed([&] { return IsXMLCharacterData(Downcast<dom::CDATASection>(node).Data()); });
          result.append(u8"<![CDATA[");
          result.append(Downcast<dom::CDATASection>(node).Data());
          result.append(u8"]]>");
        }
        else
        {
          AppendText(result, Downcast<dom::Text>(node));
        }
        break;
      case dom::NodeType::ATTRIBUTE_NODE:
        CheckWellFormed([&] { return IsXMLCharacterData(Downcast<dom::Attr>(node).Value()); });
        AppendAttributeValue(result, Downcast<dom::Attr>(node).Value());
        break;
      default: break;
    }
  }

  SerializationSyntax MarkupAccumulator::GetSerializationSyntax(dom::Document &document) noexcept
  {
    if (!Is<dom::HTMLDocument>(document))
    {
      return SerializationSyntax::XML;
    }
    return document.GetSettings().HTMLLegacyAttributeValueSerializationEnabled
             ? SerializationSyntax::HTMLLegacyAttributeValue
             : SerializationSyntax::HTML;
  }
}
