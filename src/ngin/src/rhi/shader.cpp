#include <ngin/rhi/shader.h>

#include <fstream>
namespace Ngin {
std::filesystem::path Shader::shaderDir;

Shader::Shader(std::string_view name) {
  if (shaderDir.empty()) {
    shaderDir = getExecutableDirectory();
    shaderDir.remove_filename();
  }

  std::ifstream shaderIn(shaderDir / name, std::ios::binary);
  if (shaderIn.is_open()) {
    shaderIn.seekg(0, std::ios::end);
    size = shaderIn.tellg();
    shaderIn.seekg(0, std::ios::beg);
    data = malloc(size);
    if (data) {
      shaderIn.read((char*)data, size);
    }
  }
}
}  // namespace Ngin
