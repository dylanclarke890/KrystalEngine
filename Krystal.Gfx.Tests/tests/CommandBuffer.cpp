#include "Krystal.Gfx/CommandBuffer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Commands/CommandListReader.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <cstring>

namespace krys::gfx::tests
{
  namespace
  {
    KRYS_NODISCARD List<CommandType> Types(const CommandBuffer &commands)
    {
      List<CommandType> types;
      CommandListReader reader(commands.Commands());
      while (reader.HasMore())
      {
        auto header = reader.ReadHeader();
        types.push_back(header.Type);
        reader.SkipBytes(header.SizeInBytes);
      }
      return types;
    }
  }

  TEST_CASE("CommandBuffer: records commands in order", "[Gfx][CommandBuffer]")
  {
    CommandBuffer commands;
    CHECK(commands.IsEmpty());
    CHECK(commands.IsComplete());

    auto target = TextureHandle::Create(3u, 1u);
    commands.PushDebugGroup(u8"Frame");
    commands.BeginRenderPass(
      RenderPassDesc::SingleColour({.Texture = target, .ClearColour = {1.f, 0.f, 0.f, 1.f}}));
    CHECK_FALSE(commands.IsComplete());
    commands.SetViewport(0.f, 0.f, 64.f, 32.f);
    commands.BindPipeline(PipelineHandle::Create(0u, 0u));
    commands.BindVertexBuffer(1u, BufferHandle::Create(2u, 0u), 16u);
    commands.BindIndexBuffer(BufferHandle::Create(4u, 0u), IndexType::UInt16);
    commands.DrawIndexed(6u, 2u, 3u, -1, 0u);
    commands.EndRenderPass();
    commands.PopDebugGroup();
    CHECK(commands.IsComplete());

    CHECK(Types(commands)
          == List<CommandType> {
            commands::PushDebugGroup::Type, commands::BeginRenderPass::Type, commands::SetViewport::Type,
            commands::BindPipeline::Type, commands::BindVertexBuffer::Type, commands::BindIndexBuffer::Type,
            commands::DrawIndexed::Type, commands::EndRenderPass::Type, commands::PopDebugGroup::Type});

    CommandListReader reader(commands.Commands());
    static_cast<void>(reader.ReadHeader());
    const auto &group = reader.ReadCommand<commands::PushDebugGroup>();
    CHECK(std::strcmp(group.Name.data(), "Frame") == 0);

    static_cast<void>(reader.ReadHeader());
    const auto &pass = reader.ReadCommand<commands::BeginRenderPass>();
    CHECK(pass.ColourAttachmentCount == 1u);
    CHECK(pass.ColourAttachments[0].Texture == target);
    CHECK(pass.ColourAttachments[0].ClearColour[0] == 1.f);

    static_cast<void>(reader.ReadHeader());
    const auto &viewport = reader.ReadCommand<commands::SetViewport>();
    CHECK(viewport.Width == 64.f);
    CHECK(viewport.MaxDepth == 1.f);

    static_cast<void>(reader.ReadHeader());
    static_cast<void>(reader.ReadCommand<commands::BindPipeline>());
    static_cast<void>(reader.ReadHeader());
    const auto &vertexBuffer = reader.ReadCommand<commands::BindVertexBuffer>();
    CHECK(vertexBuffer.Slot == 1u);
    CHECK(vertexBuffer.Offset == 16u);

    static_cast<void>(reader.ReadHeader());
    const auto &indexBuffer = reader.ReadCommand<commands::BindIndexBuffer>();
    CHECK(indexBuffer.IndexType == IndexType::UInt16);

    static_cast<void>(reader.ReadHeader());
    const auto &draw = reader.ReadCommand<commands::DrawIndexed>();
    CHECK(draw.IndexCount == 6u);
    CHECK(draw.InstanceCount == 2u);
    CHECK(draw.FirstIndex == 3u);
    CHECK(draw.VertexOffset == -1);
  }

  TEST_CASE("CommandBuffer: push constants and reset", "[Gfx][CommandBuffer]")
  {
    struct Constants
    {
      float Values[4];
    };

    CommandBuffer commands;
    commands.PushConstants(Constants {{1.f, 2.f, 3.f, 4.f}});

    CommandListReader reader(commands.Commands());
    CHECK(reader.ReadHeader().Type == commands::PushConstants::Type);
    const auto &pushed = reader.ReadCommand<commands::PushConstants>();
    REQUIRE(pushed.Size == sizeof(Constants));
    Constants read {};
    std::memcpy(&read, pushed.Data.data(), sizeof(Constants));
    CHECK(read.Values[3] == 4.f);

    commands.Reset();
    CHECK(commands.IsEmpty());
  }

  TEST_CASE("CommandBuffer: long debug group names are truncated", "[Gfx][CommandBuffer]")
  {
    CommandBuffer commands;
    commands.PushDebugGroup(
      u8"A debug group name that is much longer than the sixty-four bytes a command holds");
    commands.PopDebugGroup();

    CommandListReader reader(commands.Commands());
    static_cast<void>(reader.ReadHeader());
    const auto &group = reader.ReadCommand<commands::PushDebugGroup>();
    CHECK(std::strlen(group.Name.data()) == 63u);
  }
}
