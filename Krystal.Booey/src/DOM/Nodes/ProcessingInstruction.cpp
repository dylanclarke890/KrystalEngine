#include "Krystal.Booey/DOM/Nodes/ProcessingInstruction.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Loader/Cache/CachedCSSStyleSheet.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Booey/XML/XMLDocumentParser.hpp"

namespace krys::boo::dom
{
  ProcessingInstruction::ProcessingInstruction(Document &document, DOMString target, DOMString data) noexcept
      : CharacterData(document, krys::move(data), NodeType::PROCESSING_INSTRUCTION_NODE),
        _target(krys::move(target))
  {
    // Initialize:
    // 3. Set pi's target to target.
    // 4. Set pi's data to data.
    // 5. Update attributes from data given pi.
    UpdateAttributesFromData();
  }

  ExceptionOr<Ref<ProcessingInstruction>> ProcessingInstruction::Create(Document &document, DOMString target,
                                                                        DOMString data) noexcept
  {
    // Create a processing instruction node:
    // 1. Let pi be the result of creating a node that implements ProcessingInstruction, given document.
    // 2. Initialize pi with target and data:
    //    1. If target does not match the Name production, then throw an "InvalidCharacterError" DOMException.
    if (!NameValidation::IsValidXMLName(target))
    {
      return ExceptionCode::InvalidCharacterError;
    }

    // 2. If data contains the string "?>", then throw an "InvalidCharacterError" DOMException.
    if (data.contains(u8"?>"))
    {
      return ExceptionCode::InvalidCharacterError;
    }

    // 3.-5. In the constructor.
    // 3. Return pi.
    return AdoptRef<ProcessingInstruction>(
      *new ProcessingInstruction(document, krys::move(target), krys::move(data)));
  }

  List<DOMString> ProcessingInstruction::GetAttributeNames() const noexcept
  {
    // Return the result of getting the keys of this's attribute map.
    List<DOMString> names;
    names.reserve(_attributeMap.size());
    for (auto &attribute : _attributeMap)
    {
      names.push_back(attribute.Name);
    }
    return names;
  }

  Maybe<DOMString> ProcessingInstruction::GetAttribute(DOMStringView name) const noexcept
  {
    // Return pi's attribute map[name] with default null.
    auto it = std::ranges::find(_attributeMap, name, &xml::PseudoAttribute::Name);
    if (it == _attributeMap.end())
    {
      return null;
    }
    return it->Value;
  }

  ExceptionOr<void> ProcessingInstruction::SetAttribute(DOMStringView name, DOMString value) noexcept
  {
    // 1. If name is not a valid attribute local name, then throw an "InvalidCharacterError" DOMException.
    if (!NameValidation::IsValidAttributeLocalName(name))
    {
      return ExceptionCode::InvalidCharacterError;
    }

    // 2. Set this's attribute map[name] to value.
    auto it = std::ranges::find(_attributeMap, name, &xml::PseudoAttribute::Name);
    if (it != _attributeMap.end())
    {
      it->Value = krys::move(value);
    }
    else
    {
      _attributeMap.push_back({DOMString(name), krys::move(value)});
    }

    // 3. Update data from attributes given this.
    UpdateDataFromAttributes();
    return {};
  }

  void ProcessingInstruction::RemoveAttribute(DOMStringView name) noexcept
  {
    // 1. Remove this's attribute map[name].
    std::erase_if(_attributeMap,
                  [&](const xml::PseudoAttribute &attribute) { return attribute.Name == name; });
    // 2. Update data from attributes given this.
    UpdateDataFromAttributes();
  }

  ExceptionOr<bool> ProcessingInstruction::ToggleAttribute(DOMStringView name, Maybe<bool> force) noexcept
  {
    // 1. If name is not a valid attribute local name, then throw an "InvalidCharacterError" DOMException.
    if (!NameValidation::IsValidAttributeLocalName(name))
    {
      return ExceptionCode::InvalidCharacterError;
    }

    // 2. Let attributes be this's attribute map.
    auto it = std::ranges::find(_attributeMap, name, &xml::PseudoAttribute::Name);
    // 3. If attributes[name] does not exist:
    if (it == _attributeMap.end())
    {
      // 3a. If force is not given or is true, set attributes[name] to the empty string, update data from
      //     attributes given this, and then return true.
      if (!force.has_value() || *force)
      {
        _attributeMap.push_back({DOMString(name), DOMString()});
        UpdateDataFromAttributes();
        return true;
      }

      // 3b. Return false.
      return false;
    }

    // 4. If force is not given or is false, remove attributes[name], update data from attributes given this,
    // and
    //    then return false.
    if (!force.has_value() || !*force)
    {
      _attributeMap.erase(it);
      UpdateDataFromAttributes();
      return false;
    }

    // 5. Return true.
    return true;
  }

  bool ProcessingInstruction::HasAttribute(DOMStringView name) const noexcept
  {
    // Return true if this's attribute map[name] exists; otherwise false.
    return std::ranges::find(_attributeMap, name, &xml::PseudoAttribute::Name) != _attributeMap.end();
  }

  void ProcessingInstruction::UpdateAttributesFromData() noexcept
  {
    // 1. Clear pi's attribute map.
    _attributeMap.clear();

    // 2. Let result be the parsing result of invoking the rules for parsing pseudo-attributes from a string
    // given
    //    pi's data.
    auto result = xml::ParsePseudoAttributes(Data());

    // 3. If result is an error, then return.
    if (!result)
    {
      return;
    }

    // 4. For each pseudo-attribute pseudoAttr of result: set pi's attribute map[pseudoAttr's name] to
    //    pseudoAttr's value.
    _attributeMap = krys::move(*result);
  }

  void ProcessingInstruction::UpdateDataFromAttributes() noexcept
  {
    // 1. Let data be the empty string.
    DOMString data;

    // 2. For each name -> value of pi's attribute map:
    for (auto &[name, value] : _attributeMap)
    {
      // 2a. If data is not the empty string, then append U+0020 SPACE to data.
      if (!data.empty())
      {
        data += u8' ';
      }

      // 2b. Append name to data.
      // 2c. Append U+003D (=) to data.
      // 2d. Append U+0022 (") to data.
      data += name;
      data += u8"=\"";

      // 2e. Replace any U+0026 (&) in value with "&amp;".
      // 2f. Replace any U+003C (<) in value with "&lt;".
      // 2g. Replace any U+003E (>) in value with "&gt;".
      // 2h. Replace any U+0022 (") in value with "&quot;".
      // 2i. Append value to data.
      for (auto ch : value)
      {
        switch (ch)
        {
          case u8'&': data += u8"&amp;"; break;
          case u8'<': data += u8"&lt;"; break;
          case u8'>': data += u8"&gt;"; break;
          case u8'"': data += u8"&quot;"; break;
          default:    data += ch; break;
        }
      }

      // 2j. Append U+0022 (") to data.
      data += u8'"';
    }

    // 3. Replace data of pi with 0, pi's length, data, and true.
    (void)TextAlgorithms::Replace(*this, 0uz, Length(), krys::move(data), true);
  }

  ProcessingInstruction::~ProcessingInstruction() noexcept
  {
    if (RefPtr<css::CSSStyleSheet> sheet = _sheet)
    {
      sheet->ClearOwnerNode();
    }

    if (CachedResourceHandle cachedSheet = _cachedSheet)
    {
      cachedSheet->RemoveClient(*this);
    }

    if (IsConnected())
    {
      NodeDocument().StyleScope().RemoveStyleSheetCandidateNode(*this);
    }
  }

  css::CSSStyleSheet *ProcessingInstruction::Sheet() const noexcept
  {
    return _sheet.get();
  }

  void ProcessingInstruction::CheckStyleSheet() noexcept
  {
    Ref<Document> document = ShareRef(NodeDocument());
    if (_target == u8"xml-stylesheet" && document->Frame() && ParentNode() == document.get())
    {
      // see http://www.w3.org/TR/xml-stylesheet/
      // ### support stylesheet included in a fragment of this (or another) document
      // The pseudo-attributes are the attribute map; it's empty if the data didn't parse.
      if (!HasAttributes())
      {
        return;
      }
      auto get = [&](utf8_stringview name)
      {
        return GetAttribute(name).value_or(DOMString());
      };
      auto type = get(u8"type");

      _isCSS = type.empty() || type == u8"text/css";
      if (!_isCSS)
      {
        return;
      }

      auto href = get(u8"href");
      auto alternate = get(u8"alternate");
      _alternate = alternate == u8"yes";
      _title = get(u8"title");
      _media = get(u8"media");

      if (_alternate && _title.empty())
      {
        return;
      }

      if (href.size() > 1 && href[0] == u8'#')
      {
        _localHref = href.substr(1);
      }
      else
      {
        if (CachedResourceHandle cachedSheet = std::exchange(_cachedSheet, nullptr))
        {
          cachedSheet->RemoveClient(*this);
        }

        if (!_loading)
        {
          _loading = true;
          document->StyleScope().AddPendingSheet(*this);
        }

        krys_debug_assert(!_cachedSheet);

        // FIXME: The charset pseudo-attribute's encoding, or else the document's, once sheets decode other
        // encodings.
        CachedResourceRequest request(ResourceRequest(document->CompleteURL(href)),
                                      CachedResourceLoader::DefaultCachedResourceOptions());
        request.SetCharset(get(u8"charset"));

        auto requested = document->GetCachedResourceLoader().RequestCSSStyleSheet(krys::move(request));
        _cachedSheet = requested ? *requested : CachedResourceHandle<CachedCSSStyleSheet>();
        if (CachedResourceHandle cachedSheet = _cachedSheet)
        {
          cachedSheet->AddClient(*this);
        }
        else
        {
          // The request may have been denied if (for example) the stylesheet is local and the document is
          // remote.
          _loading = false;
          document->StyleScope().RemovePendingSheet(*this);
        }
      }
    }
  }

  bool ProcessingInstruction::IsLoading() const noexcept
  {
    if (_loading)
    {
      return true;
    }
    return _sheet && _sheet->IsLoading();
  }

  bool ProcessingInstruction::SheetLoaded() noexcept
  {
    if (!IsLoading())
    {
      auto &styleScope = NodeDocument().StyleScope();
      if (styleScope.HasPendingSheet(*this))
      {
        styleScope.RemovePendingSheet(*this);
      }
      return true;
    }
    return false;
  }

  void ProcessingInstruction::SetCSSStyleSheet(const utf8_string &href, const url::URL &baseURL,
                                               utf8_stringview charset,
                                               const CachedCSSStyleSheet *sheet) noexcept
  {
    if (!IsConnected())
    {
      krys_debug_assert(!_sheet);
      return;
    }

    krys_debug_assert(sheet);

    Ref<Document> document = ShareRef(NodeDocument());
    krys_debug_assert(_isCSS);
    css::ParserContext parserContext(*document, baseURL, charset);

    // FIXME: Whether the cached sheet is CORS-same-origin, once there are origins.
    auto cssSheet = css::CSSStyleSheet::Create(
      css::StyleSheetContents::Create(css::CSSOMString(href), parserContext), *this, true);
    cssSheet->SetDisabled(_alternate);
    cssSheet->SetTitle(css::CSSOMString(_title));
    cssSheet->SetMediaQueries(css::mq::MediaQueryParser::Parse(_media, document->CssParserContext()));

    _sheet = krys::move(cssSheet);

    // We don't need the cross-origin security check here because we are
    // getting the sheet text in "strict" mode. This enforces a valid CSS MIME
    // type.
    ParseStyleSheet(sheet->SheetText().value_or(utf8_string()));
  }

  void ProcessingInstruction::ParseStyleSheet(const utf8_string &sheet) noexcept
  {
    Ref<css::CSSStyleSheet> styleSheet = ShareRef(*_sheet);
    styleSheet->Contents().ParseString(css::CSSOMString(sheet));

    if (CachedResourceHandle cachedSheet = std::exchange(_cachedSheet, nullptr))
    {
      cachedSheet->RemoveClient(*this);
    }

    _loading = false;

    styleSheet->Contents().CheckLoaded();
  }

  void ProcessingInstruction::OnInsert() noexcept
  {
    CharacterData::OnInsert();
    if (!IsConnected())
    {
      return;
    }
    NodeDocument().StyleScope().AddStyleSheetCandidateNode(*this, _createdByParser);
  }

  void ProcessingInstruction::OnPostConnection() noexcept
  {
    CharacterData::OnPostConnection();
    CheckStyleSheet();
  }

  void ProcessingInstruction::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    CharacterData::OnRemove(isSubtreeRoot, oldAncestor);
    if (!oldAncestor.IsConnected())
    {
      return;
    }

    auto &styleScope = NodeDocument().StyleScope();
    styleScope.RemoveStyleSheetCandidateNode(*this);

    if (RefPtr<css::CSSStyleSheet> sheet = std::exchange(_sheet, nullptr))
    {
      sheet->ClearOwnerNode();
    }

    if (_loading)
    {
      _loading = false;
      styleScope.RemovePendingSheet(*this);
    }

    styleScope.DidChangeActiveStyleSheetCandidates();
  }
}
