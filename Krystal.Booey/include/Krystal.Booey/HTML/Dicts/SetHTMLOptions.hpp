#pragma once

#include "Krystal.Booey/HTML/Sanitizer/Sanitizer.hpp"
#include "Krystal.Booey/HTML/Sanitizer/SanitizerConfig.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::html
{
  /// @brief How markup is sanitized: by a sanitizer, a configuration, or a preset.
  using SanitizerInput = Variant<Ref<Sanitizer>, SanitizerConfig, SanitizerPresets>;

  /// @see https://html.spec.whatwg.org/#sethtmloptions
  struct SetHTMLOptions
  {
    SanitizerInput Sanitizer {SanitizerPresets::Default};
  };

  /// @see https://html.spec.whatwg.org/#sethtmlunsafeoptions
  struct SetHTMLUnsafeOptions
  {
    Maybe<SanitizerInput> Sanitizer;
  };

  /// @see https://html.spec.whatwg.org/#parsehtmlunsafeoptions
  struct ParseHTMLUnsafeOptions
  {
    Maybe<SanitizerInput> Sanitizer;
  };
}
