#ifndef RECORDER_H
#define RECORDER_H

#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <atomic>
#include <fstream>
#include <vector>
#include <alsa/asoundlib.h>

/**
 * @class Recorder
 * @brief handles audio recording from an ALSA device into a WAV file.
 */
class Recorder
{
private:
  std::atomic<bool> recording;
  std::thread recordingThread;
  snd_pcm_t *handle;
  std::string deviceName;
  std::string savePath;
  snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

  void record();
  void writeWavHeader(std::ofstream &file, int pcmDataSize, int numChannels, int sampleRate, int bitsPerSample);

public:
  unsigned int sampleRate = 44100;
  unsigned int numChannels;

  /**
   * @brief constructs a Recorder object.
   */
  Recorder();

  /**
   * @brief destructs the Recorder object, ensuring any ongoing recording is stopped.
   */
  ~Recorder();

  /**
   * @brief starts the recording process in a separate thread.
   * @param deviceName the ALSA device name (e.g., "default").
   * @param numChannels the number of channels to record.
   * @param path directory where wav files will be stored (if empty, default is ~/Music/recordings).
   * @return true if recording started successfully, false otherwise.
   */
  bool start(const std::string &deviceName, const std::string &path);

  /**
   * @brief stops the current recording session and saves the file.
   */
  void stop();

  /**
   * @brief checks if a recording is currently in progress.
   */
  bool isRecording() const;
};

#endif