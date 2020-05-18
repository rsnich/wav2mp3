#include "../dir_content.hpp"
#include <algorithm>
#include <windows.h>

class dir_content::impl {
public:
  impl(std::string &&file_ext) : m_file_ext(std::move(file_ext)) {
    memset(&m_file_data, 0x0, sizeof(WIN32_FIND_DATA));
  }
  ~impl() { close_dir(); }

  int open_dir(const std::string &dir_path) {
    std::string path = dir_path + "\\*";
    if (!m_file_ext.empty())
      path += "." + m_file_ext;

    m_file_handle = FindFirstFile(path.c_str(), &m_file_data);
    m_status = m_file_handle == INVALID_HANDLE_VALUE ? -1 : 0;
    return m_status;
  }

  int close_dir() { return FindClose(m_file_handle) ? 0 : -1; }

  int get_reg_file(std::string &file_name) {
    int status = 0;

    // Read next directory entry
    std::string direnry_name;
    DWORD file_type = 0xFFFFFFFF & ~FILE_ATTRIBUTE_DIRECTORY;
    while ((status = get_direnry_name(file_type, direnry_name)) == 0) {
      file_name = std::move(direnry_name);
      break;
    }
    return status;
  }

private:
  int get_direnry_name(DWORD d_type, std::string &direntry_name) {
    if (m_status < 0 || *m_file_data.cFileName == '\0')
      return -1;

    direntry_name = m_file_data.cFileName;

    BOOL status = FALSE;
    while ((status = FindNextFile(m_file_handle, &m_file_data)) != FALSE) {
      if (m_file_data.cFileName[0] != '\0' &&
          strcmp(m_file_data.cFileName, ".") &&
          strcmp(m_file_data.cFileName, "..") &&
          m_file_data.dwFileAttributes & d_type) {
        break;
      }
    }

    m_status = status ? 0 : -1;
    return 0;
  }

private:
  /**
   * @brief Contains file data
   */
  WIN32_FIND_DATA m_file_data;
  /**
   * @brief Handle of file operations
   */
  HANDLE m_file_handle = INVALID_HANDLE_VALUE;
  /**
   * @brief File extension to filter files
   */
  std::string m_file_ext;
  int m_status = 0;
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
