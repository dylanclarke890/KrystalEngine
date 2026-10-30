#include "Krystal.Booey/Loader/Cache/CachedSVGDocument.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Booey/XML/Parser/XMLDocumentParser.hpp"
#include "Krystal.Core/Text/Encodings/Decode.hpp"
#include "Krystal.Core/Text/Encodings/Encode.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"

namespace krys::boo
{
  namespace
  {
    constexpr text::DecodePolicy DocumentDecodePolicy {
      .OnInvalidSequence = text::EncodingErrorAction::Replace,
      .OnIncompleteSequence = text::EncodingErrorAction::Replace,
      .CheckByteOrderMark = text::CheckForBOM(true)};

    /// @brief Decodes a document's bytes as UTF-8, dropping a byte order mark and replacing invalid
    /// sequences.
    KRYS_NODISCARD utf8_string DecodeDocument(Span<const uint8> bytes) noexcept
    {
      if (bytes.empty())
      {
        return {};
      }
      auto decoded = text::Decode<text::UTF8, DocumentDecodePolicy>(
        utf8_stringview(reinterpret_cast<const char8 *>(bytes.data()), bytes.size()));
      auto encoded = text::Encode<text::UTF8>(decoded.Output);
      return utf8_string(encoded.Output.begin(), encoded.Output.end());
    }
  }

  CachedSVGDocument::CachedSVGDocument(CachedResourceRequest &&request, Ref<Settings> &&settings) noexcept
      : CachedResource(krys::move(request), Type::SVGDocumentResource), _settings(krys::move(settings))
  {
  }

  CachedSVGDocument::~CachedSVGDocument() noexcept = default;

  void CachedSVGDocument::FinishLoading(const SharedBuffer *data) noexcept
  {
    if (data)
    {
      // We don't need to create a new frame because the new document belongs to the parent UseElement.
      auto document = CreateRef<svg::SVGDocument>();
      document->SetSettings(Ref<Settings>(_settings));
      document->SetURL(url::URL(Response().URL()));

      auto parser = xml::XMLDocumentParser::Create(*document);
      parser->Append(DecodeDocument(data->GetSpan()));
      parser->Finish();
      parser->Detach();

      _document = krys::move(document);
    }
    CachedResource::FinishLoading(data);
  }
}
