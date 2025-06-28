#include <ostream>
#include <string_view>
#include <iostream>
#include <filesystem>
#include <optional>
#include <string_view>

namespace fs = std::filesystem;

std::optional<uint64_t> get_file_size(const fs::path& p) {
  if (!fs::is_regular_file(p)) {
    std::cout << "ERROR: file `" << p << "` does not exist." << std::endl;
    return std::nullopt;
  }
  return fs::file_size(p);
}

std::optional<std::string_view> get_file_extension(const std::string_view p) {
  std::string_view path(p); // explicit type conversion
  std::string_view delimiter = ".";
  size_t delimiter_pos = path.rfind(delimiter);
  if (delimiter_pos == 0) {
    return std::nullopt;
  }
  std::string_view extension = path.substr(delimiter_pos + delimiter.length());
  return extension;
}

int main(int argc, char* argv[]) {
  fs::path directory;
  if (argc < 2) {
    directory = std::filesystem::current_path();
  } else {
    directory = argv[1];
  }

  std::cout << "Listing contents of: " << directory << std::endl;

  for (const auto& entry : fs::directory_iterator(directory)) {
    auto file_size = get_file_size(entry.path().string());
    auto file_extension = get_file_extension(entry.path().string());
    if (file_size.has_value() && file_extension.has_value()) {
      std::cout << entry.path().filename() << "\t" << file_extension.value() << " " << file_size.value() << std::endl;
    }
  }
  return 0;
}
