#pragma once

#include <memory>
#include <string>

class dir_content {
public:
  /**
   * @brief Construct a new dir content object
   */
  dir_content(std::string &&file_ext);
  /**
   * @brief Destroy the dir content object
   */
  ~dir_content();
  /**
   * @brief Opens specified directory
   *
   * @param dir_path Path to the directory which should be opened
   * @return int 0 - success, < 0 - an error
   */
  int open_dir(const std::string &dir_path);
  /**
   * @brief Closes current dir
   *
   * @return int 0 - success, < 0 - an error
   */
  int close_dir();
  /**
   * @brief Get a regular file object
   *
   * @param file_name Returned file name
   * @return int 0 - success, > 0 - end of the directory stream, < 0 - an error
   */
  int get_reg_file(std::string &file_name) const;

private:
  class impl;
  std::unique_ptr<impl> m_impl;
};