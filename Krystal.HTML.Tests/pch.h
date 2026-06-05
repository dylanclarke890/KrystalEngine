#pragma once

// Catch2 Testing Framework - Most expensive header (22s+ compile time)
#include <catch_all.hpp>

// Standard Library Headers - Expensive template-heavy headers
#include <chrono>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

// Commonly used Krystal.HTML headers
#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/EventTarget.hpp"
#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
