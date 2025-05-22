#pragma once
#include <filesystem>

namespace mini
{
static class Path
{
  public:
    static std::filesystem::path ExecutablePath();
    static std::filesystem::path ExecutableDir();
    static std::filesystem::path ShadersDir();
    static std::filesystem::path ResourcesDir();
    static std::filesystem::path MeshesDir();
    static std::filesystem::path TexturesDir();
    static std::filesystem::path CurrentWorkingDir();
};
} // namespace mini
