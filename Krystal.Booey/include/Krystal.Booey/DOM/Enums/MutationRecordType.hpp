#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::MutationRecordType, 3uz);