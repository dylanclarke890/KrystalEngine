#pragma once

#include "Krystal.Core/Core.hpp"

#include <cassert>
#include <format>

namespace Krys::Platform
{
  /// @brief The state of a key during an interaction.
  enum class KeyState : uint8
  {
    Pressed,
    Held,
    Released
  };

  /// @brief Supported keyboard keys.
  enum class Key : uint32
  {
    UNKNOWN,
    NUM_0,
    NUM_1,
    NUM_2,
    NUM_3,
    NUM_4,
    NUM_5,
    NUM_6,
    NUM_7,
    NUM_8,
    NUM_9,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    EQUAL,
    MINUS,
    RIGHT_BRACKET,
    LEFT_BRACKET,
    QUOTE,
    SEMI_COLON,
    BACKSLASH,
    COMMA,
    SLASH,
    PERIOD,
    GRAVE,
    KEYPAD_DECIMAL,
    KEYPAD_MULTIPLY,
    KEYPAD_PLUS,
    KEYPAD_CLEAR,
    KEYPAD_DIVIDE,
    KEYPAD_ENTER,
    KEYPAD_MINUS,
    KEYPAD_EQUALS,
    KEYPAD_0,
    KEYPAD_1,
    KEYPAD_2,
    KEYPAD_3,
    KEYPAD_4,
    KEYPAD_5,
    KEYPAD_6,
    KEYPAD_7,
    KEYPAD_8,
    KEYPAD_9,
    RETURN,
    TAB,
    SPACE,
    DEL,
    ESCAPE,
    COMMAND,
    SHIFT,
    CAPS_LOCK,
    OPTION,
    CONTROL,
    RIGHT_SHIFT,
    RIGHT_OPTION,
    RIGHT_CONTROL,
    FUNCTION,
    VOLUME_UP,
    VOLUME_DOWN,
    MUTE,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    HELP,
    HOME,
    FORWARD_DELETE,
    END,
    PAGE_UP,
    PAGE_DOWN,
    LEFT_ARROW,
    RIGHT_ARROW,
    DOWN_ARROW,
    UP_ARROW,
    ENTER
  };
}

#pragma region Helper Macros

#define FOR_EACH_KEY_STATE(func)                                                                             \
  func(Pressed);                                                                                             \
  func(Held);                                                                                                \
  func(Released)

#define FOR_EACH_KEY(func)                                                                                   \
  func(UNKNOWN);                                                                                             \
  func(NUM_0);                                                                                               \
  func(NUM_1);                                                                                               \
  func(NUM_2);                                                                                               \
  func(NUM_3);                                                                                               \
  func(NUM_4);                                                                                               \
  func(NUM_5);                                                                                               \
  func(NUM_6);                                                                                               \
  func(NUM_7);                                                                                               \
  func(NUM_8);                                                                                               \
  func(NUM_9);                                                                                               \
  func(A);                                                                                                   \
  func(B);                                                                                                   \
  func(C);                                                                                                   \
  func(D);                                                                                                   \
  func(E);                                                                                                   \
  func(F);                                                                                                   \
  func(G);                                                                                                   \
  func(H);                                                                                                   \
  func(I);                                                                                                   \
  func(J);                                                                                                   \
  func(K);                                                                                                   \
  func(L);                                                                                                   \
  func(M);                                                                                                   \
  func(N);                                                                                                   \
  func(O);                                                                                                   \
  func(P);                                                                                                   \
  func(Q);                                                                                                   \
  func(R);                                                                                                   \
  func(S);                                                                                                   \
  func(T);                                                                                                   \
  func(U);                                                                                                   \
  func(V);                                                                                                   \
  func(W);                                                                                                   \
  func(X);                                                                                                   \
  func(Y);                                                                                                   \
  func(Z);                                                                                                   \
  func(EQUAL);                                                                                               \
  func(MINUS);                                                                                               \
  func(RIGHT_BRACKET);                                                                                       \
  func(LEFT_BRACKET);                                                                                        \
  func(QUOTE);                                                                                               \
  func(SEMI_COLON);                                                                                          \
  func(BACKSLASH);                                                                                           \
  func(COMMA);                                                                                               \
  func(SLASH);                                                                                               \
  func(PERIOD);                                                                                              \
  func(GRAVE);                                                                                               \
  func(KEYPAD_DECIMAL);                                                                                      \
  func(KEYPAD_MULTIPLY);                                                                                     \
  func(KEYPAD_PLUS);                                                                                         \
  func(KEYPAD_CLEAR);                                                                                        \
  func(KEYPAD_DIVIDE);                                                                                       \
  func(KEYPAD_ENTER);                                                                                        \
  func(KEYPAD_MINUS);                                                                                        \
  func(KEYPAD_EQUALS);                                                                                       \
  func(KEYPAD_0);                                                                                            \
  func(KEYPAD_1);                                                                                            \
  func(KEYPAD_2);                                                                                            \
  func(KEYPAD_3);                                                                                            \
  func(KEYPAD_4);                                                                                            \
  func(KEYPAD_5);                                                                                            \
  func(KEYPAD_6);                                                                                            \
  func(KEYPAD_7);                                                                                            \
  func(KEYPAD_8);                                                                                            \
  func(KEYPAD_9);                                                                                            \
  func(RETURN);                                                                                              \
  func(TAB);                                                                                                 \
  func(SPACE);                                                                                               \
  func(DEL);                                                                                                 \
  func(ESCAPE);                                                                                              \
  func(COMMAND);                                                                                             \
  func(SHIFT);                                                                                               \
  func(CAPS_LOCK);                                                                                           \
  func(OPTION);                                                                                              \
  func(CONTROL);                                                                                             \
  func(RIGHT_SHIFT);                                                                                         \
  func(RIGHT_OPTION);                                                                                        \
  func(RIGHT_CONTROL);                                                                                       \
  func(FUNCTION);                                                                                            \
  func(VOLUME_UP);                                                                                           \
  func(VOLUME_DOWN);                                                                                         \
  func(MUTE);                                                                                                \
  func(F1);                                                                                                  \
  func(F2);                                                                                                  \
  func(F3);                                                                                                  \
  func(F4);                                                                                                  \
  func(F5);                                                                                                  \
  func(F6);                                                                                                  \
  func(F7);                                                                                                  \
  func(F8);                                                                                                  \
  func(F9);                                                                                                  \
  func(F10);                                                                                                 \
  func(F11);                                                                                                 \
  func(F12);                                                                                                 \
  func(F13);                                                                                                 \
  func(F14);                                                                                                 \
  func(F15);                                                                                                 \
  func(F16);                                                                                                 \
  func(F17);                                                                                                 \
  func(F18);                                                                                                 \
  func(F19);                                                                                                 \
  func(F20);                                                                                                 \
  func(HELP);                                                                                                \
  func(HOME);                                                                                                \
  func(FORWARD_DELETE);                                                                                      \
  func(END);                                                                                                 \
  func(PAGE_UP);                                                                                             \
  func(PAGE_DOWN);                                                                                           \
  func(LEFT_ARROW);                                                                                          \
  func(RIGHT_ARROW);                                                                                         \
  func(DOWN_ARROW);                                                                                          \
  func(UP_ARROW);                                                                                            \
  func(ENTER)

#pragma endregion

template <>
struct std::formatter<Krys::Platform::Key>
{
  constexpr auto parse(std::format_parse_context &ctx)
  {
    return ctx.begin();
  }

  auto format(const Krys::Platform::Key &key, std::format_context &ctx) const
  {
    switch (key)
    {
#define CASE(val)                                                                                            \
  case CONCAT(Krys::Platform::Key::, val): return std::format_to(ctx.out(), "{}", STRINGIFY(val))

      FOR_EACH_KEY(CASE);
      default: assert(false); break;

#undef CASE
    }

    return std::format_to(ctx.out(), "{}", static_cast<Krys::uint32>(key));
  }
};

template <>
struct std::formatter<Krys::Platform::KeyState>
{
  constexpr auto parse(std::format_parse_context &ctx)
  {
    return ctx.begin();
  }

  auto format(const Krys::Platform::KeyState &state, std::format_context &ctx) const
  {
    switch (state)
    {
#define CASE(val)                                                                                            \
  case CONCAT(Krys::Platform::KeyState::, val): return std::format_to(ctx.out(), "{}", STRINGIFY(val))

      FOR_EACH_KEY_STATE(CASE);
      default: assert(false); break;

#undef CASE
    }

    return std::format_to(ctx.out(), "{}", static_cast<Krys::uint32>(state));
  }
};