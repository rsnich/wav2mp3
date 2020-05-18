#pragma once
#include <functional>
#include <memory>
#include <mp3lame/lame.h>
#include <string>

/**
 * @brief Contains functionality to convert WAV format into MP3 format
 */
class wav2mp3_convertor {
public:
  /**
   * @brief Construct a new wav2mp3 convertor object
   */
  wav2mp3_convertor() = default;
  /**
   * @brief Destroy the wav2mp3 convertor object
   */
  ~wav2mp3_convertor() = default;
  /**
   * @brief Conerts specifies WAV file into MP3 format
   *
   * @param wav_path Path to the source (WAV) file
   * @param mp3_path Path to the destination (MP3) file
   * @return int int 0 - success, error otherwise
   */
  int convert(const std::string &wav_path, const std::string &mp3_path);

private:
  /**
   * @brief Initialises and configures lame library
   *
   * @return int 0 - success, error otherwise
   */
  int lame_configure();
  /**
   * @brief Encodes a WAV chunk
   *
   * @param samples Number of samples caluculated during inialization
   * @param samples_buffer Allocated sample buffer
   * @param output Allocated buffer for MP3 output
   * @param output_size Size of output buffer
   * @return int 0 - success, error otherwise
   */
  int encode_wav_chunk(unsigned int samples,
                       std::unique_ptr<short[]> &samples_buffer,
                       std::unique_ptr<unsigned char[]> &output,
                       int &output_size);
  /**
   * @brief Takes the rest of encoded MP3 data
   *
   * @param output Allocated buffer for MP3 output
   * @param output_size
   * @return int Size of output buffer
   */
  int encode_wav_flush(std::unique_ptr<unsigned char[]> &output,
                       int &output_size);

private:
  /**
   * @brief Handle of the lame library
   */
  using lame_unique_ptr_t =
      std::unique_ptr<lame_global_flags,
                      std::function<void(lame_global_flags *)>>;
  lame_unique_ptr_t m_lame_handle;
};