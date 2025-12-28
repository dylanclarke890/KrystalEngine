#pragma once

#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::HTML::DOM
{
  struct EncodingRegistry : NonCopyMovable<EncodingRegistry>
  {
  private:
    using LabelToEncodingMap = Map<utf8_string, Encoding *>;

    static inline LabelToEncodingMap _labelToEncodingMap;

  public:
    EncodingRegistry() = delete;
    ~EncodingRegistry() = delete;
  };
}