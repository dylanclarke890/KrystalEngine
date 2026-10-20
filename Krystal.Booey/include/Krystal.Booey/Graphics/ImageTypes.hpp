#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  enum class SubsamplingLevel : uint8
  {
    First = 0,
    Default = First,
    Level0 = First,
    Level1,
    Level2,
    Level3,
    Last = Level3,
    Max
  };

  using RepetitionCount = int32;

  enum : int32
  {
    RepetitionCountNone = 0,
    RepetitionCountOnce = 1,
    RepetitionCountInfinite = -1,
  };

  enum class AlphaOption : uint8
  {
    Premultiplied,
    NotPremultiplied
  };

  enum class GammaAndColorProfileOption : uint8
  {
    Applied,
    Ignored
  };

  enum class ImageAnimatingState : bool
  {
    No,
    Yes
  };

  enum class EncodedDataStatus : uint8
  {
    Error,
    Unknown,
    TypeAvailable,
    SizeAvailable,
    Complete
  };

  enum class DecodingStatus : uint8
  {
    Invalid,
    Partial,
    Complete,
    Decoding
  };

  enum class ShowDebugBackground : bool
  {
    No,
    Yes
  };

  enum class AllowImageSubsampling : bool
  {
    No,
    Yes
  };
}
