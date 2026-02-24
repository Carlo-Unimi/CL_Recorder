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

int menu::printDevices()
{
	int maxLen = 0;

	for (size_t i = 0; i < this->devices.size(); i++)
	{
		mvwprintw(this->content_window, 5 + i, 3, this->devices[i].description.c_str());
		if ((int)this->devices[i].description.length() > maxLen)
			maxLen = this->devices[i].description.length();
	}

	// draw pipes after the device list
	for (size_t i = 0; i < this->devices.size(); i++)
		mvwprintw(this->content_window, 5 + i, 4 + maxLen, "| [ ]");

	return maxLen;
}

void menu::drawContentWindow()
{
	wclear(this->content_window);
	box(this->content_window, 0, 0);
	int sel, maxLen;
	switch (this->current_option)
	{
	//* recording settings
	case 0:
		this->content[0].content = {"Configure your recording settings here:"};

		mvwprintw(this->content_window, 3, 2, "Channels:     [%d]", this->recorder.numChannels);
		mvwprintw(this->content_window, 4, 2, "Sample Rate:  [%d Hz]", this->recorder.sampleRate);
		mvwprintw(this->content_window, 5, 2, "Format:       [16-bit PCM]");
		mvwprintw(this->content_window, 6, 2, "");
		mvwprintw(this->content_window, 7, 2, "Select option:");
		mvwprintw(this->content_window, 8, 2, "[1] Change number of Channels ");
		mvwprintw(this->content_window, 9, 2, "[2] Change Sample Rate");

		printContent();
		break;

	//* device selection
	case 1:
		this->content[1].content = {"Select an input device from the available options:", "", "[use arrow keys to select]"};
		printContent();

		maxLen = printDevices();

		// select the current device
		for (size_t i = 0; i < this->devices.size(); i++)
		{
			if (this->devices[i].selected)
				mvwprintw(this->content_window, 5 + i, maxLen + 7, "X");
		}
		break;

	//* file path input
	case 2:
		this->content[2].content = {"Specify file paths for saving recordings:", "[default: ~/recordings/]", "", "press enter to modify"};
		printContent();

		if (path[0] != '\0')
			mvwprintw(this->content_window, getmaxy(this->content_window) - 2, 2, "Current path: %s", this->path);
		break;

	//* exit option
	case 3:
		this->content[3].content = {"Exit option selected.", "", "Press Enter to exit the program."};
		printContent();
		break;

	//* recording option
	case 4:
		if (this->options[this->current_option] == "START RECORDING")
		{
			this->content[4].content = {"START RECORDING option selected.", "", "Press Enter to start recording."};
		}
		else if (this->options[this->current_option] == "STOP RECORDING ")
		{
			this->content[4].content = {"RECORDING IN PROGRESS...", "", "Press Enter to stop recording.", "", "VU Meters:"};

			// draw VU meters if recording
			if (this->recorder.isRecording())
			{
				std::vector<float> levels = this->recorder.getChannelLevels();
				if (!levels.empty())
				{
					vu_window::drawVUMeters(this->content_window, levels, getmaxy(this->content_window) - 2);
				}
			}
		}
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
	this->running = true;
	this->devices = AudioManager::list_input_devices();

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

menu::~menu()
{
	delwin(this->content_window);
	delwin(this->window);
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

		// set 5 updates per seocond when recording for VU meter refresh
		if (this->recorder.isRecording() && this->current_option == 4)
		{
			wtimeout(this->window, 200);
		}
		else
		{
			wtimeout(this->window, -1);
		}

		int ch = wgetch(this->window);

		// if no key pressed, continue to refresh
		if (ch == ERR)
			continue;

		switch (ch)
		{

		//* modify number of channels that are being recorded (1-32)
		case '1':
			if (this->current_option == 0)
			{
				char buf[16] = {0};
				mvwprintw(this->content_window, 12, 2, "Enter number of channels: ");
				wrefresh(this->content_window);
				curs_set(1);
				echo();
				wgetnstr(this->content_window, buf, 6);
				noecho();
				curs_set(0);
				int val = atoi(buf);
				if (val > 0 && val <= 32)
				{
					this->recorder.numChannels = (unsigned int)val;
				}
				else
				{
					mvwprintw(this->content_window, 11, 2, "Invalid channel number");
				}
				break;
			}
			break;

		//* modify sample rate (4000 < sR < 384000)
		case '2':
			if (this->current_option == 0)
			{
				char buf[16] = {0};
				mvwprintw(this->content_window, 11, 2, "[suggested: 44100 Hz, 48000 Hz, 96000 Hz]");
				mvwprintw(this->content_window, 12, 2, "Enter sample rate (Hz): ");
				wrefresh(this->content_window);
				curs_set(1);
				echo();
				wgetnstr(this->content_window, buf, 10);
				noecho();
				curs_set(0);
				int val = atoi(buf);
				if (val > 0 && val <= 384000)
				{
					this->recorder.sampleRate = (unsigned int)val;
				}
				else
				{
					mvwprintw(this->content_window, 11, 2, "Invalid sample rate");
				}
			}
			break;

		//* navigate left - options (left arrow - 'a')
		case KEY_LEFT:
		case 'a':
			if (this->current_option > 0)
				this->current_option--;
			else
				this->current_option = this->options.size() - 1;
			break;

		//* navigate right - options (right arrow - 'd')
		case KEY_RIGHT:
		case 'd':
			if (this->current_option < (int)this->options.size() - 1)
				this->current_option++;
			else
				this->current_option = 0;
			break;

		//* navigate up - devices (up arrow - 'w')
		case KEY_UP:
		case 'w':
			if (this->current_option == 1 && !this->devices.empty())
			{
				int sel = -1;
				for (size_t i = 0; i < this->devices.size(); i++)
					if (this->devices[i].selected)
						sel = i;

				if (sel != -1)
					this->devices[sel].selected = false;

				if (sel > 0)
					sel--;
				else
					sel = this->devices.size() - 1;

				this->devices[sel].selected = true;
			}
			break;

		//* navigate down - devices (down arrow - 's')
		case KEY_DOWN:
		case 's':
			if (this->current_option == 1 && !this->devices.empty())
			{
				int sel = -1;
				for (size_t i = 0; i < this->devices.size(); i++)
					if (this->devices[i].selected)
						sel = i;

				if (sel != -1)
					this->devices[sel].selected = false;

				if (sel < (int)this->devices.size() - 1)
					sel++;
				else
					sel = 0;

				this->devices[sel].selected = true;
			}
			break;

		//* select option (enter key)
		case 10:
			if (this->options[this->current_option] == "Exit")
			{
				running = false;
				if (recorder.isRecording()) // if recording is in progress, stop it before exiting
					recorder.stop();
			}
			else if (this->current_option == 2)
			{
				mvwprintw(this->content_window, 5, 2, "-> ");
				curs_set(1);
				echo();
				wgetnstr(this->content_window, this->path, 50);
				noecho();
				curs_set(0);
				this->content[2].content.push_back("path set to: " + std::string(this->path));
			}
			else if (this->options[this->current_option] == "START RECORDING")
			{
				std::string selectedDevice;
				for (const auto &dev : devices)
				{
					if (dev.selected)
					{
						selectedDevice = dev.name;
						break;
					}
				}
				if (!selectedDevice.empty())
				{
					if (recorder.start(selectedDevice, std::string(this->path)))
					{
						this->options[this->current_option] = "STOP RECORDING ";
						this->content[this->current_option].content = {"RECORDING IN PROGRESS...", "", "Press Enter to stop recording."};
					}
				}
			}
			else if (this->options[this->current_option] == "STOP RECORDING ")
			{
				recorder.stop();
				this->options[this->current_option] = "START RECORDING";
				this->content[this->current_option].content = {"START RECORDING option selected.", "", "Press Enter to start recording."};
			}
			break;
		}
	}
}
