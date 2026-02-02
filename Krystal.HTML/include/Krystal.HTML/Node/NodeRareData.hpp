#pragma once

#include "Krystal.HTML/Node/ChildNodeList.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include "Krystal.Lib/Types/Lazy.hpp"

namespace Krys::HTML
{
  class Node;

  struct NodeRareData
  {
    LazyUnique<ChildNodeList> ChildNodes {};
  };
}