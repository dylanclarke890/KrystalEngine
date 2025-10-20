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
    }

    void Shutdown() noexcept override
    {
      // TODO: Destroy all render targets
      // The issue isn't destroying the render targets themselves as they'll disappear on destruction, but
      // ensuring that the images and image views are cleaned up correctly, otherwise we're leaking GPU
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

    void Destroy(RenderTargetHandle handle) noexcept override
    {
      assert(handle.IsValid() && "Invalid render target handle.");
      auto *rt = _renderTargets.TryGet(handle);
      if (rt == nullptr)
      {
        assert(false && "Invalid render target handle.");
        return;
      }

      for (const auto &attachment : rt->GetAllAttachments())
      {
        _imageViews.Unload(attachment.ImageView);
        _images.Unload(attachment.Image);
      }

      _renderTargets.Remove(handle);
    }

    NO_DISCARD RenderTargetHandle GetScreenRenderTarget() const noexcept override
    {
      return _screenRenderTarget;
    }
  };
}