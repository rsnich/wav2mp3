#include <algorithm>
#include <dir_content.hpp>
#include <dirent.h>
#include <string.h>

class dir_content::impl {
public:
  impl(std::string &&file_ext) : m_file_ext(std::move(file_ext)) {
    std::transform(m_file_ext.begin(), m_file_ext.end(), m_file_ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });
  }
  ~impl() { close_dir(); }
  int open_dir(const std::string &dir_path) {
    m_dir = opendir(dir_path.c_str());
    return m_dir == nullptr ? -errno : 0;
  }

  int close_dir() {
    int status = closedir(m_dir);
    m_dir = nullptr;
    return status;
  }

  int get_reg_file(std::string &file_name) const {
    int status = 0;
    // Check file extension here
    auto is_right_file = [&](const std::string &file) {
      if (m_file_ext.empty())
        return true;

      auto found = file.rfind('.');
      if (found == std::string::npos)
        return false;

      auto ext = file.substr(found + 1);
      std::transform(ext.begin(), ext.end(), ext.begin(),
                     [](unsigned char c) { return std::tolower(c); });

      return m_file_ext == ext;
    };

    // Read next directory entry
    std::string direnry_name;
    while ((status = get_direnry_name(DT_REG, direnry_name)) == 0) {
      if (is_right_file(direnry_name)) {
        file_name = std::move(direnry_name);
        break;
      }
    }
    return status;
  }

private:
  int get_direnry_name(unsigned char d_type, std::string &direntry_name) const {
    struct dirent *dir_ent;
    dir_ent = readdir(m_dir);
    while (dir_ent != nullptr) {
      if (strcmp(dir_ent->d_name, ".") && strcmp(dir_ent->d_name, "..") &&
          dir_ent->d_type == d_type) {
        direntry_name = dir_ent->d_name;
        break;
      }
      dir_ent = readdir(m_dir);
    }
    int status = 0;
    if (dir_ent == nullptr)
      status = errno == 0 ? 1 : -errno;

    return status;
  }

private:
  DIR *m_dir = nullptr;
  std::string m_file_ext;
};

dir_content::dir_content(std::string &&file_ext)
    : m_impl(std::make_unique<impl>(std::move(file_ext))) {}

dir_content::~dir_content() = default;

int dir_content::open_dir(const std::string &dir_path) {
  return m_impl->open_dir(dir_path);
}

int dir_content::close_dir() { return m_impl->close_dir(); }

int dir_content::get_reg_file(std::string &file_name) const {
  return m_impl->get_reg_file(file_name);
}
