#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  namespace url
  {
    class URL;
  }

  class Font;
  class FontCreationContext;
  class FontDescription;
  class FontLoadRequest;

  /// @brief Something told when a font load request's font has loaded (or failed).
  class FontLoadRequestClient : public AbstractRefCountedAndCanMakeWeakPtr<FontLoadRequestClient>
  {
  public:
    ~FontLoadRequestClient() noexcept override = default;

    virtual void FontLoaded(FontLoadRequest &) noexcept
    {
    }
  };

  /// @brief A request for a web font's file, and the fonts made from it once it has loaded.
  class FontLoadRequest : public AbstractRefCounted
  {
  public:
    ~FontLoadRequest() noexcept override = default;

    KRYS_NODISCARD virtual const url::URL &URL() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsPending() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsLoading() const noexcept = 0;
    KRYS_NODISCARD virtual bool ErrorOccurred() const noexcept = 0;

    /// @brief Makes the font from the loaded file, if it hasn't been made yet.
    /// @return Whether there's a font.
    virtual bool EnsureCustomFontData() noexcept = 0;
    KRYS_NODISCARD virtual RefPtr<Font> CreateFont(const FontDescription &, bool syntheticBold,
                                                   bool syntheticItalic,
                                                   const FontCreationContext &) noexcept = 0;

    virtual void SetClient(FontLoadRequestClient *) noexcept = 0;

    KRYS_NODISCARD virtual bool IsCachedFontLoadRequest() const noexcept
    {
      return false;
    }
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_FONT_LOAD_REQUEST(ToClassName, Predicate)                                \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ToClassName)                                             \
    KRYS_NODISCARD static bool IsType(const krys::boo::FontLoadRequest &request) noexcept                    \
    {                                                                                                        \
      return request.Predicate;                                                                              \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
