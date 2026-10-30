#pragma once

#include "Krystal.Booey/DOM/Types/DOMHighResTimeStamp.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::dom
{
  class KeyboardEvent;
}

namespace krys::boo::html
{
  /// @brief The options a TypeAhead searches.
  class TypeAheadDataSource
  {
  public:
    virtual ~TypeAheadDataSource() noexcept = default;

    KRYS_NODISCARD virtual int32 IndexOfSelectedOption() const noexcept = 0;
    KRYS_NODISCARD virtual int32 OptionCount() const noexcept = 0;
    KRYS_NODISCARD virtual dom::DOMString OptionAtIndex(int32 index) const noexcept = 0;
  };

  /// @brief Finds the option the user is typing the start of.
  class TypeAhead
  {
  public:
    enum class MatchMode : uint8
    {
      MatchPrefix,
      CycleFirstChar,
      MatchIndex,
    };

  private:
    TypeAheadDataSource *_dataSource {nullptr};
    dom::DOMHighResTimeStamp _lastTypeTime {};
    char32 _repeatingChar {0};
    utf32_string _buffer;

  public:
    explicit TypeAhead(TypeAheadDataSource *dataSource) noexcept;

    /// @return The index of the matching option, or -1 if none matches.
    KRYS_NODISCARD int32 HandleEvent(dom::KeyboardEvent &event, EnumSet<MatchMode> matchMode) noexcept;

    /// @brief Forgets what was typed, so the next key starts a new search.
    void ResetSession() noexcept
    {
      _lastTypeTime = {};
      _repeatingChar = 0;
      _buffer.clear();
    }
  };
}
