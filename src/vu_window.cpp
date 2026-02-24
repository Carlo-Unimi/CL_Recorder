#include "../include/vu_window.h"

bool vu_window::colorsInitialized = false;

void vu_window::initColors()
{
  if (colorsInitialized)
    return;

  if (has_colors())
  {
    start_color();
    use_default_colors();

    // define color pairs for VU meter levels
    init_pair(1, COLOR_GREEN, -1);  // low level (green)
    init_pair(2, COLOR_YELLOW, -1); // medium level (yellow)
    init_pair(3, COLOR_RED, -1);    // high level (red)
    init_pair(4, COLOR_WHITE, -1);  // no signal (white/dim)

    colorsInitialized = true;
  }
}

void vu_window::drawVUMeters(WINDOW *win, const std::vector<float> &levels, int endY)
{
  initColors();

  const int barWidth = 20; // width of VU meter bar

  for (size_t ch = 0; ch < levels.size() && 7 + ch < endY; ++ch)
  {
    int y = 7 + ch;
    float level = levels[ch];

    if (level < 0.0f)
      level = 0.0f;
    if (level > 1.0f)
      level = 1.0f;

    // calc number of filled blocks based on level
    int filledBlocks = static_cast<int>(level * barWidth);

    // channel label
    mvwprintw(win, y, 2, "CH%02zu ", ch + 1);
    mvwprintw(win, y, 7, "[");

    // draw meter bar
    for (int i = 0; i < barWidth; ++i)
    {
      if (i < filledBlocks)
      {
        float posRatio = static_cast<float>(i) / barWidth;
        int colorPair;
        if (posRatio < 0.8f)
          colorPair = 1;
        else
          colorPair = 3;

        wattron(win, COLOR_PAIR(colorPair) | A_BOLD);
        mvwprintw(win, y, 8 + i, "=");
        wattroff(win, COLOR_PAIR(colorPair) | A_BOLD);
      }
      else
      {
        mvwprintw(win, y, 8 + i, " ");
      }
    }

    mvwprintw(win, y, 8 + barWidth, "]");

    // draw percentage text
    int percentage = static_cast<int>(level * 100);
    mvwprintw(win, y, 10 + barWidth, "%3d%%", percentage);
  }
}
