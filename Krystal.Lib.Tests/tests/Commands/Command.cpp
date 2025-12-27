#include "Krystal.Lib/Commands/CommandArena.hpp"
#include "Krystal.Lib/Commands/CommandList.hpp"
#include "Krystal.Lib/Commands/CommandListReader.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys;

  struct Command
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("TestCommand");
    float X {};
    float Y {};
  };

  TEST_CASE("Command - Read and write single command", "[Gfx][Command]")
  {
    CommandList commandList(100);
    commandList.Push<Command>({10.f, 20.f});

    REQUIRE(commandList.SizeInBytes() == 16u);

    CommandListReader reader(commandList);
    auto _ = reader.ReadHeader();
    const auto &cmd = reader.ReadCommand<Command>();

    REQUIRE(cmd.X == 10.f);
    REQUIRE(cmd.Y == 20.f);
    REQUIRE_FALSE(reader.HasMore());
  }
}