#include <string>
#include <vector>
#include <iostream>
#include <ncurses.h>

#include "../include/menu.h"
#include "../include/AudioManager.h"

bool parseArguments(int argc, char* argv[], int &channels, int &sampleRate)
{
  if (argc == 1) //* normal execution
    return false;

  if (argc == 2 && std::string(argv[1]) == "--help")
  {
    std::cout << "Usage: " << argv[0] << " [numChannels] [sampleRate]\n" << std::endl;
    std::cout << "  numChannels: Number of audio channels to record (default: 2)" << std::endl;
    std::cout << "  sampleRate: Sample rate for recording in Hz (default: 44100)\n" << std::endl;
    std::cout << "  --help     Show this help message" << std::endl;
    return true;
  }

  if (argc == 3) { //* valid arguments provided, continue execution
    try {
      channels = std::stoi(argv[1]);
      sampleRate = std::stoi(argv[2]);
      if (channels <= 0 || channels > 32) {
        std::cerr << "Error: numChannels must be between 1 and 32." << std::endl;
        return true;
      }
      if (sampleRate < 4000 || sampleRate > 384000) {
        std::cerr << "Error: sampleRate must be between 4000 and 384000 Hz." << std::endl;
        return true;
      }
    } catch (const std::exception &e) {
      std::cerr << "Error: Invalid arguments. Please provide valid integers for numChannels and sampleRate." << std::endl;
      return true;
    }
    return false; 
  }
}



/*    PROGRAM HEAD    */
int main(int argc, char* argv[])
{
  //* arguments parsing
  int channels = 2;
  int sampleRate = 44100;
  if (parseArguments(argc, argv, channels, sampleRate))
    return 0;

  initscr();
  noecho();
  cbreak();
  curs_set(0); // Hide the cursor
  refresh();

  std::vector<std::string> title = {
      " ###  #           ####   #####   ###   ###   ####   ####   #####  ####  ",
      "#     #           #   #  #      #     #   #  #   #  #   #  #      #   # ",
      "#     #           ####   ####   #     #   #  ####   #   #  ####   ####  ",
      "#     #           #  #   #      #     #   #  #  #   #   #  #      #  #  ",
      " ###  #####       #   #  #####   ###   ###   #   #  ####   #####  #   # "};
  std::vector<std::string> options = {"Recording options", "Input device", "File paths", "Exit", "START RECORDING"};

  menu start_menu(title, options, channels, sampleRate);

  start_menu.run();

  endwin();
  return 0;
}
