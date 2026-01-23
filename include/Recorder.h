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

public:
  /**
   * @brief constructs a Recorder object.
   */
  Recorder() = default;

  /**
   * @brief destructs the Recorder object, ensuring any ongoing recording is stopped.
   */
  ~Recorder() = default;

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
};

#endif