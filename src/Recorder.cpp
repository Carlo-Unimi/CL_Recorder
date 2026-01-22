#include "../include/Recorder.h"

Recorder::Recorder() : m_recording(false) {};

Recorder::~Recorder() {stop();}

bool Recorder::start(const std::string &deviceName, const std::string &filePath)
{
  if (m_recording)
  {
    return false; // Already recording
  }

  m_recording = true;
  m_worker = std::thread(&Recorder::recordingLoop, this, deviceName, filePath);
  return true;
}

void Recorder::stop()
{
  if (m_recording)
  {
    m_recording = false;
    if (m_worker.joinable())
    {
      m_worker.join();
    }
  }
}

bool Recorder::isRecording() const {return m_recording;}

void Recorder::writeWavHeader(std::ofstream &file, int sampleRate, int bitsPerSample, int channels)
{
  file << "RIFF";
  file << "----"; // Placeholder for file size
  file << "WAVE";
  file << "fmt ";

  int subChunk1Size = 16;
  short audioFormat = 1; // PCM

  file.write(reinterpret_cast<const char *>(&subChunk1Size), 4);
  file.write(reinterpret_cast<const char *>(&audioFormat), 2);
  file.write(reinterpret_cast<const char *>(&channels), 2);
  file.write(reinterpret_cast<const char *>(&sampleRate), 4);

  int byteRate = sampleRate * channels * (bitsPerSample / 8);
  int blockAlign = channels * (bitsPerSample / 8);

  file.write(reinterpret_cast<const char *>(&byteRate), 4);
  file.write(reinterpret_cast<const char *>(&blockAlign), 2);
  file.write(reinterpret_cast<const char *>(&bitsPerSample), 2);

  file << "data";
  file << "----"; // Placeholder for data size
}

void Recorder::updateWavHeader(const std::string &filePath)
{
  std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);
  if (!file.is_open())
    return;

  file.seekp(0, std::ios::end);
  int fileSize = (int)file.tellp();
  int dataSize = fileSize - 44;

  // Update RIFF chunk size (File size - 8)
  int riffSize = fileSize - 8;
  file.seekp(4, std::ios::beg);
  file.write(reinterpret_cast<const char *>(&riffSize), 4);

  // Update data chunk size
  file.seekp(40, std::ios::beg);
  file.write(reinterpret_cast<const char *>(&dataSize), 4);

  file.close();
}

void Recorder::recordingLoop(std::string deviceName, std::string filePath)
{
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  int err;
  int dir;
  unsigned int rate = m_sampleRate;

  // Open PCM device for recording (capture)
  if ((err = snd_pcm_open(&handle, deviceName.c_str(), SND_PCM_STREAM_CAPTURE, 0)) < 0)
  {
    std::cerr << "Cannot open audio device " << deviceName << " (" << snd_strerror(err) << ")" << std::endl;
    m_recording = false;
    return;
  }

  // Allocate hardware parameters
  snd_pcm_hw_params_alloca(&params);
  snd_pcm_hw_params_any(handle, params);
  snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
  snd_pcm_hw_params_set_format(handle, params, m_format);
  snd_pcm_hw_params_set_channels(handle, params, m_channels);
  snd_pcm_hw_params_set_rate_near(handle, params, &rate, &dir);

  if ((err = snd_pcm_hw_params(handle, params)) < 0)
  {
    std::cerr << "Cannot set hardware parameters (" << snd_strerror(err) << ")" << std::endl;
    snd_pcm_close(handle);
    m_recording = false;
    return;
  }

  if ((err = snd_pcm_prepare(handle)) < 0)
  {
    std::cerr << "Cannot prepare audio interface for use (" << snd_strerror(err) << ")" << std::endl;
    snd_pcm_close(handle);
    m_recording = false;
    return;
  }

  // Open file
  std::ofstream file(filePath, std::ios::binary);
  if (!file.is_open())
  {
    std::cerr << "Cannot open file for writing: " << filePath << std::endl;
    snd_pcm_close(handle);
    m_recording = false;
    return;
  }

  writeWavHeader(file, rate, 16, m_channels);

  // Buffer for reading audio
  snd_pcm_uframes_t frames = 1024;
  int frame_size = m_channels * 2; // 2 bytes for 16-bit
  std::vector<char> buffer(frames * frame_size);

  while (m_recording)
  {
    int rc = snd_pcm_readi(handle, buffer.data(), frames);
    if (rc == -EPIPE)
    {
      // Overrun
      snd_pcm_prepare(handle);
    }
    else if (rc < 0)
    {
      std::cerr << "Error from read: " << snd_strerror(rc) << std::endl;
    }
    else if (rc > 0)
    {
      file.write(buffer.data(), rc * frame_size);
    }
  }

  file.close();
  snd_pcm_drain(handle);
  snd_pcm_close(handle);

  // Update header with correct sizes
  updateWavHeader(filePath);
}
