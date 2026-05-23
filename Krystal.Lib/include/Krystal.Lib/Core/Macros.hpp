#pragma once

#define KRYS_CONCAT_IMPL(x, y) x##y
#define KRYS_CONCAT(x, y) KRYS_CONCAT_IMPL(x, y)

#define KRYS_STRINGIFY_IMPL(x) #x
#define KRYS_STRINGIFY(x) KRYS_STRINGIFY_IMPL(x)
