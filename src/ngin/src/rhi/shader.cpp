#include <ngin/rhi/shader.h>

#include <fstream>
namespace Ngin {
std::filesystem::path Shader::shaderDir;

Shader::Shader(std::string_view name) {
  if (shaderDir.empty()) {
    shaderDir = getExecutableDirectory();
    logDebug(std::format("Shader dir before removal of filename: {}", shaderDir.string()));
  }

  logDebug(std::format("Current shader dir: {}", shaderDir.string()));

  std::ifstream shaderIn(shaderDir / name, std::ios::binary);
  logDebug(std::format("opening file: {}", (shaderDir / name).string()));
  if (shaderIn.is_open()) {
    shaderIn.seekg(0, std::ios::end);
    size = shaderIn.tellg();
    shaderIn.seekg(0, std::ios::beg);
    data = malloc(size);
    if (data) {
      shaderIn.read((char*)data, size);
    }
  } else {
    logError(std::format("File \"{}\" could not be opened", (shaderDir / name).string()));
  }
}
}  // namespace Ngin
