/*
 *  tetra-kit
 *  Copyright (C) 2020  LarryTh <dev@logami.fr>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <cstdarg>
#include "window.h"
#include "cid.h"
#include "call_identifier.h"

#ifdef WITH_NCURSES

#include <ncurses.h>

static WINDOW * wn_top;                                                         // ncurses windows
static WINDOW * wn_middle;
static WINDOW * wn_bottom;

/*
 * screen functions
 *
 */

void scr_init()
{
    // ncurses screen
    initscr();

    wn_top    = subwin(stdscr, 3,             COLS, 0,         0);              // define windows dimensions
    wn_middle = subwin(stdscr, LINES / 2 - 3, COLS, 3,         0);
    wn_bottom = subwin(stdscr, LINES / 2,     COLS, LINES / 2, 0);

    scrollok(wn_middle, TRUE);                                                  // automatic scroll for middle and bottom windows
    scrollok(wn_bottom, TRUE);

    box(wn_bottom, ACS_VLINE, ACS_HLINE);                                       // draw outline

    wrefresh(wn_top);
    wrefresh(wn_middle);
    wrefresh(wn_bottom);
}


void scr_update(const char * info)
{
    wclear(wn_bottom);
    box(wn_bottom, ACS_VLINE, ACS_HLINE);

    wprintw(wn_middle, "%s", info);

    int cnt = 0;
    while (true)
    {
        call_identifier_t * cid = get_cid(cnt);
        cnt++;

        if (cid == NULL) break;

        if (cid->m_data_received > 0.)
        {
            mvwprintw(wn_bottom, cnt + 2, 2, "CID [Usage] = %06u [%02u] (%.0f kB) file %s, SSI =",
                      cid->m_cid,
                      cid->m_usage_marker,
                      cid->m_data_received,
                      cid->m_file_name[cid->m_usage_marker].c_str());
        }
        else
        {
            mvwprintw(wn_bottom, cnt + 2, 2, "CID [Usage] = %06u [%02u], SSI =",
                      cid->m_cid,
                      cid->m_usage_marker);
        }

        for (size_t idx = 0; idx < cid->m_ssi.size(); idx++)       // print cid ssi
        {
            wprintw(wn_bottom, " %08u", cid->m_ssi[idx].ssi);
        }
    }

    wrefresh(wn_top);
    wrefresh(wn_middle);
    wrefresh(wn_bottom);
}


void scr_clean()
{
    // clean data
    delwin(wn_top);
    delwin(wn_middle);
    delwin(wn_bottom);
    endwin();
}


void scr_print_bottom(int row, int col, const char *fmt, ...)
{
    // print informations to bottom window
    va_list args;
    va_start(args, fmt);

    mvwprintw(wn_middle, row, col, fmt, args);
    wrefresh(wn_bottom);

    va_end(args);
}



void scr_print_middle(const char *fmt, ...)
{
    // print informations to middle window
    va_list args;
    va_start(args, fmt);

    wprintw(wn_middle, fmt, args);
    wrefresh(wn_middle);

    va_end(args);
}

#else

/*
 * Functions with no ncurses, print to screen only
 */

void scr_init()
{

}


void scr_update(const char * info)
{
    printf("%s", info);

    int cnt = 0;
    while (true)
    {
        call_identifier_t * cid = get_cid(cnt);
        cnt++;

        if (cid == NULL) break;

        if (cid->m_data_received > 0.)
        {
            printf("CID [Usage] = %06u [%02u] (%.0f kB) file %s, SSI =",
                      cid->m_cid,
                      cid->m_usage_marker,
                      cid->m_data_received,
                      cid->m_file_name[cid->m_usage_marker].c_str());
        }
        else
        {
            printf("CID [Usage] = %06u [%02u], SSI =",
                      cid->m_cid,
                      cid->m_usage_marker);
        }

        for (size_t idx = 0; idx < cid->m_ssi.size(); idx++)       // print cid ssi
        {
            printf(" %08u", cid->m_ssi[idx].ssi);
        }

        printf("\n");
    }
}


void scr_clean()
{

}


void scr_print_bottom(int row, int col, const char *fmt, ...)
{
    // print informations to bottom window
    va_list args;
    va_start(args, fmt);

    printf(fmt, args);
    printf("\n");

    va_end(args);
}



void scr_print_middle(const char *fmt, ...)
{
    // print informations to middle window
    va_list args;
    va_start(args, fmt);

    printf(fmt, args);

    va_end(args);
}

#endif
