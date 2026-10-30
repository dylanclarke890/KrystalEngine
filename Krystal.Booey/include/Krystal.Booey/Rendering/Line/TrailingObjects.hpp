#pragma once

#include "Krystal.Booey/Rendering/LegacyInlineIterator.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include <functional>

namespace krys::boo
{
  class RenderBoxModelObject;
  class RenderText;

  /// @brief The trailing whitespace on a line and the boxes after it, whose transitions collapse it away.
  class TrailingObjects
  {
  private:
    CheckedPtr<RenderText> _whitespace;
    SmallList<std::reference_wrapper<RenderBoxModelObject>, 4> _boxes;

  public:
    enum class CollapseFirstSpace : bool
    {
      No,
      Yes
    };

    void SetTrailingWhitespace(RenderText &whitespace) noexcept
    {
      _whitespace = ShareCheckedPtr(&whitespace);
    }
    void Clear() noexcept
    {
      _whitespace = nullptr;
      _boxes.clear(); // Keeps our capacity.
    }

    void AppendBoxIfNeeded(RenderBoxModelObject &box) noexcept
    {
      if (_whitespace)
      {
        _boxes.push_back(box);
      }
    }

    void UpdateWhitespaceCollapsingTransitionsForTrailingBoxes(LineWhitespaceCollapsingState &,
                                                               const LegacyInlineIterator &lBreak,
                                                               CollapseFirstSpace) noexcept;
  };
}
