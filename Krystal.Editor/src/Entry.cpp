#include "Krystal.Core/Detection.hpp"
#include "Krystal.Editor/Editor.hpp"
#include "Krystal.Engine/Application.hpp"

#ifdef KRYS_PLATFORM_WINDOWS
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <windows.h>
#endif

int main(int argc, char **argv)
{
  using namespace Krys;

  ApplicationSettings settings {};
  settings.Name = "Krystal Editor";

  settings.WindowSettings = {};
  settings.WindowSettings.Title = "Krystal Editor";
  settings.WindowSettings.VSync = false;
  settings.WindowSettings.Width = 1'920;
  settings.WindowSettings.Height = 1'080;

  auto result = Application::Create<Editor>(argc, argv, settings);
  if (!result.has_value())
  {
#ifdef KRYS_PLATFORM_WINDOWS
    ::MessageBoxA(nullptr, result.error().c_str(), NULL, MB_OK | MB_ICONERROR | MB_DEFAULT_DESKTOP_ONLY);
#endif
    return -1;
  }

  Unique<Editor> editor = std::move(result.value());
  editor->Run();
}