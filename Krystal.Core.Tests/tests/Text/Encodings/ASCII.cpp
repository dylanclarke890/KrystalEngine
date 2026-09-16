#include "Krystal.Core/Text/Encodings/ASCII.hpp"
#include "Krystal.Core/Text/Encodings/Decode.hpp"
#include "Krystal.Core/Text/Encodings/Encode.hpp"
#include <catch_all.hpp>

namespace krys::text::tests
{
  TEST_CASE("Encodings::ASCII", "[Text]")
  {
    SECTION("Decode valid string")
    {
      const string input = "Hello, World!";
      auto result = Decode<ASCII>(input);

      REQUIRE(result.Error == DecodeError::None);
      REQUIRE(result.CodeUnitsConsumed == input.size());
      REQUIRE(result.CodePointsProduced == input.size());
      REQUIRE(result.Output == U"Hello, World!");
    }

    SECTION("Encode valid string")
    {
      const utf32_string input = U"Hello, World!";
      auto result = Encode<ASCII>(input);
      REQUIRE(result.Error == EncodeError::None);
      REQUIRE(result.CodePointsConsumed == input.size());
      REQUIRE(result.CodeUnitsProduced == input.size());
      REQUIRE(result.Output == "Hello, World!");
    }

    SECTION("Decode invalid string")
    {
      SECTION("DecodePolicy::Stop")
      {
        const string input = "Hello, 世界!";
        auto result = Decode<ASCII, DecodePolicy {.OnInvalidSequence = EncodingErrorAction::Stop}>(input);

        REQUIRE(result.Error == DecodeError::InvalidSequence);
        REQUIRE(result.CodeUnitsConsumed == 7uz);
        REQUIRE(result.CodePointsProduced == 7uz);
        REQUIRE(result.Output == U"Hello, ");
      }

      SECTION("DecodePolicy::Skip")
      {
        const string input = "Hello, 世界!";
        auto result = Decode<ASCII, DecodePolicy {.OnInvalidSequence = EncodingErrorAction::Skip}>(input);

        REQUIRE(result.Error == DecodeError::None);
        REQUIRE(result.CodeUnitsConsumed == input.size());
        REQUIRE(result.CodePointsProduced == 8uz);
        REQUIRE(result.Output == U"Hello, !");
      }

      SECTION("DecodePolicy::Replace")
      {
        const string input = "Hello, 世界!";
        auto result = Decode<ASCII, DecodePolicy {.OnInvalidSequence = EncodingErrorAction::Replace}>(input);

        REQUIRE(result.Error == DecodeError::None);
        REQUIRE(result.CodeUnitsConsumed == input.size());
        REQUIRE(result.CodePointsProduced == 14uz);
        REQUIRE(result.Output == U"Hello, ������!");
      }
    }

    SECTION("Encode invalid string")
    {
      SECTION("EncodePolicy::Stop")
      {
        const utf32_string input = U"Hello, 世界!";
        auto result = Encode<ASCII, EncodePolicy {.OnUnmappableCodePoint = EncodingErrorAction::Stop}>(input);

        REQUIRE(result.Error == EncodeError::UnmappableCodePoint);
        REQUIRE(result.CodePointsConsumed == 7uz);
        REQUIRE(result.CodeUnitsProduced == 7uz);
        REQUIRE(result.Output == "Hello, ");
      }

      SECTION("EncodePolicy::Skip")
      {
        const utf32_string input = U"Hello, 世界!";
        auto result = Encode<ASCII, EncodePolicy {.OnUnmappableCodePoint = EncodingErrorAction::Skip}>(input);

        REQUIRE(result.Error == EncodeError::None);
        REQUIRE(result.CodePointsConsumed == input.size());
        REQUIRE(result.CodeUnitsProduced == 8uz);
        REQUIRE(result.Output == "Hello, !");
      }

      SECTION("EncodePolicy::Replace")
      {
        const utf32_string input = U"Hello, 世界!";
        auto result =
          Encode<ASCII, EncodePolicy {.OnUnmappableCodePoint = EncodingErrorAction::Replace}>(input);

        REQUIRE(result.Error == EncodeError::None);
        REQUIRE(result.CodePointsConsumed == input.size());
        REQUIRE(result.CodeUnitsProduced == 10uz);
        REQUIRE(result.Output == "Hello, ??!");
      }
    }
  }
}