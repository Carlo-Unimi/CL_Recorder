#include <vector>
#include <string>
#include <ncurses.h>
#include <iostream>

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

	int current_option;

public:
	std::vector<std::string> title;
	std::vector<std::string> options;
	WINDOW *window;			// the entire menu window
	WINDOW *content_window; // the content area window

	/**
	 * @brief constructor for the menu class.
	 * @param title vector of strings representing the title lines of the menu.
	 *
	 * initializes the menu with the given title and sets up the window. it doesn't display the menu yet.
	 */
	menu(std::vector<std::string> title, std::vector<std::string> options);
	void run();
};