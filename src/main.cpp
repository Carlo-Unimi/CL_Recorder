#include <string>
#include <vector>
#include <iostream>
#include <ncurses.h>

#include "../include/menu.h"
#include "../include/AudioManager.h"

bool parseArguments(int argc, char* argv[])
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
}



/*    PROGRAM HEAD    */
int main(int argc, char* argv[])
{

  //* arguments parsing
  if (parseArguments(argc, argv))
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

  menu start_menu(title, options);

  start_menu.run();

  endwin();
  return 0;
}
