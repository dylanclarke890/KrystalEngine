#pragma once

#include "Krystal.Booey/Graphics/FontSelectionAlgorithm.hpp"
#include "Krystal.Booey/Graphics/FontTaggedSettings.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class Font;
  class FontDescription;
  class FontFeatureValues;
  class FontPaletteValues;
  enum class ExternalResourceDownloadPolicy : bool;

  namespace dom
  {
    class Document;
  }
}

namespace krys::boo::css
{
  class CSSFontFaceClient;
  class CSSFontFaceSource;
  class CSSFontSelector;
  class FontFaceRule;
  class MutableStyleProperties;
  class StyleProperties;
  class Value;
  class ValueList;

  /// @brief A font face from an `@font-face` rule (or a face of an installed family that shares its name
  /// with one): its descriptors, its sources in order, and its loading state.
  // FIXME: The `FontFace` wrapper, once there's scripting.
  class CSSFontFace final : public RefCounted<CSSFontFace>, public CanMakeWeakPtr<CSSFontFace>
  {
    KRYS_NON_COPY_MOVABLE(CSSFontFace);

  public:
    // Pending => Loading  => TimedOut
    //              ||  \\    //  ||
    //              ||   \\  //   ||
    //              ||    \\//    ||
    //              ||     //     ||
    //              ||    //\\    ||
    //              ||   //  \\   ||
    //              \/  \/    \/  \/
    //             Success    Failure
    enum class Status : uint8
    {
      Pending,
      Loading,
      TimedOut,
      Success,
      Failure
    };

    struct UnicodeRange
    {
      char32 From;
      char32 To;

      KRYS_NODISCARD bool operator==(const UnicodeRange &) const noexcept = default;
    };

    struct FontLoadTiming
    {
      /// @brief `MonotonicTime::duration::max()` is an infinite period.
      MonotonicTime::duration BlockPeriod;
      MonotonicTime::duration SwapPeriod;
    };

  private:
    RefPtr<FontFaceRule> _cssConnection;
    // The descriptors of a face without a rule.
    RefPtr<MutableStyleProperties> _properties;
    RefPtr<Value> _family;
    List<UnicodeRange> _ranges;

    FontFeatureSettings _featureSettings;
    FontLoadingBehavior _loadingBehavior {FontLoadingBehavior::Auto};

    float _sizeAdjust {1.0f};

    List<UniquePtr<CSSFontFaceSource>> _sources;
    List<WeakPtr<CSSFontFaceClient>> _clients;
    FontSelectionSpecifiedCapabilities _fontSelectionCapabilities;

    Status _status {Status::Pending};
    bool _isLocalFallback {false};
    bool _sourcesPopulated {false};
    bool _mayBePurged {true};

    Timer _timeoutTimer;

    CSSFontFace(FontFaceRule *connection, bool isLocalFallback) noexcept;

  public:
    ~CSSFontFace() noexcept;

    KRYS_NODISCARD static Ref<CSSFontFace> Create(CSSFontSelector &fontSelector,
                                                  FontFaceRule *cssConnection = nullptr,
                                                  bool isLocalFallback = false) noexcept;

    void SetFamily(Value &family) noexcept;
    void SetStyle(Value &style) noexcept;
    void SetWeight(Value &weight) noexcept;
    void SetWidth(Value &width) noexcept;
    void SetSizeAdjust(Value &sizeAdjust) noexcept;
    void SetUnicodeRange(ValueList &list) noexcept;
    void SetFeatureSettings(Value &featureSettings) noexcept;
    void SetDisplay(Value &loadingBehaviorValue) noexcept;

    KRYS_NODISCARD Span<const UnicodeRange> Ranges() const noexcept KRYS_LIFETIME_BOUND
    {
      krys_debug_assert(_status != Status::Failure);
      return _ranges;
    }

    KRYS_NODISCARD RefPtr<Value> FamilyCSSValue() const noexcept;

    void SetFontSelectionCapabilities(FontSelectionCapabilities capabilities) noexcept
    {
      _fontSelectionCapabilities = capabilities;
    }
    KRYS_NODISCARD FontSelectionCapabilities GetFontSelectionCapabilities() const noexcept
    {
      krys_debug_assert(_status != Status::Failure);
      return _fontSelectionCapabilities.ComputeFontSelectionCapabilities();
    }

    KRYS_NODISCARD bool IsLocalFallback() const noexcept
    {
      return _isLocalFallback;
    }
    KRYS_NODISCARD Status GetStatus() const noexcept
    {
      return _status;
    }
    KRYS_NODISCARD FontFaceRule *CssConnection() const noexcept;

    void AddClient(CSSFontFaceClient &client) noexcept;
    void RemoveClient(CSSFontFaceClient &client) noexcept;

    KRYS_NODISCARD bool ComputeFailureState() const noexcept;

    void OpportunisticallyStartFontDataURLLoading() noexcept;

    void AdoptSource(UniquePtr<CSSFontFaceSource> &&source) noexcept;
    void SourcesPopulated() noexcept
    {
      _sourcesPopulated = true;
    }
    KRYS_NODISCARD size_t SourceCount() const noexcept
    {
      return _sources.size();
    }

    void FontLoaded(CSSFontFaceSource &source) noexcept;

    void Load() noexcept;

    KRYS_NODISCARD RefPtr<Font> GetFont(const FontDescription &fontDescription, bool syntheticBold,
                                        bool syntheticItalic, ExternalResourceDownloadPolicy policy,
                                        const FontPaletteValues &fontPaletteValues,
                                        RefPtr<FontFeatureValues> fontFeatureValues) noexcept;

    static void AppendSources(CSSFontFace &fontFace, ValueList &srcList, dom::Document *context,
                              bool isInitiatingElementInUserAgentShadowTree) noexcept;

    KRYS_NODISCARD bool RangesMatchCodePoint(char32 character) const noexcept;

    KRYS_NODISCARD FontLoadTiming GetFontLoadTiming() const noexcept;
    KRYS_NODISCARD bool ShouldIgnoreFontLoadCompletions() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool Purgeable() const noexcept;

    KRYS_NODISCARD AllowUserInstalledFonts GetAllowUserInstalledFonts() const noexcept
    {
      return AllowUserInstalledFonts::Yes;
    }

    void UpdateStyleIfNeeded() noexcept;

    void SetErrorState() noexcept;

  private:
    size_t Pump(ExternalResourceDownloadPolicy policy) noexcept;
    void SetStatus(Status newStatus) noexcept;

    void FontLoadEventOccurred() noexcept;
    void TimeoutFired() noexcept;

    KRYS_NODISCARD const StyleProperties &Properties() const noexcept;
    KRYS_NODISCARD MutableStyleProperties &MutableProperties() noexcept;

    template <typename TFunc>
    void IterateClients(TFunc &&callback) noexcept;
  };

  /// @brief Something told when a font face loads, changes state or has its descriptors changed.
  class CSSFontFaceClient : public AbstractRefCountedAndCanMakeWeakPtr<CSSFontFaceClient>
  {
  public:
    ~CSSFontFaceClient() noexcept override = default;

    virtual void FontLoaded(CSSFontFace &) noexcept
    {
    }
    virtual void FontStateChanged(CSSFontFace &, CSSFontFace::Status /* oldState */,
                                  CSSFontFace::Status /* newState */) noexcept
    {
    }
    virtual void FontPropertyChanged(CSSFontFace &, Value * /* oldFamily */ = nullptr) noexcept
    {
    }
    virtual void UpdateStyleIfNeeded(CSSFontFace &) noexcept
    {
    }
  };
}
