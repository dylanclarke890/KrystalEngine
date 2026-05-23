#include "Krystal.Text/LineEndings.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("LineEndings(NormalizeToLF)", "[Text][LineEndings]")
  {
    using namespace Krys;
    List<byte> input = {byte {'H'},  byte {'e'},  byte {'l'},  byte {'l'}, byte {'o'},
                        byte {'\r'}, byte {'\n'}, byte {'W'},  byte {'o'}, byte {'r'},
                        byte {'l'},  byte {'d'},  byte {'\r'}, byte {'!'}, byte {'\n'}};
    List<byte> expected = {byte {'H'},  byte {'e'},  byte {'l'}, byte {'l'}, byte {'o'},
                           byte {'\n'}, byte {'W'},  byte {'o'}, byte {'r'}, byte {'l'},
                           byte {'d'},  byte {'\n'}, byte {'!'}, byte {'\n'}};

    List<byte> result = Text::LineEndings::NormalizeToLF(std::move(input));
    REQUIRE(result == expected);
  }

  TEST_CASE("LineEndings(NormalizeToCRLF)", "[Text][LineEndings]")
  {
    using namespace Krys;
    List<byte> input = {byte {'H'},  byte {'e'},  byte {'l'}, byte {'l'}, byte {'o'},
                        byte {'\n'}, byte {'W'},  byte {'o'}, byte {'r'}, byte {'l'},
                        byte {'d'},  byte {'\r'}, byte {'!'}, byte {'\n'}};
    List<byte> expected = {byte {'H'},  byte {'e'},  byte {'l'}, byte {'l'},  byte {'o'}, byte {'\r'},
                           byte {'\n'}, byte {'W'},  byte {'o'}, byte {'r'},  byte {'l'}, byte {'d'},
                           byte {'\r'}, byte {'\n'}, byte {'!'}, byte {'\r'}, byte {'\n'}};

    List<byte> result = Text::LineEndings::NormalizeToCRLF(std::move(input));
    REQUIRE(result == expected);
  }
}