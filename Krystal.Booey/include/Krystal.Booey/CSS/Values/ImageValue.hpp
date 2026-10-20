#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/URL.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class CachedImage;
  class CachedResourceLoader;
  class RenderElement;
  class StyleImage;
  struct ResourceLoaderOptions;
}

namespace krys::boo::style
{
  class BuilderState;
}

namespace krys::boo::css
{
  /// @brief An image referenced by a `url()`.
  class ImageValue final : public Value
  {
  private:
    CSSURL _location;
    CachedResourceHandle<CachedImage> _cachedImage;
    // Set once the image is asked for, even if it couldn't be (a null handle).
    bool _didRequestImage {false};
    CSSOMStringAtom _initiatorType {CSSOMStringAtom::Null()};
    RefPtr<ImageValue> _unresolvedValue;
    bool _isInvalid {false};

    ImageValue() noexcept;
    ImageValue(CSSURL &&location, CSSOMStringAtom &&initiatorType) noexcept;

  public:
    ~ImageValue() noexcept;

    KRYS_NODISCARD static Ref<ImageValue> Create() noexcept;
    KRYS_NODISCARD static Ref<ImageValue>
      Create(CSSURL location, CSSOMStringAtom initiatorType = CSSOMStringAtom::Null()) noexcept;
    KRYS_NODISCARD static Ref<ImageValue>
      Create(url::URL imageURL, CSSOMStringAtom initiatorType = CSSOMStringAtom::Null()) noexcept;

    KRYS_NODISCARD Ref<ImageValue> CopyForComputedStyle(const CSSURL &resolvedURL) const noexcept;

    KRYS_NODISCARD bool IsPending() const noexcept;
    KRYS_NODISCARD CachedImage *LoadImage(CachedResourceLoader &loader,
                                          const ResourceLoaderOptions &options) noexcept;
    KRYS_NODISCARD CachedImage *GetCachedImage() const noexcept;

    // Take care when using this, and read https://drafts.csswg.org/css-values/#relative-urls
    KRYS_NODISCARD const CSSURL &URL() const noexcept
    {
      return _location;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool CustomTraverseSubresources(
      KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept;
    KRYS_NODISCARD bool CustomMayDependOnBaseUrl() const noexcept;
    KRYS_NODISCARD bool Equals(const ImageValue &other) const noexcept;
    KRYS_NODISCARD bool KnownToBeOpaque(const RenderElement &renderer) const noexcept;

    KRYS_NODISCARD RefPtr<StyleImage> CreateStyleImage(const style::BuilderState &state) const noexcept;

    KRYS_NODISCARD bool IsLoadedFromOpaqueSource() const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (_unresolvedValue)
      {
        if (func(*_unresolvedValue) == IterationStatus::Done)
        {
          return IterationStatus::Done;
        }
      }
      return IterationStatus::Continue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ImageValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsImageValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
