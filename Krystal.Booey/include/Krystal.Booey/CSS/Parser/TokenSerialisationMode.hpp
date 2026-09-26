#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class TokenSerialisationMode : bool
  {
    Normal,
    // "Specified values of custom properties must be serialized exactly as specified by the author.
    // Simplifications that might occur in other properties, such as dropping comments, normalizing
    // whitespace, reserializing numeric tokens from their value, etc., must not occur."
    // https://drafts.csswg.org/css-variables-2/#serializing-custom-props
    CustomProperty
  };
}