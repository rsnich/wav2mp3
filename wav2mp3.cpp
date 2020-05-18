#include "thread_pool.hpp"
#include "utils/file_utils/dir_content.hpp"
#include "wav2mp3_convertor.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdarg.h>

/**
 * @brief Initial number of threads in the thread pool
 */
constexpr size_t threads_number = 16;

static const char *wav_file_ext = "wav";
static const char *mp3_file_ext = "mp3";

void usage() {
  std::cout << "A path to a directory with WAV files should be provided."
            << std::endl;
  std::cout << std::endl;
  std::cout << "    wav2mp3 <a directory with WAV files>" << std::endl;
  std::cout << std::endl;
  std::cout << "All found WAV files will be converted to MP3 files and "
            << std::endl;
  std::cout << "saved with the same name and in the same directory."
            << std::endl;
  std::cout << "********************************************************"
            << std::endl;
  std::cout << std::endl;
}

int main(int argc, char *argv[]) {
  std::cout << "********************************************************"
            << std::endl;
  std::cout << "           wav2mp3 WAV to MP3 covertor v1.0" << std::endl;
  std::cout << "********************************************************"
            << std::endl;
  std::cout << std::endl;

  if (argc < 2) {
    usage();
    return -1;
  }

  const char *input_dir = argv[1];

  // Create thread pool
  thread_pool thread_processing(
      std::max(threads_number, size_t(std::thread::hardware_concurrency())));

  // Scan the input directory
  dir_content dc(wav_file_ext);
  if (dc.open_dir(input_dir) != 0) {
    std::cout << "Cannot open the directory: '" << input_dir
              << "' or requested files not found." << std::endl;
    return -1;
  }

  int found_wav_files = 0;
  int processed_error = 0;

  std::string wav_file_name;
  while (dc.get_reg_file(wav_file_name) == 0) {
    // Create MP3 file name
    auto found = wav_file_name.rfind('.');
    if (found == std::string::npos)
      continue;

    auto ext = wav_file_name.substr(found + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    auto mp3_file_name = wav_file_name;
    mp3_file_name.replace(mp3_file_name.begin() + found + 1,
                          mp3_file_name.end(), mp3_file_ext);

    auto wav_path = std::string(input_dir) + "/" + wav_file_name;
    auto mp3_path = std::string(input_dir) + "/" + mp3_file_name;

    // Submit a task for found a new WAV file
    auto task = [wav_path, mp3_path, &processed_error]() {
      wav2mp3_convertor convertor;
      if (convertor.convert(wav_path, mp3_path) != 0)
        ++processed_error;
    };
    thread_processing.enqueue_task(std::move(task));
    ++found_wav_files;
  }
  dc.close_dir();

  auto interval_start = std::chrono::high_resolution_clock::now();

  std::cout << "Found " << found_wav_files << " WAV files" << std::endl;
  std::cout << std::endl;
  if (found_wav_files > 0)
    std::cout << "Processing..." << std::endl;

  // Wait until all tasks are processed
  thread_processing.wait_finished();

  auto interval_now = std::chrono::high_resolution_clock::now();
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
      interval_now - interval_start);

  if (found_wav_files > 0) {
    std::cout << "Processed: " << found_wav_files - processed_error
              << std::endl;
    std::cout << "Errors: " << processed_error << std::endl;
    std::cout << "Processing time: " << milliseconds.count() << "ms"
              << std::endl;
  }

  return 0;
}
