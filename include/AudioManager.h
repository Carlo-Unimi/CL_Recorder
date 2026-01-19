#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <vector>
#include <string>
#include <iostream>
#include <alsa/asoundlib.h>

// struct for audio device information
struct AudioDevice {
  std::string name;        // id
  std::string description; // readable description
  std::string ioid;        // input/output
};

class AudioManager {
public:
  /**
   * @brief lists all available input audio devices using ALSA.
   */
  static std::vector<AudioDevice> list_input_devices();
};

#endif