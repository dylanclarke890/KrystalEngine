#include "Krystal.Booey/CSS/Font/FontFaceSource.hpp"
#include "Krystal.Booey/CSS/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Font/FontFace.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Graphics/FontCascadeDescription.hpp"
#include "Krystal.Booey/Graphics/FontCreationContext.hpp"
#include "Krystal.Booey/URL/URL.hpp"

namespace krys::boo::css
{
  void CSSFontFaceSource::SetStatus(Status newStatus) noexcept
  {
    switch (newStatus)
    {
      case Status::Pending: krys_debug_assert(false); break;
      case Status::Loading: krys_debug_assert(GetStatus() == Status::Pending); break;
      case Status::Success: krys_debug_assert(GetStatus() == Status::Loading); break;
      case Status::Failure: krys_debug_assert(GetStatus() == Status::Loading); break;
    }

    _status = newStatus;
  }

  CSSFontFaceSource::CSSFontFaceSource(CSSFontFace &owner, StringAtom fontFaceName) noexcept
      : _fontFaceName(krys::move(fontFaceName)), _owningCSSFontFace(owner)
  {
  }

  CSSFontFaceSource::CSSFontFaceSource(CSSFontFace &owner, CSSFontSelector &fontSelector,
                                       Ref<FontLoadRequest> &&request) noexcept
      : _owningCSSFontFace(owner), _fontSelector(WeakPtr<CSSFontSelector>::WithRef(fontSelector)),
        _fontRequest(krys::move(request))
  {
    // This may synchronously call fontLoaded().
    _fontRequest->SetClient(this);

    if (GetStatus() == Status::Pending && !_fontRequest->IsPending())
    {
      SetStatus(Status::Loading);
      if (!ShouldIgnoreFontLoadCompletions())
      {
        if (_fontRequest->ErrorOccurred())
        {
          SetStatus(Status::Failure);
        }
        else
        {
          SetStatus(Status::Success);
        }
      }
    }
  }

  CSSFontFaceSource::~CSSFontFaceSource() noexcept
  {
    if (_fontRequest)
    {
      _fontRequest->SetClient(nullptr);
    }
  }

  void CSSFontFaceSource::AddRef() const noexcept
  {
    _owningCSSFontFace.AddRef();
  }

  void CSSFontFaceSource::SubRef() const noexcept
  {
    _owningCSSFontFace.SubRef();
  }

  uint32 CSSFontFaceSource::GetRefCount() const noexcept
  {
    return _owningCSSFontFace.GetRefCount();
  }

  bool CSSFontFaceSource::ShouldIgnoreFontLoadCompletions() const noexcept
  {
    return _owningCSSFontFace.ShouldIgnoreFontLoadCompletions();
  }

  void CSSFontFaceSource::OpportunisticallyStartFontDataURLLoading() noexcept
  {
    constexpr size_t MB = 1'024 * 1'024;
    if (GetStatus() == Status::Pending && _fontRequest && _fontRequest->URL().ProtocolIsData()
        && _fontRequest->URL().String().size() < MB)
    {
      Load();
    }
  }

  void CSSFontFaceSource::FontLoaded(FontLoadRequest &fontRequest) noexcept
  {
    krys_debug_assert(&fontRequest == _fontRequest.get());
    (void)fontRequest;

    if (ShouldIgnoreFontLoadCompletions())
    {
      return;
    }

    // If the font is in the cache, this will be synchronously called from FontLoadRequest::addClient().
    if (_status == Status::Pending)
    {
      SetStatus(Status::Loading);
    }
    else if (_status == Status::Failure)
    {
      // This function may be called twice if loading was cancelled.
      krys_debug_assert(_fontRequest->ErrorOccurred());
      return;
    }

    if (_fontRequest->ErrorOccurred() || !_fontRequest->EnsureCustomFontData())
    {
      SetStatus(Status::Failure);
    }
    else
    {
      SetStatus(Status::Success);
    }

    Ref<CSSFontFace> protectedFace = ShareRef(_owningCSSFontFace);
    protectedFace->FontLoaded(*this);
  }

  void CSSFontFaceSource::Load(dom::Document *) noexcept
  {
    SetStatus(Status::Loading);

    if (_fontRequest)
    {
      krys_debug_assert(_fontSelector);
      if (auto *context = _fontSelector ? _fontSelector->GetDocument() : nullptr)
      {
        context->BeginLoadingFontSoon(*_fontRequest);
      }
    }
    else
    {
      // We are only interested in whether or not fontForFamily() returns null or not. Luckily, none of
      // the values in the FontDescription other than the family name can cause the function to return
      // null if it wasn't going to otherwise (and vice-versa).
      FontCascadeDescription fontDescription;
      fontDescription.SetOneFamily(_fontFaceName);
      fontDescription.SetComputedSize(1);
      fontDescription.SetShouldAllowUserInstalledFonts(_owningCSSFontFace.GetAllowUserInstalledFonts());
      bool success = FontCache::Singleton().FontForFamily(fontDescription, _fontFaceName,
                                                          {FontLookupOptions::ExactFamilyNameMatch})
                     != nullptr;
      SetStatus(success ? Status::Success : Status::Failure);
    }
  }

  RefPtr<Font> CSSFontFaceSource::GetFont(const FontDescription &fontDescription, bool syntheticBold,
                                          bool syntheticItalic,
                                          const FontCreationContext &fontCreationContext) noexcept
  {
    krys_debug_assert(GetStatus() == Status::Success);

    if (!_fontRequest)
    {
      // We're local. Just return a Font from the normal cache.
      // We don't want to check alternate font family names here, so pass true as the exactFamilyNameMatch
      // parameter.
      EnumSet<FontLookupOptions> options {FontLookupOptions::ExactFamilyNameMatch};
      if (!syntheticBold)
      {
        options.Add(FontLookupOptions::DisallowBoldSynthesis);
      }
      if (!syntheticItalic)
      {
        options.Add(FontLookupOptions::DisallowObliqueSynthesis);
      }
      // FIXME: The font face's features, palette and size adjustment (fontCreationContext), once the font
      // cache takes them.
      return FontCache::Singleton().FontForFamily(fontDescription, _fontFaceName, options);
    }

    auto success = _fontRequest->EnsureCustomFontData();
    krys_debug_assert(success);
    (void)success;

    krys_debug_assert(GetStatus() == Status::Success);
    auto result =
      _fontRequest->CreateFont(fontDescription, syntheticBold, syntheticItalic, fontCreationContext);
    krys_debug_assert(result);
    return result;
  }
}
