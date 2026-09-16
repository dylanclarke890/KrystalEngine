#include "Krystal.Core/Commands/CommandArena.hpp"
#include "Krystal.Core/Commands/CommandList.hpp"
#include "Krystal.Core/Commands/CommandListReader.hpp"
#include <catch_all.hpp>

namespace krys::tests
{
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