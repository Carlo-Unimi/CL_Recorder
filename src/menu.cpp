#include "../include/menu.h"

void menu::display()
{
	wrefresh(this->window);
	wrefresh(this->content_window);
}

void menu::printContent()
{
	for (size_t i = 0; i < this->content[this->current_option].content.size(); i++)
		mvwprintw(this->content_window, 1 + i, 2, this->content[this->current_option].content[i].c_str());
}

void menu::draw_option_line(int h)
{
	int total_length = 1;
	for (const auto &opt : this->options)
		total_length += opt.length() + 4;

	std::string line(total_length, '-');
	mvwprintw(this->window, this->title.size() + h, 2, line.c_str());
}

void menu::drawContentWindow()
{
	wclear(this->content_window);
	box(this->content_window, 0, 0);
	switch (this->current_option)
	{
	case 0:
		this->content[0].content = {"Configure your recording settings here:"};
		printContent();
		break;
	case 1:
		this->content[1].content = {"Select an input device from the available options:"};
		printContent();
		break;
	case 2:
		this->content[2].content = {"Specify file paths for saving recordings:", "[default: ~/recordings/]", "", "press enter to modify"};
		printContent();

		if (path[0] != '\0')
			mvwprintw(this->content_window, getmaxy(this->content_window) - 2, 2, "Current path: %s", this->path);

		if (wgetch(this->content_window) == 10) // press enter to modify the path
		{
			mvwprintw(this->content_window, 5, 2, "-> ");
			echo();
			wgetnstr(this->content_window, this->path, 50);
			noecho();
			this->content[2].content.push_back("path set to: " + std::string(this->path));
			printContent();
		}
		timeout(3000);
		break;
	case 3:
		this->content[3].content = {"Exit option selected.", "", "Press Enter to exit the program."};
		printContent();
		break;
	case 4:
		this->content[4].content = {"START RECORDING option selected.", "", "Press Enter to start recording."};
		printContent();
		break;
	default:
		this->content[this->current_option].content = {"No content available."};
		break;
	}
}

void menu::draw_options()
{
	int aux = 2;
	// prints the options on a single row
	this->draw_option_line(2);
	for (size_t i = 0; i < this->options.size(); i++)
	{
		mvwprintw(this->window, this->title.size() + 3, aux, "|");
		if (i == this->current_option)
			wattron(this->window, A_REVERSE);
		mvwprintw(this->window, this->title.size() + 3, aux + 2, this->options[i].c_str());
		if (i == this->current_option)
			wattroff(this->window, A_REVERSE);
		aux += this->options[i].length() + 4;
	}
	mvwprintw(this->window, this->title.size() + 3, aux, "|");
	this->draw_option_line(4);
}

menu::menu(std::vector<std::string> title, std::vector<std::string> options) : title(title), options(options)
{
	this->content.resize(options.size());
	int max_y = getmaxy(stdscr);
	int max_x = getmaxx(stdscr);
	path[0] = '\0'; // initialize path buffer
	this->current_option = 0;

	this->window = newwin(max_y, max_x, 0, 0);
	keypad(this->window, TRUE);
	box(this->window, 0, 0);

	// prints the title
	for (size_t i = 0; i < this->title.size(); i++)
		mvwprintw(this->window, i + 1, (max_x - this->title[i].length()) / 2, this->title[i].c_str());

	this->draw_options();

	// creates the content window
	this->content_window = derwin(this->window, max_y - 12, max_x - 4, 11, 2);
	box(this->content_window, 0, 0);
}

void menu::run()
{
	while (running)
	{
		this->draw_options();
		// draws the content window based on the highlited option
		this->display();
		this->drawContentWindow();
		this->display();

		int ch = wgetch(this->window);
		switch (ch)
		{
		case KEY_LEFT:
		case 'a':
			if (this->current_option > 0)
				this->current_option--;
			else
				this->current_option = this->options.size() - 1;
			break;
		case KEY_RIGHT:
		case 'd':
			if (this->current_option < (int)this->options.size() - 1)
				this->current_option++;
			else
				this->current_option = 0;
			break;
		case 10: // enter key
			if (this->options[this->current_option] == "Exit")
				running = false;
			break;
		}
	}
}
