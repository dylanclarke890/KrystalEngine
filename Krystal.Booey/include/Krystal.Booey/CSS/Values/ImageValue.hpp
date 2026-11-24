#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Url.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Net/Url.hpp"

namespace krys::boo
{
  class CachedImage;
  class CachedResourceLoader;
  class StyleImage;

  struct ResourceLoaderOptions;
}

namespace krys::boo::style
{
  class BuilderState;
}

namespace krys::boo::css
{
  /// @brief An image named by a `url()`, which loads the first time it's asked for.
  /// @details Copies made for another resolved URL keep a link back to the value they came from, and share
  /// its loaded image: loading through either one fills in both.
  /// @see https://www.w3.org/TR/css-images-4/#url-notation
  class ImageValue final : public Value
  {
  private:
    css::Url _location;
    CachedResourceHandle<CachedImage> _cachedImage;
    /// @brief The value this one was copied from for another resolved URL.
    RefPtr<ImageValue> _unresolvedValue;
    /// @brief What asked for the image, as Resource Timing reports it: `css` unless an element did.
    /// @see https://w3c.github.io/resource-timing/#dom-performanceresourcetiming-initiatortype
    CSSOMStringAtom _initiatorType;
    /// @brief Set once the image has been requested, even if the request failed and left no image.
    bool _didRequestImage {false};
    /// @brief Set for the value standing for an image that can't be shown, which has no URL.
    bool _isInvalid {false};

  public:
    ~ImageValue() noexcept;

    /// @param initiatorType The element that asked for the image, such as `body` for its `background`
    /// attribute; null for `css`.
    KRYS_NODISCARD static Ref<ImageValue> Create(css::Url location,
                                                 CSSOMStringAtom initiatorType = null) noexcept;

    /// @brief An image at an already-resolved URL, which is also its text.
    /// @param initiatorType As for the other `Create`.
    KRYS_NODISCARD static Ref<ImageValue> Create(net::Url url, CSSOMStringAtom initiatorType = null) noexcept;

    /// @brief The computed value of an image that can't be shown; it serialises as nothing.
    KRYS_NODISCARD static Ref<ImageValue> CreateInvalid() noexcept;

    /// @brief This value, or a copy at @p resolvedUrl if that differs from this one's.
    KRYS_NODISCARD Ref<ImageValue> CopyForComputedStyle(const css::Url &resolvedUrl) const noexcept;

    /// @brief The URL as written and as resolved.
    /// @note A relative URL resolves against the style sheet's base URL, which may not be the document's.
    /// @see https://www.w3.org/TR/css-values-4/#relative-urls
    KRYS_NODISCARD const css::Url &Url() const noexcept
    {
      return _location;
    }

    /// @brief Requests the image the first time, with @p options and the URL's modifiers.
    /// @return The image, or null if it couldn't be requested.
    KRYS_NODISCARD CachedImage *LoadImage(CachedResourceLoader &loader,
                                          const ResourceLoaderOptions &options) noexcept;

    /// @brief The image, or null if it hasn't been requested or couldn't be.
    KRYS_NODISCARD CachedImage *GetCachedImage() const noexcept;

    /// @brief The style image for this value, at its URL resolved against @p state's document.
    KRYS_NODISCARD RefPtr<StyleImage> CreateStyleImage(const style::BuilderState &state) const noexcept;

    // The Value operations.

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    /// @brief Whether both have the same URL, with the same modifiers.
    KRYS_NODISCARD bool Equals(const ImageValue &other) const noexcept;

    KRYS_NODISCARD bool CustomMayDependOnBaseUrl() const noexcept;

    KRYS_NODISCARD bool CustomTraverseSubresources(
      KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      return _unresolvedValue ? func(*_unresolvedValue) : IterationStatus::Continue;
    }

  private:
    ImageValue(css::Url location, CSSOMStringAtom initiatorType) noexcept;

    /// @brief Selects the constructor of `CreateInvalid`.
    struct InvalidTag
    {
    };

    explicit ImageValue(InvalidTag) noexcept;

    /// @brief A copy at @p location that shares this value's image and links back to it.
    KRYS_NODISCARD Ref<ImageValue> CopyWithLocation(css::Url location) const noexcept;

    /// @brief This value for a holder that will load its image, which writes to it.
    KRYS_NODISCARD Ref<ImageValue> ShareForLoading() const noexcept
    {
      return ShareRef(const_cast<ImageValue &>(*this));
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ImageValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.Type() == krys::boo::css::ValueType::Image;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
