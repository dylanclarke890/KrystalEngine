#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief What the legacy line layout knows of the line it's building.
  class LineInfo
  {
  private:
    bool _isFirstLine {true};
    bool _isLastLine {false};
    bool _isEmpty {true};
    uint32 _runsFromLeadingWhitespace {0};

  public:
    LineInfo() noexcept = default;

    KRYS_NODISCARD bool IsFirstLine() const noexcept
    {
      return _isFirstLine;
    }
    KRYS_NODISCARD bool IsLastLine() const noexcept
    {
      return _isLastLine;
    }
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _isEmpty;
    }
    KRYS_NODISCARD uint32 RunsFromLeadingWhitespace() const noexcept
    {
      return _runsFromLeadingWhitespace;
    }
    void ResetRunsFromLeadingWhitespace() noexcept
    {
      _runsFromLeadingWhitespace = 0;
    }
    void IncrementRunsFromLeadingWhitespace() noexcept
    {
      _runsFromLeadingWhitespace++;
    }

    void SetFirstLine(bool firstLine) noexcept
    {
      _isFirstLine = firstLine;
    }
    void SetLastLine(bool lastLine) noexcept
    {
      _isLastLine = lastLine;
    }
    void SetEmpty(bool empty) noexcept
    {
      _isEmpty = empty;
    }
  };
}
