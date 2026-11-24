#include "Krystal.Booey/Loader/Cache/CachedSVGDocument.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/XML/XMLDocumentParser.hpp"
#include "Krystal.Core/Types/SharedBuffer.hpp"
#include "Krystal.Text/Encodings/Decode.hpp"
#include "Krystal.Text/Encodings/Encode.hpp"
#include "Krystal.Text/Encodings/UTF.hpp"

namespace krys::boo
{
  namespace
  {
    constexpr text::DecodePolicy DocumentDecodePolicy {
      .OnInvalidSequence = text::EncodingErrorAction::Replace,
      .OnIncompleteSequence = text::EncodingErrorAction::Replace,
      .CheckByteOrderMark = text::CheckForBOM::Yes};

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
      // The document has no frame of its own: it's only for the elements that reference it.
      auto document = CreateRef<SVGDocument>();
      document->SetSettings(Ref<Settings>(_settings));
      document->SetUrl(net::Url(Response().Url()));

      auto parser = xml::XMLDocumentParser::Create(*document);
      parser->Append(DecodeDocument(data->GetSpan()));
      parser->Finish();
      parser->Detach();

      _document = krys::move(document);
    }
    CachedResource::FinishLoading(data);
  }
}
