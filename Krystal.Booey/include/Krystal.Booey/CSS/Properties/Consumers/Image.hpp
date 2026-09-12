#pragma once
#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css::CSSPropertyParserHelpers
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

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::CSSPropertyParserHelpers::AllowedImageType, 5uz)

namespace krys::boo::css
{
  class CSSValue;
  class TokenRange;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <image>
    // https://drafts.csswg.org/css-images-4/#image-values

    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeImage(TokenRange &tokens, CSSPropertyParserState &state,
                   AllowedImageType allowedImageTypes = AllowedImageType::URLFunction
                                                        | AllowedImageType::ImageSet
                                                        | AllowedImageType::GeneratedImage) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeImageOrNone(TokenRange &tokens, CSSPropertyParserState &state,
                         AllowedImageType allowedImageTypes = AllowedImageType::URLFunction
                                                              | AllowedImageType::ImageSet
                                                              | AllowedImageType::GeneratedImage) noexcept;
  }
}