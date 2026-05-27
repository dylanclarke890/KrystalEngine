#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class MutationRecordType : uint8
  {
    ChildList,
    Attributes,
    CharacterData,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::MutationRecordType, 3uz);