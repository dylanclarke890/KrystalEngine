#pragma once

#include "Krystal.Booey/Loader/FontLoadRequest.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class Font;
  class FontCreationContext;
  class FontDescription;

  namespace dom
  {
    class Document;
  }
}

namespace krys::boo::css
{
  class CSSFontFace;
  class CSSFontSelector;

  /// @brief One source of a font face: an installed font (`local()`) or a web font file (`url()`).
  /// @note Owned by its font face, which it shares its reference count with.
  // FIXME: SVG fonts (`<font-face>` elements) and fonts from array buffers (the `FontFace` interface), once
  // there are those.
  class CSSFontFaceSource final : public FontLoadRequestClient
  {
    KRYS_NON_COPY_MOVABLE(CSSFontFaceSource);

  public:
    // clang-format off
    //                      => Success
    //                    //
    // Pending => Loading
    //                    \\.
    //                      => Failure
    // clang-format on
    enum class Status : uint8
    {
      Pending,
      Loading,
      Success,
      Failure
    };

  private:
    StringAtom _fontFaceName {StringAtom::Null()}; // Font name for local fonts
    CSSFontFace &_owningCSSFontFace;
    WeakPtr<CSSFontSelector> _fontSelector;     // For remote fonts, to orchestrate loading.
    const RefPtr<FontLoadRequest> _fontRequest; // Also for remote fonts, a pointer to the resource request.

    Status _status {Status::Pending};

  public:
    CSSFontFaceSource(CSSFontFace &owner, StringAtom fontFaceName) noexcept;
    CSSFontFaceSource(CSSFontFace &owner, CSSFontSelector &fontSelector,
                      Ref<FontLoadRequest> &&request) noexcept;
    ~CSSFontFaceSource() noexcept override;

    // FontLoadRequestClient.
    void AddRef() const noexcept final;
    void SubRef() const noexcept final;
    KRYS_NODISCARD uint32 GetRefCount() const noexcept final;

    KRYS_NODISCARD Status GetStatus() const noexcept
    {
      return _status;
    }

    void OpportunisticallyStartFontDataURLLoading() noexcept;

    void Load(dom::Document *document = nullptr) noexcept;
    KRYS_NODISCARD RefPtr<Font> GetFont(const FontDescription &fontDescription, bool syntheticBold,
                                        bool syntheticItalic,
                                        const FontCreationContext &fontCreationContext) noexcept;

    KRYS_NODISCARD FontLoadRequest *GetFontLoadRequest() const noexcept
    {
      return _fontRequest.get();
    }
    KRYS_NODISCARD bool RequiresExternalResource() const noexcept
    {
      return _fontRequest.get();
    }

  private:
    KRYS_NODISCARD bool ShouldIgnoreFontLoadCompletions() const noexcept;

    void FontLoaded(FontLoadRequest &fontRequest) noexcept override;

    void SetStatus(Status newStatus) noexcept;
  };
}
