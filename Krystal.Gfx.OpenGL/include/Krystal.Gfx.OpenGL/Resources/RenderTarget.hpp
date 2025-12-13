#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Resources/RenderTarget.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Clipspace.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include <cassert>

namespace Krys::Gfx::OpenGL
{
  struct RenderTargetAttachment
  {
    AttachmentType Type {};
    ImageHandle Image {0u};
    ImageViewHandle ImageView {0u};
    GLuint Texture {0u};
  };

  class RenderTarget
  {
    NO_COPY(RenderTarget)

    uint32 _width {0u};
    uint32 _height {0u};
    GLuint _fbo {0u};
    List<RenderTargetAttachment> _colorAttachments;
    RenderTargetAttachment _depthAttachment {};
    RenderTargetAttachment _stencilAttachment {};
    RenderTargetAttachment _depthStencilAttachment {};

    RenderTarget() = default;

  public:
    MOVE_SWAP(RenderTarget)

    RenderTarget(uint32 width, uint32 height) noexcept : _width(width), _height(height)
    {
      glCreateFramebuffers(1, &_fbo);
    }

    ~RenderTarget() noexcept
    {
      glDeleteFramebuffers(1, &_fbo);
    }

    NO_DISCARD static RenderTarget CreateScreenFramebuffer(uint32 width, uint32 height) noexcept
    {
      RenderTarget rt;
      rt.SetDimensions(width, height);
      return rt;
    }

    void AddColorAttachment(const RenderTargetAttachment &attachment) noexcept
    {
      glNamedFramebufferTexture(_fbo, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(_colorAttachments.size()),
                                attachment.Texture, 0);
      _colorAttachments.push_back(attachment);
    }

    void SetDepthAttachment(const RenderTargetAttachment &attachment) noexcept
    {
      assert(_depthStencilAttachment.Texture == 0 && "Can't have both depth and depth-stencil attachments");
      glNamedFramebufferTexture(_fbo, GL_DEPTH_ATTACHMENT, attachment.Texture, 0);
      _depthAttachment = attachment;
    }

    void SetStencilAttachment(const RenderTargetAttachment &attachment) noexcept
    {
      assert(_depthStencilAttachment.Texture == 0 && "Can't have both stencil and depth-stencil attachments");
      glNamedFramebufferTexture(_fbo, GL_STENCIL_ATTACHMENT, attachment.Texture, 0);
      _stencilAttachment = attachment;
    }

    void SetDepthStencilAttachment(const RenderTargetAttachment &attachment) noexcept
    {
      assert(_depthAttachment.Texture == 0 && _stencilAttachment.Texture == 0
             && "Can't have both stencil/depth and depth-stencil attachments");
      glNamedFramebufferTexture(_fbo, GL_DEPTH_STENCIL_ATTACHMENT, attachment.Texture, 0);
      _depthStencilAttachment = attachment;
    }

    void SetDepthOnly() const noexcept
    {
      glNamedFramebufferDrawBuffer(_fbo, GL_NONE);
      glNamedFramebufferReadBuffer(_fbo, GL_NONE);
    }

    void SetDrawBuffers(Span<GLenum> buffers) const noexcept
    {
      glNamedFramebufferDrawBuffers(_fbo, static_cast<GLsizei>(buffers.size()), buffers.data());
    }

    void Bind() const noexcept
    {
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    }

    void BindRead() const noexcept
    {
      glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
    }

    void BindDraw() const noexcept
    {
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    }

    void Unbind() const noexcept
    {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    bool Complete() const
    {
      if (!_colorAttachments.empty())
      {
        List<GLenum> buffers;
        for (uint i = 0; i < _colorAttachments.size(); ++i)
          buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
        glNamedFramebufferDrawBuffers(_fbo, static_cast<GLsizei>(buffers.size()), buffers.data());
      }

      GLenum status = glCheckNamedFramebufferStatus(_fbo, GL_FRAMEBUFFER);
      bool success = status == GL_FRAMEBUFFER_COMPLETE;
      assert(success && "Framebuffer is not complete");
      return success;
    }

    List<RenderTargetAttachment> GetAllAttachments() const noexcept
    {
      List<RenderTargetAttachment> attachments;
      for (const auto &colorAttachment : _colorAttachments)
      {
        attachments.push_back(colorAttachment);
      }
      if (_depthAttachment.Texture != 0)
      {
        attachments.push_back(_depthAttachment);
      }
      if (_stencilAttachment.Texture != 0)
      {
        attachments.push_back(_stencilAttachment);
      }
      if (_depthStencilAttachment.Texture != 0)
      {
        attachments.push_back(_depthStencilAttachment);
      }
      return attachments;
    }

    NO_DISCARD GLuint GetHandle() const noexcept
    {
      return _fbo;
    }

    NO_DISCARD uint32 Width() const noexcept
    {
      return _width;
    }

    NO_DISCARD uint32 Height() const noexcept
    {
      return _height;
    }

    NO_DISCARD const RenderTargetAttachment &GetColourAttachment(size_t index) const
    {
      if (index >= _colorAttachments.size())
      {
        throw std::out_of_range("Color attachment index out of range.");
      }
      return _colorAttachments[index];
    }

    NO_DISCARD const RenderTargetAttachment &GetDepthAttachment() const noexcept
    {
      return _depthAttachment;
    }

    NO_DISCARD const RenderTargetAttachment &GetStencilAttachment() const noexcept
    {
      return _stencilAttachment;
    }

    NO_DISCARD const RenderTargetAttachment &GetDepthStencilAttachment() const noexcept
    {
      return _depthStencilAttachment;
    }

    NO_DISCARD Maths::Mat4 GetProjectionMatrix() const noexcept
    {
      return Maths::Ortho(0.f, static_cast<float>(_width), static_cast<float>(_height), 0.f);
    }

    /// @brief Sets the dimensions of the render target. NOTE: Does not resize attachments.
    void SetDimensions(uint32 width, uint32 height) noexcept
    {
      _width = width;
      _height = height;
    }

  private:
    void Swap(RenderTarget &other)
    {
      std::swap(_width, other._width);
      std::swap(_height, other._height);
      std::swap(_fbo, other._fbo);
      std::swap(_colorAttachments, other._colorAttachments);
      std::swap(_depthAttachment, other._depthAttachment);
      std::swap(_stencilAttachment, other._stencilAttachment);
      std::swap(_depthStencilAttachment, other._depthStencilAttachment);
    }
  };
}