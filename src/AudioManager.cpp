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

        // consider only input or bidirectional devices
        if (name != NULL && (io == NULL || std::string(io) == "Input")) {
            AudioDevice device;
            device.name = std::string(name);

            if (desc != NULL) {
                std::string d(desc);
                // ALSA formats descriptions with newlines, replace them with spaces for the menu
                size_t pos;
                while ((pos = d.find('\n')) != std::string::npos) {
                   d.replace(pos, 1, " - ");
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
    return devices;
}