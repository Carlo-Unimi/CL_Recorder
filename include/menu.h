#include <vector>
#include <string>
#include <ncurses.h>
#include <iostream>

struct content_info
{
    std::vector<std::string> content;
};

/**
 * @class menu
 * @brief Structure to hold menu options including title, choices, and a flag for recording capability.
 */
class menu
{
private:
    /**
     * @brief display the menu on the screen.
     */
    void display();

    /**
     * @brief draws the options under the title with the current selected option highlighted.
     */
    void draw_options();

    /**
     * @brief draws a horizontal line for the options section based on the lenght of the options.
     */
    void draw_option_line(int h);

    /**
     * @brief handles the content window drawing.
     */
    void drawContentWindow();

    int current_option;

public:
    std::vector<std::string> title;
    std::vector<std::string> options;
    WINDOW *window;         // the entire menu window
    WINDOW *content_window; // the content area window
    std::vector<content_info> content;

    /**
     * @brief constructor for the menu class.
     * @param title vector of strings representing the title lines of the menu.
     *
     * initializes the menu with the given title and sets up the window. it doesn't display the menu yet.
     */
    menu(std::vector<std::string> title, std::vector<std::string> options);
    void run();
};