#include "Krystal.Gfx/CommandBuffer.hpp"
#include "Krystal.Core/Assert.hpp"
#include <cstring>

namespace krys::gfx
{
  void CommandBuffer::BeginRenderPass(const RenderPassDesc &desc) noexcept
  {
    krys_assert_msg(!_inRenderPass, "Render passes can't be nested.");
    krys_assert_msg(desc.ColourAttachmentCount <= MaxColourAttachments, "Too many colour attachments.");
    krys_assert_msg(desc.ColourAttachmentCount > 0u || desc.DepthStencil.Texture.IsValid(),
                    "A render pass needs at least one target.");

    _commands.Push(commands::BeginRenderPass {.ColourAttachments = desc.ColourAttachments,
                                              .ColourAttachmentCount = desc.ColourAttachmentCount,
                                              .DepthStencil = desc.DepthStencil});
    _inRenderPass = true;
    _hasPipeline = false;
  }

  void CommandBuffer::EndRenderPass() noexcept
  {
    krys_assert_msg(_inRenderPass, "No render pass to end.");
    _commands.Push(commands::EndRenderPass {});
    _inRenderPass = false;
    _hasPipeline = false;
  }

  void CommandBuffer::SetViewport(float x, float y, float width, float height, float minDepth,
                                  float maxDepth) noexcept
  {
    krys_assert_msg(_inRenderPass, "The viewport is set inside a render pass.");
    _commands.Push(commands::SetViewport {x, y, width, height, minDepth, maxDepth});
  }

  void CommandBuffer::SetScissor(int32 x, int32 y, uint32 width, uint32 height) noexcept
  {
    krys_assert_msg(_inRenderPass, "The scissor is set inside a render pass.");
    _commands.Push(commands::SetScissor {x, y, width, height});
  }

  void CommandBuffer::SetBlendConstant(float red, float green, float blue, float alpha) noexcept
  {
    _commands.Push(commands::SetBlendConstant {.Constant = {red, green, blue, alpha}});
  }

  void CommandBuffer::SetStencilReference(uint32 reference) noexcept
  {
    _commands.Push(commands::SetStencilReference {reference});
  }

  void CommandBuffer::BindPipeline(PipelineHandle pipeline) noexcept
  {
    krys_assert_msg(_inRenderPass, "Pipelines are bound inside a render pass.");
    krys_assert_msg(pipeline.IsValid(), "Invalid pipeline.");
    _commands.Push(commands::BindPipeline {pipeline});
    _hasPipeline = true;
  }

  void CommandBuffer::BindVertexBuffer(uint32 slot, BufferHandle buffer, uint64 offset) noexcept
  {
    krys_assert_msg(buffer.IsValid(), "Invalid buffer.");
    _commands.Push(commands::BindVertexBuffer {slot, buffer, offset});
  }

  void CommandBuffer::BindIndexBuffer(BufferHandle buffer, IndexType type, uint64 offset) noexcept
  {
    krys_assert_msg(buffer.IsValid(), "Invalid buffer.");
    _commands.Push(commands::BindIndexBuffer {buffer, type, offset});
  }

  void CommandBuffer::BindUniformBuffer(uint32 slot, BufferHandle buffer, uint64 offset, uint64 size) noexcept
  {
    krys_assert_msg(buffer.IsValid(), "Invalid buffer.");
    _commands.Push(commands::BindUniformBuffer {slot, buffer, offset, size});
  }

  void CommandBuffer::BindTexture(uint32 slot, TextureHandle texture, SamplerHandle sampler) noexcept
  {
    krys_assert_msg(texture.IsValid() && sampler.IsValid(), "Invalid texture or sampler.");
    _commands.Push(commands::BindTexture {slot, texture, sampler});
  }

  void CommandBuffer::Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex,
                           uint32 firstInstance) noexcept
  {
    krys_assert_msg(_inRenderPass && _hasPipeline, "Draws need a render pass and a pipeline.");
    _commands.Push(commands::Draw {vertexCount, instanceCount, firstVertex, firstInstance});
  }

  void CommandBuffer::DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex,
                                  int32 vertexOffset, uint32 firstInstance) noexcept
  {
    krys_assert_msg(_inRenderPass && _hasPipeline, "Draws need a render pass and a pipeline.");
    _commands.Push(
      commands::DrawIndexed {indexCount, instanceCount, firstIndex, vertexOffset, firstInstance});
  }

  void CommandBuffer::CopyBufferToTexture(BufferHandle source, uint64 sourceOffset, TextureHandle destination,
                                          const TextureRegion &region) noexcept
  {
    krys_assert_msg(!_inRenderPass, "Copies happen outside render passes.");
    _commands.Push(commands::CopyBufferToTexture {source, sourceOffset, destination, region});
  }

  void CommandBuffer::BlitTexture(TextureHandle source, const TextureRegion &sourceRegion,
                                  TextureHandle destination, const TextureRegion &destinationRegion,
                                  SamplerFilter filter) noexcept
  {
    krys_assert_msg(!_inRenderPass, "Blits happen outside render passes.");
    _commands.Push(commands::BlitTexture {source, sourceRegion, destination, destinationRegion, filter});
  }

  void CommandBuffer::PushDebugGroup(utf8_stringview name) noexcept
  {
    commands::PushDebugGroup command {.Name = {}};
    size_t length = name.size() < command.Name.size() - 1u ? name.size() : command.Name.size() - 1u;
    std::memcpy(command.Name.data(), name.data(), length);
    _commands.Push(command);
    ++_debugGroupDepth;
  }

  void CommandBuffer::PopDebugGroup() noexcept
  {
    krys_assert_msg(_debugGroupDepth > 0u, "No debug group to pop.");
    _commands.Push(commands::PopDebugGroup {});
    --_debugGroupDepth;
  }

  void CommandBuffer::Reset() noexcept
  {
    _commands.Clear();
    _inRenderPass = false;
    _hasPipeline = false;
    _debugGroupDepth = 0u;
  }
}
