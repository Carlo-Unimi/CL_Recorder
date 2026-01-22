#include "../include/AudioManager.h"

std::vector<AudioDevice> AudioManager::list_input_devices() {
    std::vector<AudioDevice> devices;
    void **hints;
    const char *iface = "pcm";
    int err;

    if ((err = snd_device_name_hint(-1, iface, &hints)) < 0) {
        return devices;
    }

    void **n = hints;
    while (*n != NULL) {
        char *name = snd_device_name_get_hint(*n, "NAME");
        char *desc = snd_device_name_get_hint(*n, "DESC");
        char *io = snd_device_name_get_hint(*n, "IOID");

        std::string nameStr = (name != NULL) ? std::string(name) : "";

        // only hardware devices (start with "hw:" or "plughw:")
        if (!nameStr.empty() && (nameStr.rfind("plughw:", 0) == 0) && (io == NULL || std::string(io) == "Input")) 
        {    
            AudioDevice device;
            device.name = nameStr;
            device.selected = false;

            if (desc != NULL) {
                std::string d(desc);
                size_t pos;
                while ((pos = d.find('\n')) != std::string::npos) {
                  d.replace(pos, 1, " - ");
                  size_t dashPos = d.find(" - ");
                  if (dashPos != std::string::npos) {
                    d = d.substr(0, dashPos);
                  }
                }
                device.description = d;
            } else {
                device.description = device.name;
            }
            
            devices.push_back(device);
        }

        if (name) free(name);
        if (desc) free(desc);
        if (io) free(io);
        n++;
    }

    snd_device_name_free_hint(hints);
    if (!devices.empty())
        devices[0].selected = true; // select first device by default
    return devices;
}