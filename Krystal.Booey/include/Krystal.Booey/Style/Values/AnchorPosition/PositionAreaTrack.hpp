#pragma once

namespace krys::boo::style
{
  // Specifies which tracks(s) on the axis that the position-area span occupies.
  // Represented as 3 bits: start track, center track, end track.
  enum class PositionAreaTrack : uint8
  {
    Start = 0b001,     // First track.
    SpanStart = 0b011, // First and center tracks.
    End = 0b100,       // Last track.
    SpanEnd = 0b110,   // Center and last track.
    Center = 0b010,    // Center track.
    SpanAll = 0b111,   // All tracks along the axis.
  };

  KRYS_NODISCARD constexpr PositionAreaTrack FlipPositionAreaTrack(PositionAreaTrack track) noexcept
  {
    // We need to cast values out of the enum type restrictions in order to do math.
    auto trackBits = static_cast<uint8>(track);
    constexpr uint8 startBit = static_cast<uint8>(PositionAreaTrack::Start);
    constexpr uint8 endBit = static_cast<uint8>(PositionAreaTrack::End);
    constexpr uint8 sideBits = startBit | endBit;

    bool isSymmetric = !(trackBits & startBit) == !(trackBits & endBit);
    auto invertedValue = isSymmetric ? trackBits
                                     // Flip side bits and merge with not-side bits.
                                     : ((trackBits & sideBits) ^ sideBits) | (trackBits & ~sideBits);

    return static_cast<PositionAreaTrack>(invertedValue);
  }
}
