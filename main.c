
#define _POSIX_C_SOURCE 199309L
#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void get_memory_info(long *total, long *used, long *free, long *shared, long *buffers, long *cache, long *available, long *swap_total, long *swap_used, long *swap_free)
{
  FILE *file = fopen("/proc/meminfo", "r");
  if (!file)
  {
    perror("Failed to open /proc/meminfo");
    exit(1);
  }

  char label[256];
  long value;
  long mem_free = 0, temp_buffers = 0, temp_cache = 0, temp_sreclaimable = 0;

  while (fscanf(file, "%s %ld kB", label, &value) != EOF)
  {
    if (strcmp(label, "MemTotal:") == 0)
      *total = value;
    else if (strcmp(label, "MemAvailable:") == 0)
      *available = value;
    else if (strcmp(label, "MemFree:") == 0)
      mem_free = value;
    else if (strcmp(label, "Shmem:") == 0)
      *shared = value;
    else if (strcmp(label, "Buffers:") == 0)
      temp_buffers = value;
    else if (strcmp(label, "Cached:") == 0)
      temp_cache = value;
    else if (strcmp(label, "SReclaimable:") == 0)
      temp_sreclaimable = value;
    else if (strcmp(label, "SwapTotal:") == 0)
      *swap_total = value;
    else if (strcmp(label, "SwapFree:") == 0)
      *swap_free = value;
  }
  fclose(file);

  *buffers = temp_buffers;
  *cache = temp_cache + temp_sreclaimable;
  *free = mem_free;
  *used = *total - mem_free - *buffers - *cache;
  *swap_used = *swap_total - *swap_free;
}

double toMB(long kb)
{
  return kb / 1024.0;
}

double toGB(long kb)
{
  return kb / (1024.0 * 1024);
}

int main()
{
  initscr();
  noecho();
  curs_set(0);
  nodelay(stdscr, TRUE);

  attron(A_BOLD);
  if (can_change_color())
  {
    init_color(COLOR_GREEN, 300, 0, 0);
  }

  if (has_colors())
  {
    start_color();

    // Light Mode
    init_pair(1, COLOR_RED, COLOR_WHITE);
    init_pair(2, COLOR_BLUE, COLOR_WHITE);
    init_pair(3, COLOR_BLACK, COLOR_WHITE);

    // Dark Mode
    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_WHITE, COLOR_BLACK);
  }

  int dark_mode = 0;
  bkgd(COLOR_PAIR(3));
  refresh();

  while (1)
  {
    clear();

    long total, used, free, shared, buffers, cache, available, swap_total, swap_used, swap_free;
    get_memory_info(&total, &used, &free, &shared, &buffers, &cache, &available, &swap_total, &swap_used, &swap_free);

    double percent_used = (used * 100.0) / total;

    int ch = getch();
    if (ch == 'q'){
      break;
    } else if (ch == 'l') {
      dark_mode = 0;
    } else if (ch == 'd') {
      dark_mode = 1;
    }

    bkgd(dark_mode ? COLOR_PAIR(6): COLOR_PAIR(3));
    attron(dark_mode ? COLOR_PAIR(4) : COLOR_PAIR(1));

    mvprintw(LINES - 2, 2, "[Q] Quit  [L] Light Mode  [D] Dark Mode  [R] Refresh");

    attron(dark_mode ? COLOR_PAIR(4) : COLOR_PAIR(1));

    mvprintw(3, 5, "[SYSTEM MEMORY STATUS]");

    attron(dark_mode ? COLOR_PAIR(5) : COLOR_PAIR(2));

    attroff(dark_mode ? COLOR_PAIR(4) : COLOR_PAIR(1));

    mvprintw(4, 5, "--------------------------------------------------------------------------------------");
    mvprintw(5, 5, "| %-25s | %-18s | %-15s | %-15s |", "Metric", "Value - KB", "Value - MB", "Value - GB");
    mvprintw(6, 5, "--------------------------------------------------------------------------------------");

    attron(dark_mode ? COLOR_PAIR(5) : COLOR_PAIR(2));

    mvprintw(7, 5, "| %-25s | %-18ld | %15.2f | %15.2f |", "Total Mem", total, toMB(total), toGB(total));
    mvprintw(8, 5, "| %-25s | %-18ld | %15.2f | %15.2f |", "Used Mem", used, toMB(used), toGB(used));
    mvprintw(9, 5, "| %-25s | %-18ld | %15.2f | %15.2f |", "Free Mem", free, toMB(free), toGB(free));
    mvprintw(10, 5, "| %-25s | %-18ld | %15.2f | %15.2f |", "Shared Mem", shared, toMB(shared), toGB(shared));
    mvprintw(11, 5, "| %-25s | %-18ld | %15.2f | %15.2f |", "Available Mem", available, toMB(available), toGB(available));
    mvprintw(12, 5, "| %-25s | %-18ld | %15.2f | %15.2f |", "Buffers", buffers, toMB(buffers), toGB(buffers));
    mvprintw(13, 5, "| %-25s | %-18ld | %15.2f | %15.2f |", "Cache", cache, toMB(cache), toGB(cache));

    attron(dark_mode ? COLOR_PAIR(4) : COLOR_PAIR(1));

    mvprintw(15, 5, "[SWAP MEMORY STATUS]");

    attron(dark_mode ? COLOR_PAIR(5) : COLOR_PAIR(2));

    attroff(dark_mode ? COLOR_PAIR(4) : COLOR_PAIR(1));

    mvprintw(16, 5, "--------------------------------------------------------------------------------------");
    mvprintw(17, 5, "| %-25s | %-18s | %-15s | %-15s |", "Metric", "Value - KB", "Value - MB", "Value - GB");
    mvprintw(18, 5, "--------------------------------------------------------------------------------------");

    attron(dark_mode ? COLOR_PAIR(5) : COLOR_PAIR(2));

    mvprintw(19, 5, "| %-25s | %-18ld | %15.2f | %15.2f |", "Swap Total", swap_total, toMB(swap_total), toGB(swap_total));
    mvprintw(20, 5, "| %-25s | %-18ld | %15.2f | %15.2f |", "Swap Used", swap_used, toMB(swap_used), toGB(swap_used));
    mvprintw(21, 5, "| %-25s | %-18ld | %15.2f | %15.2f |", "Swap Free", swap_free, toMB(swap_free), toGB(swap_free));

    attroff(dark_mode ? COLOR_PAIR(5) : COLOR_PAIR(2));

    mvprintw(22, 5, "--------------------------------------------------------------------------------------");

    attron(dark_mode ? COLOR_PAIR(4) : COLOR_PAIR(1));

    mvprintw(25, 5, "| %-25s | %9.2f%%   |", "Memory Usage", percent_used);

    refresh();
    struct timespec req = {0, 50000000};
    nanosleep(&req, NULL);
  }

  attroff(A_BOLD);
  clear();
  endwin();
  return 0;
}
