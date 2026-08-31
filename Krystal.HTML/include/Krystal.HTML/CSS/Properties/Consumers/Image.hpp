#pragma once
#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  enum class AllowedImageType
  {
    None = 0,
    URLFunction = 1 << 0,
    RawStringAsURL = 1 << 1,
    ImageSet = 1 << 2,
    GeneratedImage = 1 << 3
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::CSSPropertyParserHelpers::AllowedImageType, 5uz)

namespace Krys::HTML
{
  class CSSValue;
  class CSSTokenRange;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <image>
    // https://drafts.csswg.org/css-images-4/#image-values

    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeImage(CSSTokenRange &tokens, CSSPropertyParserState &state,
                   AllowedImageType allowedImageTypes = AllowedImageType::URLFunction
                                                        | AllowedImageType::ImageSet
                                                        | AllowedImageType::GeneratedImage) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeImageOrNone(CSSTokenRange &tokens, CSSPropertyParserState &state,
                         AllowedImageType allowedImageTypes = AllowedImageType::URLFunction
                                                              | AllowedImageType::ImageSet
                                                              | AllowedImageType::GeneratedImage) noexcept;
  }
}