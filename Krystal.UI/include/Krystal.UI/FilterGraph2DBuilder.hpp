#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterEffectApplier.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Gfx/CommandBuffer.hpp"
#include "Krystal.Gfx/Render2D/FilterGraph2D.hpp"

namespace krys::boo
{
  class FilterEffect;
  class ImageBuffer;
}

namespace krys::ui
{
  class ImageBufferTextures;

  /// @brief Booey's accelerated filter backend: applying a filter's effects appends nodes to a Render2D
  /// filter graph, each effect's result holding the index of the node that computes it.
  /// @note Image buffers the graph reads (a filter's source, and the images `feImage` draws) are rendered
  /// into textures as they're read.
  class FilterGraph2DBuilder
  {
    KRYS_NON_COPY_MOVABLE(FilterGraph2DBuilder);

  private:
    ImageBufferTextures &_imageBuffers;
    gfx::CommandBuffer &_commands;
    gfx::FilterGraph2D _graph;
    /// @brief The graph's sources, by the rendering resource identifier of the image buffer each holds.
    HashMap<uint64, uint32> _sources;
    /// @brief The image buffers `feImage` effects drew into, kept until the graph is rendered.
    List<Ref<boo::ImageBuffer>> _images;

  public:
    FilterGraph2DBuilder(ImageBufferTextures &imageBuffers, gfx::CommandBuffer &commands) noexcept;
    ~FilterGraph2DBuilder() noexcept;

    /// @brief The applier of @p effect, which appends its nodes; null for an effect it can't apply.
    KRYS_NODISCARD UniquePtr<boo::FilterEffectApplier>
      CreateApplier(const boo::FilterEffect &effect) noexcept;

    /// @brief Ends the graph with @p result, in sRGB.
    /// @return Whether there's a graph to render.
    KRYS_NODISCARD bool Finish(boo::FilterImage &result) noexcept;

    KRYS_NODISCARD const gfx::FilterGraph2D &Graph() const noexcept
    {
      return _graph;
    }

    /// @brief Appends @p effect's nodes, computing @p result from @p inputs.
    /// @return Whether it could.
    KRYS_NODISCARD bool Apply(const boo::FilterEffect &effect, const boo::Filter &filter,
                              Span<const Ref<boo::FilterImage>> inputs, boo::FilterImage &result) noexcept;

  private:
    /// @brief What reads @p image: the node computing it, or a source holding its image buffer.
    KRYS_NODISCARD Maybe<gfx::FilterInput2D> InputFor(boo::FilterImage &image) noexcept;
    /// @brief A source holding @p buffer over @p region.
    KRYS_NODISCARD Maybe<gfx::FilterInput2D> SourceFor(boo::ImageBuffer &buffer,
                                                       const gfx::PixelRect2D &region) noexcept;
    /// @brief Appends @p node, returning its index.
    uint32 AddNode(gfx::FilterNode2D &&node) noexcept;
  };
}
