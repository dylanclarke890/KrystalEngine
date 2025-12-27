#pragma once

#include "Krystal.Gfx/ResourceManager.hpp"
#include "Krystal.Gfx.OpenGL/Registries/ImageRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/ImageViewRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Resources/RenderTarget.hpp"
#include "Krystal.Gfx/Registries/IRenderTargetRegistry.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <cassert>

namespace Krys::Gfx::OpenGL
{
  class RenderTargetRegistry : public IRenderTargetRegistry
  {
    NO_COPY_MOVE(RenderTargetRegistry)

    using RenderTargetManager = Gfx::ResourceManager<RenderTarget, RenderTargetHandle>;

  private:
    RenderTargetManager _renderTargets;
    ImageRegistry &_images;
    ImageViewRegistry &_imageViews;
    RenderTargetHandle _screenRenderTarget {0u};

  public:
    RenderTargetRegistry(ImageRegistry &images, ImageViewRegistry &imageViews) noexcept
        : _images(images), _imageViews(imageViews)
    {
    }

    virtual ~RenderTargetRegistry() = default;

    void Startup() override
    {
      // Create the render target that represents the screen framebuffer.
      RenderTarget screenRT = RenderTarget::CreateScreenFramebuffer(0u, 0u);
      _screenRenderTarget = _renderTargets.Add(std::move(screenRT));
    }

    void Shutdown() noexcept override
    {
      // TODO: Destroy all render targets
      // The issue isn't destroying the render targets themselves as they'll be deallocated on destruction,
      // but ensuring that the images and image views are cleaned up correctly, otherwise we're leaking GPU
      // resources.
    }

    KRYS_NODISCARD RenderTargetHandle Create(const RenderTargetDesc &desc) override
    {
      // TODO: support for cubemaps and texture arrays as attachments
      // TODO!: support for multi-sampling
      assert(desc.Width > 0 && desc.Height > 0 && "Invalid width and height.");
      assert(desc.Attachments.size() > 0 && "Must specify at least one attachment.");

      RenderTarget rt(desc.Width, desc.Height);

      bool isDepthOnly = true;
      for (auto &attachmentDesc : desc.Attachments)
      {
        ImageHandle imageHandle = _images.Create({
          .Type = ImageType::Image2D,
          .Format = attachmentDesc.Format,
          .Width = desc.Width,
          .Height = desc.Height,
          .Depth = 1,
          .MipLevels = 1,
          .ArrayLayers = 1,
        });
        Image &image = _images.Get(imageHandle);

        RenderTargetAttachment rta {
          .Type = attachmentDesc.Type,
          .Image = imageHandle,
          .Texture = image.Id(),
        };

        switch (attachmentDesc.Type)
        {
          case AttachmentType::Colour:
            rt.AddColorAttachment(rta);
            isDepthOnly = false;
            break;
          case AttachmentType::Depth:        rt.SetDepthAttachment(rta); break;
          case AttachmentType::Stencil:      rt.SetStencilAttachment(rta); break;
          case AttachmentType::DepthStencil: rt.SetDepthStencilAttachment(rta); break;
          default:                           assert(false && "Unknown attachment type."); break;
        };
      }

      if (isDepthOnly)
      {
        rt.SetDepthOnly();
      }

      rt.Complete();

      return _renderTargets.Add(std::move(rt));
    }

    bool Destroy(RenderTargetHandle handle) noexcept override
    {
      assert(handle.IsValid() && "Invalid render target handle.");
      auto *rt = _renderTargets.TryGet(handle);
      if (rt == nullptr)
      {
        assert(false && "Invalid render target handle.");
        return false;
      }

      for (const auto &attachment : rt->GetAllAttachments())
      {
        _images.Destroy(attachment.Image);
      }

      return _renderTargets.Remove(handle);
    }

    KRYS_NODISCARD Maths::Vec2 GetDimensions(RenderTargetHandle handle) noexcept override
    {
      assert(handle.IsValid() && "Invalid render target handle.");
      auto *rt = _renderTargets.TryGet(handle);
      if (rt == nullptr)
      {
        assert(false && "Invalid render target handle.");
        return {0.f, 0.f};
      }
      return {static_cast<float>(rt->Width()), static_cast<float>(rt->Height())};
    }

    KRYS_NODISCARD ImageHandle GetColourAttachmentImage(RenderTargetHandle handle,
                                                            uint32 index) noexcept override
    {
      assert(handle.IsValid() && "Invalid render target handle.");
      assert(_renderTargets.TryGet(handle) != nullptr && "Render target not found in resource manager.");

      auto &rt = _renderTargets.Get(handle);
      const RenderTargetAttachment &attachment = rt.GetColourAttachment(index);
      return attachment.Image;
    }

    KRYS_NODISCARD RenderTargetHandle GetScreenRenderTarget() const noexcept override
    {
      return _screenRenderTarget;
    }

    void OnWindowResize(uint32 width, uint32 height) noexcept
    {
      _renderTargets.Get(_screenRenderTarget).SetDimensions(width, height);
    }

    KRYS_NODISCARD RenderTarget &Get(RenderTargetHandle handle)
    {
      assert(handle.IsValid() && "Invalid render target handle.");
      return _renderTargets.Get(handle);
    }

    void Bind(RenderTargetHandle handle) noexcept
    {
      assert(handle.IsValid() && "Invalid render target handle.");
      if (handle == _screenRenderTarget)
      {
        // Bind the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
      }
      else
      {
        _renderTargets.Get(handle).Bind();
      }
    }
  };
}