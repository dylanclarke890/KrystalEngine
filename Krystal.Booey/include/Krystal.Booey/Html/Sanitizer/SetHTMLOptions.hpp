#pragma once

#include "Krystal.Booey/Html/Sanitizer/Sanitizer.hpp"
#include "Krystal.Booey/Html/Sanitizer/SanitizerConfig.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::html
{
  /// @brief How markup is sanitized: by a sanitizer, a configuration, or a preset.
  using SanitizerInput = Variant<Ref<Sanitizer>, SanitizerConfig, SanitizerPresets>;

  /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#sethtmloptions
  struct SetHTMLOptions
  {
    SanitizerInput Sanitizer {SanitizerPresets::Default};
  };

  /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#sethtmlunsafeoptions
  struct SetHTMLUnsafeOptions
  {
    Maybe<SanitizerInput> Sanitizer;
  };

  /// @see https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#parsehtmlunsafeoptions
  struct ParseHTMLUnsafeOptions
  {
    Maybe<SanitizerInput> Sanitizer;
  };
}
