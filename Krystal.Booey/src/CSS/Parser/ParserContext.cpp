#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::css
{
  namespace
  {
    void ApplyUASheetBehaviorsToContext(ParserContext &context) noexcept
    {
      context.thumbAndTrackPseudoElementsEnabled = true;
      context.cssInternalAutoBaseParsingEnabled = true;
    }
  }

  const ParserContext &StrictParserContext() noexcept
  {
    static MainThreadNeverDestroyed<ParserContext> strictContext(ParserMode::HTMLStandard);
    return strictContext;
  }

  ParserContext::ParserContext(ParserMode mode, const url::URL &baseURL) noexcept
      : BaseURL(baseURL), Mode(mode)
  {
    if (IsUASheetBehavior(mode))
    {
      ApplyUASheetBehaviorsToContext(*this);
    }

    StaticValuePool::Init();
  }
  ParserContext::ParserContext(const dom::Document &document) noexcept
      : ParserContext(document.CssParserContext())
  {
  }

  ParserContext::ParserContext(const dom::Document &document, const url::URL &sheetBaseURL,
                               utf8_stringview charset) noexcept
      : ParserContext(ParserMode::HTMLStandard)
  {
    BaseURL = sheetBaseURL.IsNull() ? document.BaseURL() : sheetBaseURL;
    Charset = charset;
    Mode = document.InQuirksMode() ? ParserMode::HTMLQuirks : ParserMode::HTMLStandard;
    IsHTMLDocument = Is<dom::HTMLDocument>(document);
    // FIXME: Whether the document's security origin can request the base URL, once there are origins;
    // every URL can be requested until then.
    HasDocumentSecurityOrigin = true;

    auto &settings = document.GetSettings();
    useSystemAppearance = settings.UseSystemAppearance;
    springTimingFunctionEnabled = settings.SpringTimingFunctionEnabled;
    cssTransformStyleSeparatedEnabled = settings.CSSTransformStyleSeparatedEnabled;
    thumbAndTrackPseudoElementsEnabled = settings.ThumbAndTrackPseudoElementsEnabled;
    colorLayersEnabled = settings.CSSColorLayersEnabled;
    PropertySettings = css::PropertySettings(settings);
    // FIXME: The media text track display quirk, once documents have quirks.
  }

  void ParserContext::SetUASheetMode() noexcept
  {
    Mode = ParserMode::UASheet;
    ApplyUASheetBehaviorsToContext(*this);
  }

  size_t ParserContext::Hash() const noexcept
  {
    uint32 bits =
      static_cast<uint32>(IsHTMLDocument) << 0 | static_cast<uint32>(HasDocumentSecurityOrigin) << 1
      | static_cast<uint32>(static_cast<bool>(LoadedFromOpaqueSource)) << 2
      | static_cast<uint32>(useSystemAppearance) << 3 | static_cast<uint32>(springTimingFunctionEnabled) << 4
      | static_cast<uint32>(cssTransformStyleSeparatedEnabled) << 5
      | static_cast<uint32>(thumbAndTrackPseudoElementsEnabled) << 6
      | static_cast<uint32>(colorLayersEnabled) << 7
      | static_cast<uint32>(cssInternalAutoBaseParsingEnabled) << 8
      | static_cast<uint32>(webkitMediaTextTrackDisplayQuirkEnabled) << 9;
    return krys::Hash::Combine(BaseURL, Charset, PropertySettings, Mode, bits);
  }
}
