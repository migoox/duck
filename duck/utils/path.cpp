#include "path.h"

#include <Window.h>
namespace mini
{
std::filesystem::path Path::ExecutablePath()
{
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer);
}

std::filesystem::path Path::ExecutableDir()
{
    return ExecutablePath().parent_path();
}

std::filesystem::path Path::ShadersDir()
{
    return ExecutableDir() / "shaders";
}

std::filesystem::path Path::MeshesDir()
{
    return ExecutableDir() / "resources" / "meshes";
}

std::filesystem::path Path::TexturesDir()
{
    return ExecutableDir() / "resources" / "textures";
}

std::filesystem::path Path::ResourcesDir()
{
    return ExecutableDir() / "resources";
}

std::filesystem::path Path::CurrentWorkingDir()
{
    return std::filesystem::current_path();
}
} // namespace mini
