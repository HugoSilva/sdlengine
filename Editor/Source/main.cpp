#include <core.h>

#include "editor_app.h"

#if _WIN32
extern "C" {
_declspec(dllexport) DWORD nv_optimus_enablement = 0x00000001;
__declspec(dllexport) int amd_power_xpress_request_high_performance = 1;
}
#endif

int main(int argc, char** argv) {
  EditorApp editor;
  editor.Start();
  return EXIT_SUCCESS;
}
