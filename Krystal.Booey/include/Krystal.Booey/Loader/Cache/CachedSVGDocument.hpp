#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResource.hpp"
#include "Krystal.Booey/Svg/SVGDocument.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo
{
  struct Settings;

  /// @brief A cached SVG file, parsed into a document of its own (without a frame) for the elements that
  /// reference it (`<use>`).
  // SVG documents are UTF-8 only, whatever the response or the XML declaration say (see DIFFERENCES.md).
  class CachedSVGDocument final : public CachedResource
  {
  private:
    RefPtr<SVGDocument> _document;
    const Ref<Settings> _settings;

  public:
    CachedSVGDocument(CachedResourceRequest &&, Ref<Settings> &&) noexcept;

    ~CachedSVGDocument() noexcept override;

    /// @brief The parsed document, once it's loaded.
    KRYS_NODISCARD SVGDocument *Document() const noexcept
    {
      return _document.get();
    }

  private:
    void FinishLoading(const SharedBuffer *) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE(CachedSVGDocument, CachedResource::Type::SVGDocumentResource)
