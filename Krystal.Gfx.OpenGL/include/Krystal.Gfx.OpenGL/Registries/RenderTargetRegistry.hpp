#pragma once

#include "Krystal.Gfx.Lib/ResourceManager.hpp"
#include "Krystal.Gfx.OpenGL/Registries/ImageRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/ImageViewRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Resources/RenderTarget.hpp"
#include "Krystal.Gfx/Registries/IRenderTargetRegistry.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include <cassert>

namespace Krys::Gfx::OpenGL
{
  class RenderTargetRegistry : public IRenderTargetRegistry
  {
    NO_COPY_MOVE(RenderTargetRegistry)

    using RenderTargetManager = Gfx::ResourceManager<RenderTarget, RenderTargetHandle>;

  private:
    struct CachedTarget
    {
      RenderTargetHandle Handle;
      uint32 Width;
      uint32 Height;
      PixelFormat Format;
      bool InUse;
    };

    RenderTargetManager _renderTargets;
    ImageRegistry &_images;
    ImageViewRegistry &_imageViews;
    RenderTargetHandle _screenRenderTarget {0u};
    List<CachedTarget> _cachedTargets; // TODO: use a better structure for caching

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

    NO_DISCARD RenderTargetHandle Create(const RenderTargetDesc &desc) override
    {
      // TODO: support for cubemaps and texture arrays as attachments
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

        ImageViewHandle imageViewHandle = _imageViews.Create({
          .Image = imageHandle,
          .Target = ImageType::Image2D,
          .Format = attachmentDesc.Format,
          .SubResourceRange = {.BaseMipLevel = 0, .MipLevelCount = 1, .BaseArrayLayer = 0, .LayerCount = 1},
        });

        auto &imageView = _imageViews.Get(imageViewHandle);
        RenderTargetAttachment rta {
          .Type = attachmentDesc.Type,
          .Image = imageHandle,
          .ImageView = imageViewHandle,
          .Texture = imageView.Id(),
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

    NO_DISCARD RenderTargetHandle Acquire(const RenderTargetDesc &desc) override
    {
      for (auto &cachedTarget : _cachedTargets)
      {
        if (!cachedTarget.InUse && cachedTarget.Width == desc.Width && cachedTarget.Height == desc.Height
            && cachedTarget.Format == desc.Attachments[0].Format)
        {
          cachedTarget.InUse = true;
          return cachedTarget.Handle;
        }
      }

      RenderTargetHandle handle = Create(desc);
      _cachedTargets.push_back(CachedTarget {
        .Handle = handle,
        .Width = desc.Width,
        .Height = desc.Height,
        .Format = desc.Attachments[0].Format,
        .InUse = true,
      });

      return handle;
    }

    void Release(RenderTargetHandle handle) noexcept override
    {
      for (auto &cachedTarget : _cachedTargets)
      {
        if (cachedTarget.Handle == handle)
        {
          cachedTarget.InUse = false;
          return;
        }
      }
      assert(false && "Attempted to release a render target that was not acquired.");
    }

    void ClearCache() noexcept override
    {
      for (auto &cachedTarget : _cachedTargets)
      {
        Destroy(cachedTarget.Handle);
      }
      _cachedTargets.clear();
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
        _imageViews.Destroy(attachment.ImageView);
        _images.Destroy(attachment.Image);
      }

      return _renderTargets.Remove(handle);
    }

    Maths::Vec2 GetDimensions(RenderTargetHandle handle) noexcept override
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

    NO_DISCARD RenderTargetHandle GetScreenRenderTarget() const noexcept override
    {
      return _screenRenderTarget;
    }

    void OnWindowResize(uint32 width, uint32 height) noexcept
    {
      _renderTargets.Get(_screenRenderTarget).SetDimensions(width, height);
    }

    NO_DISCARD RenderTarget &Get(RenderTargetHandle handle)
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