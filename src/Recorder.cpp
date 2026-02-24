#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "../include/Recorder.h"

static bool ensure_directory(const std::string &path)
{
  if (path.empty())
    return false;

  // if exists and is dir -> ok
  struct stat st;
  if (stat(path.c_str(), &st) == 0)
  {
    return S_ISDIR(st.st_mode);
  }

  // create directories recursively
  std::string cur;
  if (path[0] == '/')
    cur = "/";

  size_t pos = 0;
  while (pos < path.size())
  {
    size_t next = path.find_first_of('/', pos);
    if (next == std::string::npos)
      next = path.size();
    std::string part = path.substr(pos, next - pos);
    if (!part.empty())
    {
      if (cur.size() && cur.back() != '/')
        cur += '/';
      cur += part;
      if (stat(cur.c_str(), &st) != 0)
      {
        if (mkdir(cur.c_str(), 0777) != 0 && errno != EEXIST)
          return false;
      }
    }
    pos = next + 1;
  }
  return true;
}

Recorder::Recorder(int channels, int sampleRate) : recording(false), handle(nullptr) {
  numChannels = channels;
  format = SND_PCM_FORMAT_S16_LE;
  this->sampleRate = sampleRate;
}

Recorder::~Recorder()
{
  if (isRecording())
  {
    stop();
  }
}

bool Recorder::start(const std::string &devName, const std::string &path)
{
  if (recording)
  {
    std::cerr << "Recording is already in progress." << std::endl;
    return false;
  }

  deviceName = devName;

  // determine save path
  if (!path.empty())
  {
    savePath = path;
  }
  else
  {
    const char *homeDir = getenv("HOME");
    if (homeDir == nullptr)
    {
      std::cerr << "Unable to get home directory." << std::endl;
      return false;
    }
    savePath = std::string(homeDir) + "/Music/recordings";
  }

  // normalize: remove trailing slash
  if (!savePath.empty() && savePath.back() == '/')
    savePath.pop_back();

  if (!ensure_directory(savePath))
  {
    std::cerr << "Error creating/ensuring directory: " << savePath << std::endl;
    return false;
  }

  recording = true;
  recordingThread = std::thread(&Recorder::record, this);
  return true;
}

void Recorder::stop()
{
  if (recording)
  {
    recording = false;
    if (recordingThread.joinable())
    {
      recordingThread.join();
    }
  }
}

bool Recorder::isRecording() const
{
  return recording;
}

std::vector<float> Recorder::getChannelLevels() const
{
  std::lock_guard<std::mutex> lock(levelsMutex);
  return channelLevels;
}

void Recorder::record()
{
  int err;
  snd_pcm_hw_params_t *hw_params;

  if ((err = snd_pcm_open(&handle, deviceName.c_str(), SND_PCM_STREAM_CAPTURE, 0)) < 0)
  {
    std::cerr << "Cannot open audio device " << deviceName << ": " << snd_strerror(err) << std::endl;
    recording = false;
    return;
  }

  if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
  {
    std::cerr << "Cannot allocate hardware parameter structure: " << snd_strerror(err) << std::endl;
    recording = false;
    return;
  }

  if ((err = snd_pcm_hw_params_any(handle, hw_params)) < 0)
  {
    std::cerr << "Cannot initialize hardware parameter structure: " << snd_strerror(err) << std::endl;
    snd_pcm_hw_params_free(hw_params);
    recording = false;
    return;
  }

  if ((err = snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
  {
    std::cerr << "Cannot set access type: " << snd_strerror(err) << std::endl;
    snd_pcm_hw_params_free(hw_params);
    recording = false;
    return;
  }

  if ((err = snd_pcm_hw_params_set_format(handle, hw_params, format)) < 0)
  {
    std::cerr << "Cannot set sample format: " << snd_strerror(err) << std::endl;
    snd_pcm_hw_params_free(hw_params);
    recording = false;
    return;
  }

  if ((err = snd_pcm_hw_params_set_rate_near(handle, hw_params, &sampleRate, 0)) < 0)
  {
    std::cerr << "Cannot set sample rate: " << snd_strerror(err) << std::endl;
    snd_pcm_hw_params_free(hw_params);
    recording = false;
    return;
  }

  if ((err = snd_pcm_hw_params_set_channels(handle, hw_params, numChannels)) < 0)
  {
    std::cerr << "Cannot set channel count: " << snd_strerror(err) << std::endl;
    snd_pcm_hw_params_free(hw_params);
    recording = false;
    return;
  }

  if ((err = snd_pcm_hw_params(handle, hw_params)) < 0)
  {
    std::cerr << "Cannot set parameters: " << snd_strerror(err) << std::endl;
    snd_pcm_hw_params_free(hw_params);
    recording = false;
    return;
  }

  snd_pcm_hw_params_free(hw_params);

  if ((err = snd_pcm_prepare(handle)) < 0)
  {
    std::cerr << "Cannot prepare audio interface for use: " << snd_strerror(err) << std::endl;
    recording = false;
    return;
  }

  std::string dirPath = savePath + "/";

  std::vector<std::ofstream> outFiles;
  std::vector<std::vector<char>> pcm_data_buffers(numChannels);

  for (unsigned int i = 0; i < numChannels; ++i)
  {
    std::string filePath = dirPath + "channel_" + std::to_string(i) + ".wav";
    outFiles.emplace_back(filePath, std::ios::binary);
    writeWavHeader(outFiles[i], 0, 1, sampleRate, 16);
  }

  int buffer_frames = 128;
  int frame_size = numChannels * snd_pcm_format_width(format) / 8;
  std::vector<char> buffer(buffer_frames * frame_size);

  // Initialize channel levels
  {
    std::lock_guard<std::mutex> lock(levelsMutex);
    channelLevels.resize(numChannels, 0.0f);
  }

  while (recording)
  {
    if ((err = snd_pcm_readi(handle, buffer.data(), buffer_frames)) != buffer_frames)
    {
      if (err < 0)
      {
        std::cerr << "Read from audio interface failed: " << snd_strerror(err) << std::endl;
      }
      else if (err != buffer_frames)
      {
        std::cerr << "Short read from audio interface, frames = " << err << "/" << buffer_frames << std::endl;
      }
      continue;
    }

    for (int i = 0; i < buffer_frames; ++i)
    {
      for (unsigned int ch = 0; ch < numChannels; ++ch)
      {
        char *sample_ptr = buffer.data() + i * frame_size + ch * (snd_pcm_format_width(format) / 8);
        pcm_data_buffers[ch].insert(pcm_data_buffers[ch].end(), sample_ptr, sample_ptr + 2);
      }
    }

    // Calculate RMS levels for each channel
    std::vector<float> rmsLevels(numChannels, 0.0f);
    for (unsigned int ch = 0; ch < numChannels; ++ch)
    {
      double sumSquares = 0.0;
      for (int i = 0; i < buffer_frames; ++i)
      {
        int16_t *sample_ptr = reinterpret_cast<int16_t *>(buffer.data() + i * frame_size + ch * 2);
        double sample = static_cast<double>(*sample_ptr) / 32768.0; // normalize to -1.0 to 1.0
        sumSquares += sample * sample;
      }
      rmsLevels[ch] = static_cast<float>(std::sqrt(sumSquares / buffer_frames));
    }

    // Update shared levels
    {
      std::lock_guard<std::mutex> lock(levelsMutex);
      channelLevels = rmsLevels;
    }
  }

  snd_pcm_close(handle);
  handle = nullptr;

  for (unsigned int i = 0; i < numChannels; ++i)
  {
    int pcm_size = pcm_data_buffers[i].size();
    outFiles[i].seekp(0, std::ios::beg);
    writeWavHeader(outFiles[i], pcm_size, 1, sampleRate, 16);
    outFiles[i].write(pcm_data_buffers[i].data(), pcm_size);
    outFiles[i].close();
  }
}

void Recorder::writeWavHeader(std::ofstream &file, int pcmDataSize, int numChannels, int sampleRate, int bitsPerSample)
{
  int byteRate = sampleRate * numChannels * bitsPerSample / 8;
  int blockAlign = numChannels * bitsPerSample / 8;
  int chunkSize = 36 + pcmDataSize;

  // RIFF chunk descriptor
  file.write("RIFF", 4);
  file.write(reinterpret_cast<const char *>(&chunkSize), 4);
  file.write("WAVE", 4);

  // "fmt" sub-chunk
  file.write("fmt ", 4);
  int subchunk1Size = 16;
  file.write(reinterpret_cast<const char *>(&subchunk1Size), 4);
  short audioFormat = 1; // PCM
  file.write(reinterpret_cast<const char *>(&audioFormat), 2);
  file.write(reinterpret_cast<const char *>(&numChannels), 2);
  file.write(reinterpret_cast<const char *>(&sampleRate), 4);
  file.write(reinterpret_cast<const char *>(&byteRate), 4);
  file.write(reinterpret_cast<const char *>(&blockAlign), 2);
  file.write(reinterpret_cast<const char *>(&bitsPerSample), 2);

  // "data" sub-chunk
  file.write("data", 4);
  file.write(reinterpret_cast<const char *>(&pcmDataSize), 4);
}