#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML::Parser
{
  enum class InsertionMode : uint8
  {
    Initial,
    BeforeHTML,
    BeforeHead,
    InHead,
    InHeadNoscript,
    AfterHead,
    InBody,
    Text,
    InTable,
    InTableText,
    InCaption,
    InColumnGroup,
    InTableBody,
    InRow,
    InCell,
    InSelect,
    InSelectInTable,
    InTemplate,
    AfterBody,
    InFrameset,
    AfterFrameset,
    AfterAfterBody,
    AfterAfterFrameset
  };
}

namespace Krys
{
  template <>
  constexpr underlying_t<HTML::Parser::InsertionMode> OrdinalCount<HTML::Parser::InsertionMode> = 23;
}