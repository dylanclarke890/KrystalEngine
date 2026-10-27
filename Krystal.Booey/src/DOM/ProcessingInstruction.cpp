#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Loader/Cache/CachedCSSStyleSheet.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/XML/Parser/XMLDocumentParser.hpp"

namespace krys::boo::dom
{
  ProcessingInstruction::ProcessingInstruction(Document &document, DOMString &&target,
                                               DOMString &&data) noexcept
      : CharacterData(document, krys::move(data), NodeType::PROCESSING_INSTRUCTION_NODE),
        _target(krys::move(target))
  {
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
      auto attributes = xml::ParseAttributes(Data());
      if (!attributes)
      {
        return;
      }
      auto get = [&](utf8_stringview name)
      {
        auto it = attributes->find(DOMString(name));
        return it != attributes->end() ? it->second : DOMString();
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
