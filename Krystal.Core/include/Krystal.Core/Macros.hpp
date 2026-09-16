#pragma once

#define KRYS_CONCAT_IMPL(x, y) x##y
#define KRYS_CONCAT(x, y) KRYS_CONCAT_IMPL(x, y)

#define KRYS_STRINGIFY_IMPL(x) #x
#define KRYS_STRINGIFY(x) KRYS_STRINGIFY_IMPL(x)

#define KRYS_NON_COPYABLE(ClassName)                                                                         \
  ClassName(const ClassName &) = delete;                                                                     \
  ClassName &operator=(const ClassName &) = delete

#define KRYS_NON_MOVABLE(ClassName)                                                                          \
  ClassName(ClassName &&) = delete;                                                                          \
  ClassName &operator=(ClassName &&) = delete

#define KRYS_NON_COPY_MOVABLE(ClassName)                                                                     \
  ClassName(const ClassName &) = delete;                                                                     \
  ClassName &operator=(const ClassName &) = delete;                                                          \
  ClassName(ClassName &&) = delete;                                                                          \
  ClassName &operator=(ClassName &&) = delete