#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResource.hpp"
#include "Krystal.Booey/SVG/SVGDocument.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  struct Settings;

  /// @brief A cached SVG file, parsed into a document of its own (without a frame) for the elements that
  /// reference it (`<use>`).
  // FIXME: Encodings other than UTF-8 (the response's charset and the XML declaration's), once there are
  // decoders for them.
  class CachedSVGDocument final : public CachedResource
  {
  private:
    RefPtr<svg::SVGDocument> _document;
    const Ref<Settings> _settings;

  public:
    CachedSVGDocument(CachedResourceRequest &&, Ref<Settings> &&) noexcept;
    ~CachedSVGDocument() noexcept override;

    KRYS_NODISCARD svg::SVGDocument *Document() const noexcept
    {
      return _document.get();
    }

  private:
    void FinishLoading(const SharedBuffer *) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE(CachedSVGDocument, CachedResource::Type::SVGDocumentResource)
