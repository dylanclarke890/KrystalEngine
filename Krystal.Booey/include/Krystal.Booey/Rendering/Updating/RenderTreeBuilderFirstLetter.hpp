#pragma once

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderText;
  class RenderTextFragment;

  /// @brief Builds the `::first-letter` renderers of blocks: splits the first text into a fragment for the
  /// first letter, inside a renderer with the first-letter style, and a fragment for the rest.
  class RenderTreeBuilder::FirstLetter
  {
    KRYS_NON_COPY_MOVABLE(FirstLetter);

  private:
    RenderTreeBuilder &_builder;

  public:
    explicit FirstLetter(RenderTreeBuilder &builder) noexcept;

    void UpdateAfterDescendants(RenderBlock &block) noexcept;
    void CleanupOnDestroy(RenderTextFragment &textFragment) noexcept;

  private:
    void UpdateStyle(RenderBlock &firstLetterBlock, RenderObject &currentChild) noexcept;
    void CreateRenderers(RenderText &currentTextChild) noexcept;
  };
}
