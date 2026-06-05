#pragma once

// Catch2 Testing Framework - Most expensive header (23s+ compile time)
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

#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Vector.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
