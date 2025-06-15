#include "Krystal.Editor/Editor.hpp"
#include "Krystal.Engine/Application.hpp"

int main(int argc, char **argv)
{
  using namespace Krys;

  ApplicationSettings settings {};
  settings.WindowSettings = {};
  settings.WindowSettings.Title = "Krystal Editor";
  settings.WindowSettings.VSync = false;
  settings.WindowSettings.Width = 1'920;
  settings.WindowSettings.Height = 1'080;

  auto app = Application::Create<Editor>(argc, argv, settings);
  app->Run();
}