#ifndef vu_window_H
#define vu_window_H

#include <ncurses.h>
#include <vector>

/**
 * @class vu_window
 * @brief draws VU meters for audio channels with color-coded levels.
 */
class vu_window
{
private:
  static bool colorsInitialized;

  /**
   * @brief initializes ncurses color pairs for VU meter display.
   */
  static void initColors();

public:
  /**
   * @brief draws VU meters for all channels in the given window.
   * @param win the ncurses window to draw in.
   * @param levels vector of normalized audio levels (0.0 - 1.0) per channel.
   * @param startY starting row position in the window.
   * @param startX starting column position in the window.
   */
  static void drawVUMeters(WINDOW *win, const std::vector<float> &levels, int endY);
};

#endif