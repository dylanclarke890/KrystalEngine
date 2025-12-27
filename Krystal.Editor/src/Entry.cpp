#include "Krystal.Editor/Editor.hpp"
#include "Krystal.Engine/Application.hpp"
#include "Krystal.Lib/Core/Detection.hpp"

#if KRYS_OS(WINDOWS)
  #define WIN32_LEAN_AND_MEAN
  #define NOGDICAPMASKS
  #define NOVIRTUALKEYCODES
  #define NOWINMESSAGES
  #define NOWINSTYLES
  #define NOSYSMETRICS
  #define NOMENUS
  #define NOICONS
  #define NOKEYSTATES
  #define NOSYSCOMMANDS
  #define NORASTEROPS
  #define NOSHOWWINDOW
  #define OEMRESOURCE
  #define NOATOM
  #define NOCLIPBOARD
  #define NOCOLOR
  #define NOCTLMGR
  #define NODRAWTEXT
  #define NOKERNEL
  #define NONLS
  #define NOMEMMGR
  #define NOMETAFILE
  #define NOMINMAX
  #define NOMSG
  #define NOOPENFILE
  #define NOSCROLL
  #define NOSERVICE
  #define NOSOUND
  #define NOTEXTMETRIC
  #define NOWH
  #define NOWINOFFSETS
  #define NOCOMM
  #define NOKANJI
  #define NOHELP
  #define NOPROFILER
  #define NODEFERWINDOWPOS
  #define NOMCX
  #define NOGDI
// We need these for MessageBox
// #define NOMB
// #define NOUSER
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
  settings.WindowSettings.Size = {1'280, 720};
  settings.WindowSettings.SizeBounds = {.Resizable = true};

  auto result = CreateApplication<Editor>(argc, argv, settings);
  if (!result.has_value())
  {
#if KRYS_OS(WINDOWS)
    ::MessageBoxA(nullptr, result.error().c_str(), NULL, MB_OK | MB_ICONERROR | MB_DEFAULT_DESKTOP_ONLY);
#endif
    return -1;
  }

  Unique<Editor> editor = std::move(result.value());
  editor->Run();

  return 0;
}