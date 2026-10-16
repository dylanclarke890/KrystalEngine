#pragma once

#include "Krystal.Booey/Graphics/FontSelector.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
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
  /// @brief A document's font selector: resolves the generic families with the document's settings and finds
  /// the fonts of the families its styles name.
  /// @note FIXME: No web fonts: `@font-face` (the font face set, `FontFace`, loading),
  /// `@font-palette-values` and `@font-feature-values` come with resource loading.
  class CSSFontSelector final : public FontSelector
  {
    KRYS_NON_COPY_MOVABLE(CSSFontSelector);

  private:
    WeakPtr<dom::Document> _document;
    List<FontSelectorClient *> _clients;
    uint32 _uniqueId;
    uint32 _version {0};
    uint32 _computingRootStyleFontCount {0};
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

    /// @brief Called when the document's settings change: the generic families may resolve differently.
    void SettingsChanged() noexcept;

  private:
    void DispatchInvalidationCallbacks() noexcept;

    void OpportunisticallyStartFontDataURLLoading(const FontCascadeDescription &description,
                                                  const StringAtom &family) noexcept final;

    KRYS_NODISCARD Maybe<StringAtom> ResolveGenericFamily(const FontDescription &description,
                                                          const StringAtom &family) noexcept;
  };
}
