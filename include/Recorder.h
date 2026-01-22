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
public:
  Recorder();
  ~Recorder();

  /**
   * @brief starts the recording process in a separate thread.
   * @param deviceName the ALSA device name (e.g., "plughw:0,0").
   * @param filePath the path where the WAV file will be saved.
   * @return true if recording started successfully, false otherwise.
   */
  bool start(const std::string &deviceName, const std::string &filePath);

  /**
   * @brief stops the current recording session and saves the file.
   */
  void stop();

  /**
   * @brief checks if a recording is currently in progress.
   */
  bool isRecording() const;

private:
  /**
   * @brief the main loop that reads from the audio device and writes to the file.
   */
  void recordingLoop(std::string deviceName, std::string filePath);

  /**
   * @brief writes a WAV header to the file stream.
   * @param file output file stream.
   * @param sampleRate audio sample rate.
   * @param bitsPerSample bits per sample (e.g. 16).
   * @param channels number of channels.
   */
  void writeWavHeader(std::ofstream &file, int sampleRate, int bitsPerSample, int channels);

  /**
   * @brief updates the WAV header with the final file size after recording stops.
   */
  void updateWavHeader(const std::string &filePath);

  std::atomic<bool> m_recording; // atomic flag to control the recording thread
  std::thread m_worker;          // the thread performing the I/O

  // default Recording Configuration
  unsigned int m_sampleRate = 44100;
  int m_channels = 2;
  snd_pcm_format_t m_format = SND_PCM_FORMAT_S16_LE;
};

#endif
