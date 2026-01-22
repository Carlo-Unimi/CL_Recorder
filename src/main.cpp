#include <string>
#include <vector>
#include <iostream>
#include <ncurses.h>

#include "../include/menu.h"
#include "../include/AudioManager.h"

/*    PROGRAM HEAD    */
int main()
{
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
