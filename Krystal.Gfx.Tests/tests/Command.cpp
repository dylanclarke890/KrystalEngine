#include "Krystal.Gfx/Commands/CommandArena.hpp"
#include "Krystal.Gfx/Commands/CommandList.hpp"
#include "Krystal.Gfx/Commands/CommandListReader.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Gfx;

  struct Command
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("TestCommand");
    float X {};
    float Y {};
  };

  TEST_CASE("Command - Read and write single command", "[Gfx][Command]")
  {
    CommandArena arena;
    CommandList commandList(arena);
    commandList.Push<Command>({10.f, 20.f});

    REQUIRE(arena.Size() == 16u);
    REQUIRE(commandList.SizeInBytes() == 16u);

    CommandListReader reader(commandList);
    auto _ = reader.ReadHeader();
    const auto &cmd = reader.ReadCommand<Command>();

    REQUIRE(cmd.X == 10.f);
    REQUIRE(cmd.Y == 20.f);
    REQUIRE_FALSE(reader.HasMore());
  }
}