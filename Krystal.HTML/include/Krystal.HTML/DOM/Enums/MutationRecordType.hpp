#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  /// @brief The type of a mutation record, which is used to indicate what type of mutation occurred in the
  /// DOM.
  enum class MutationRecordType : uint8
  {
    ChildList,
    Attributes,
    CharacterData,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::MutationRecordType, 3uz);