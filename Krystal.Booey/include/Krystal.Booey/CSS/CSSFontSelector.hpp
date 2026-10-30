#pragma once

#include "Krystal.Booey/CSS/Font/FontFace.hpp"
#include "Krystal.Booey/Graphics/FontPaletteValues.hpp"
#include "Krystal.Booey/Graphics/FontSelector.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css
{
  class CSSFontFaceSet;
  class FontFaceRule;
  class FontFeatureValuesRule;
  class FontPaletteValuesRule;

  /// @brief A document's font selector: resolves the generic families with the document's settings and finds
  /// the fonts of the families its styles name, from its `@font-face` rules first.
  // FIXME: The `FontFaceSet` interface (`document.fonts`), once there's scripting.
  class CSSFontSelector final : public FontSelector
  {
    KRYS_NON_COPY_MOVABLE(CSSFontSelector);

  private:
    /// @brief The selector as a client of its font faces.
    class FontFaceClientImpl final : public CSSFontFaceClient
    {
    private:
      CSSFontSelector &_owner;

    public:
      explicit FontFaceClientImpl(CSSFontSelector &owner) noexcept : _owner(owner)
      {
      }

      void AddRef() const noexcept final
      {
        _owner.AddRef();
      }
      void SubRef() const noexcept final
      {
        _owner.SubRef();
      }
      KRYS_NODISCARD uint32 GetRefCount() const noexcept final
      {
        return _owner.GetRefCount();
      }

      void FontLoaded(CSSFontFace &fontFace) noexcept final;
      void UpdateStyleIfNeeded(CSSFontFace &fontFace) noexcept final;
    };

    struct PendingFontFaceRule
    {
      Ref<FontFaceRule> StyleRuleFontFace;
      bool IsInitiatingElementInUserAgentShadowTree;
    };

    struct PaletteKey
    {
      StringAtom Family;
      StringAtom Name;

      KRYS_NODISCARD bool operator==(const PaletteKey &) const noexcept = default;
    };

    struct PaletteKeyHash
    {
      KRYS_NODISCARD size_t operator()(const PaletteKey &key) const noexcept;
    };

    WeakPtr<dom::Document> _document;
    Ref<CSSFontFaceSet> _cssFontFaceSet;
    FontFaceClientImpl _fontFaceClient;
    List<FontSelectorClient *> _clients;

    List<PendingFontFaceRule> _stagingArea;
    HashSet<FontFaceRule *> _cssConnectionsEncounteredDuringBuild;
    List<Ref<CSSFontFace>> _cssConnectionsPossiblyToRemove;
    HashMap<PaletteKey, FontPaletteValues, PaletteKeyHash> _paletteMap;
    HashMap<utf8_string, Ref<FontFeatureValues>> _featureValues;

    uint32 _uniqueId;
    uint32 _version {0};
    uint32 _computingRootStyleFontCount {0};
    bool _creatingFont {false};
    bool _buildIsUnderway {false};
    bool _isStopped {false};

    explicit CSSFontSelector(dom::Document &document) noexcept;

  public:
    ~CSSFontSelector() noexcept override;

    KRYS_NODISCARD static Ref<CSSFontSelector> Create(dom::Document &document) noexcept;

    KRYS_NODISCARD uint32 Version() const noexcept final
    {
      return _version;
    }

    KRYS_NODISCARD uint32 UniqueId() const noexcept final
    {
      return _uniqueId;
    }

    KRYS_NODISCARD FontRanges FontRangesForFamily(const FontDescription &description,
                                                  const StringAtom &family) noexcept final;
    KRYS_NODISCARD size_t FallbackFontCount() noexcept final;
    KRYS_NODISCARD RefPtr<Font> FallbackFontAt(const FontDescription &description,
                                               size_t index) noexcept final;

    void ClearFonts() noexcept;
    void EmptyCaches() noexcept;

    /// @brief Adds the faces of an `@font-face` rule (staged until the build completes, during one).
    void AddFontFaceRule(FontFaceRule &fontFaceRule, bool isInitiatingElementInUserAgentShadowTree) noexcept;
    void AddFontPaletteValuesRule(const FontPaletteValuesRule &fontPaletteValuesRule) noexcept;
    void AddFontFeatureValuesRule(const FontFeatureValuesRule &fontFeatureValuesRule) noexcept;

    void BuildStarted() noexcept;
    void BuildCompleted() noexcept;

    void FontCacheInvalidated() noexcept final;

    KRYS_NODISCARD bool IsEmpty() const noexcept;

    void RegisterForInvalidationCallbacks(FontSelectorClient &client) noexcept final;
    void UnregisterForInvalidationCallbacks(FontSelectorClient &client) noexcept final;

    KRYS_NODISCARD bool IsSimpleFontSelectorForDescription() const noexcept final;

    KRYS_NODISCARD bool IsCSSFontSelector() const noexcept final
    {
      return true;
    }

    void IncrementIsComputingRootStyleFont() noexcept
    {
      ++_computingRootStyleFontCount;
    }

    void DecrementIsComputingRootStyleFont() noexcept
    {
      --_computingRootStyleFontCount;
    }

    void UpdateStyleIfNeeded() noexcept;

    /// @brief The document the selector's fonts are for, if it's still alive.
    KRYS_NODISCARD dom::Document *GetDocument() const noexcept
    {
      return _document.get();
    }

    KRYS_NODISCARD CSSFontFaceSet &GetCSSFontFaceSet() const noexcept
    {
      return *_cssFontFaceSet;
    }

    /// @brief The selector as a client of the font faces it creates.
    KRYS_NODISCARD CSSFontFaceClient &FontFaceClient() noexcept
    {
      return _fontFaceClient;
    }

    /// @brief Called by the font face set when a face is added, removed or changed.
    void FontModified() noexcept;

    /// @brief Called when the document's settings change: the generic families may resolve differently.
    void SettingsChanged() noexcept;

  private:
    void DispatchInvalidationCallbacks() noexcept;

    void OpportunisticallyStartFontDataURLLoading(const FontCascadeDescription &description,
                                                  const StringAtom &family) noexcept final;

    KRYS_NODISCARD Maybe<StringAtom> ResolveGenericFamily(const FontDescription &description,
                                                          const StringAtom &family) noexcept;

    KRYS_NODISCARD const FontPaletteValues &
      LookupFontPaletteValues(const StringAtom &familyName,
                              const FontDescription &description) const noexcept;
    KRYS_NODISCARD RefPtr<FontFeatureValues>
      LookupFontFeatureValues(const StringAtom &familyName) const noexcept;
  };
}
