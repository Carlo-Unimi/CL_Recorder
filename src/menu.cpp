#include "../include/menu.h"

void menu::display()
{
	wrefresh(this->window);
	wrefresh(this->content_window);
}

void menu::draw_options()
{
	// prints the options on a single row
	mvwprintw(this->window, this->title.size() + 2, 2, "------------------------------------------------------------");
	int aux = 2;
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
	mvwprintw(this->window, this->title.size() + 4, 2, "------------------------------------------------------------");
	wrefresh(this->window);
}

menu::menu(std::vector<std::string> title, std::vector<std::string> options) : title(title), options(options)
{
	int max_y = getmaxy(stdscr);
	int max_x = getmaxx(stdscr);

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

	this->display();
}

void menu::run()
{
	while (true)
	{
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
			if (this->options[this->current_option] == "Exit") return;
			// free memory and exit the program
			break;
		}
		this->draw_options();
	}
}
