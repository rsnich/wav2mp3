#include "wav2mp3_convertor.hpp"
#include <functional>
#include <iostream>

constexpr size_t wav_header_size = 44;
constexpr unsigned int wav_buffer_size = 4096;
constexpr unsigned int mp3_buffer_size = 4096;

using file_unique_ptr_t = std::unique_ptr<FILE, std::function<void(FILE *)>>;

int wav2mp3_convertor::convert(const std::string &wav_path,
                               const std::string &mp3_path) {
  // Create and configure lame library
  if (lame_configure() != 0)
    return -1;

  // Allocate WAV buffer
  std::unique_ptr<short[]> wav_buffer{new short[2 * wav_buffer_size]};
  // Allocate MP3 buffer
  std::unique_ptr<unsigned char[]> mp3_buffer{
      new unsigned char[mp3_buffer_size]};

  // Open WAV the specified file
  file_unique_ptr_t file_wav =
      file_unique_ptr_t(fopen(wav_path.c_str(), "rb"), [](FILE *to_close) {
        if (to_close != nullptr)
          fclose(to_close);
      });

  if (file_wav == nullptr)
    return -1;

  // Open MP3 the destination file
  file_unique_ptr_t file_mp3 =
      file_unique_ptr_t(fopen(mp3_path.c_str(), "wb"), [](FILE *to_close) {
        if (to_close != nullptr)
          fclose(to_close);
      });

  if (file_mp3 == nullptr)
    return -1;

  fseek(file_wav.get(), wav_header_size, SEEK_SET);

  int read, write;

  // Convert All PCM samples
  while ((read = static_cast<int>(fread(wav_buffer.get(), 2 * sizeof(short),
                                        wav_buffer_size, file_wav.get()))) >
         0) {
    // Encode samples
    if (encode_wav_chunk(read, wav_buffer, mp3_buffer, write) != 0)
      return -1;

    // write 'write' bytes that are returned in tehe mp3_buffer to disk
    if (fwrite(mp3_buffer.get(), sizeof(unsigned char), write,
               file_mp3.get()) != write)
      return -1;
  }

  // Flush the rest of data
  if (encode_wav_flush(mp3_buffer, write) < 0)
    return -1;

  if (write > 0) {
    if (fwrite(mp3_buffer.get(), sizeof(unsigned char), write,
               file_mp3.get()) != write)
      return -1;
  }

  return 0;
}

int wav2mp3_convertor::encode_wav_chunk(
    unsigned int samples, std::unique_ptr<short[]> &samples_buffer,
    std::unique_ptr<unsigned char[]> &output, int &output_size) {
  if (m_lame_handle == nullptr)
    return -1;

  output_size =
      lame_encode_buffer_interleaved(m_lame_handle.get(), samples_buffer.get(),
                                     samples, output.get(), mp3_buffer_size);

  if (output_size < 0) {
    output_size = 0;
    return -1;
  }

  return 0;
}

int wav2mp3_convertor::encode_wav_flush(
    std::unique_ptr<unsigned char[]> &output, int &output_size) {
  output_size = lame_encode_flush(m_lame_handle.get(), output.get(), 0);
  if (output_size < 0) {
    output_size = 0;
    return -1;
  }
  return 0;
}

int wav2mp3_convertor::lame_configure() {
  m_lame_handle =
      lame_unique_ptr_t(lame_init(), [](lame_global_flags *to_close) {
        if (to_close != nullptr)
          lame_close(to_close);
      });

  // Set input sample frequency
  lame_set_in_samplerate(m_lame_handle.get(), 22050);
  // Set output format joint joint stereo
  lame_set_mode(m_lame_handle.get(), JOINT_STEREO);
  // Set number of channels
  lame_set_num_channels(m_lame_handle.get(), 2);
  // Set minimum bitrate.
  lame_set_brate(m_lame_handle.get(), 128);
  // Set the GOOD quality
  lame_set_quality(m_lame_handle.get(), 5);

  // Initialise lame parameters
  int status = lame_init_params(m_lame_handle.get());
  if (status != 0) {
    return status;
  }

  return 0;
}
