#pragma once

// Standard Library Headers - Expensive template-heavy headers
#include <algorithm>
#include <chrono>
#include <concepts>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Mixins/CanMakeCheckedPtr.hpp"
#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/CheckedPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/EventTarget.hpp"
#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
