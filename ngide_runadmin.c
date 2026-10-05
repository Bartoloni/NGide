#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) int64_t ngide_run_as_admin(const char *file, const char *parameters, const char *directory) {
    HINSTANCE result = ShellExecuteA(NULL, "runas", file, parameters, directory, SW_SHOWNORMAL);
    return (int64_t)(INT_PTR)result;
}

#ifdef __cplusplus
}
#endif
