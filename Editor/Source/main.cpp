#include <core.h>
#include <filesystem>
#include <string>

#include "editor_app.h"
#include "editor_settings.h"

#if _WIN32
extern "C" {
_declspec(dllexport) DWORD nv_optimus_enablement = 0x00000001;
__declspec(dllexport) int amd_power_xpress_request_high_performance = 1;
}
#endif

int main(int argc, char** argv) {
  // The manifest is relative to the working directory: run the editor from the
  // folder the project lives in, the way the engine's own resources are found.
  // The settings are read first, because they decide which project to reopen
  // when no path was given.
  const std::string argument = argc > 1 ? argv[1] : "";

  EditorSettings settings;
  settings.Load();

  const std::filesystem::path project_file = StartupProject(settings, argument);

  EditorApp editor(project_file);
  editor.Start();
  return EXIT_SUCCESS;
}
