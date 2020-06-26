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
#include <cstdint>
#include <cstdarg>
#include <string>
#include "window.h"
#include "cid.h"
#include "call_identifier.h"

#ifdef WITH_NCURSES

#include <ncurses.h>

static WINDOW * wn_top;                                                         // ncurses windows
static WINDOW * wn_infos;
static WINDOW * wn_sds;                                                         // window for SDS messages
static WINDOW * wn_bottom;

/*
 * screen functions
 *
 */

void scr_init()
{
    // ncurses screen
    initscr();

    wn_top    = subwin(stdscr, 3,             COLS, 0,             0);          // define windows dimensions
    wn_infos  = subwin(stdscr, LINES / 4 - 3, COLS, 3,             0);
    wn_sds    = subwin(stdscr, LINES / 4,     COLS, LINES / 4,     0);
    wn_bottom = subwin(stdscr, LINES / 2,     COLS, LINES / 2,     0);

    scrollok(wn_infos,  TRUE);                                                  // automatic scroll
    scrollok(wn_sds,    TRUE);
    scrollok(wn_bottom, TRUE);

    box(wn_bottom, ACS_VLINE, ACS_HLINE);                                       // draw outline

    wrefresh(wn_top);
    wrefresh(wn_infos);
    wrefresh(wn_sds);
    wrefresh(wn_bottom);
}


void scr_update(string info)
{
    // update middle and bottom window
    wclear(wn_bottom);
    box(wn_bottom, ACS_VLINE, ACS_HLINE);

    const size_t TXT_LENGTH = 200;                                              // limit line length in infos window
    string txt = info;
    if (txt.size() > TXT_LENGTH)
    {
        txt = txt.substr(0, TXT_LENGTH);
    }    
    wprintw(wn_infos, "%s\n", txt.c_str());

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
    wrefresh(wn_infos);
    wrefresh(wn_sds);
    wrefresh(wn_bottom);
}


void scr_clean()
{
    // clean data
    delwin(wn_top);
    delwin(wn_infos);
    delwin(wn_sds);
    delwin(wn_bottom);
    endwin();
}


void scr_print_sds(string msg)
{
    wprintw(wn_sds, "%s\n", msg.c_str());
    wrefresh(wn_sds);
}


void scr_print_bottom(int row, int col, string msg)
{
    // print informations to bottom window
    mvwprintw(wn_infos, row, col, "%s", msg.c_str());
    wrefresh(wn_bottom);
}


void scr_print_infos(string msg)
{
    // print informations to middle window
    wprintw(wn_infos, "%s\n", msg.c_str());
    wrefresh(wn_infos);
}

#else

/*
 * Functions with no ncurses, print to screen only
 */

void scr_init()
{

}


void scr_update(string info)
{
    printf("%s", info.c_str());

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


void scr_print_sds(string msg)
{
    printf("%s\n", msg.c_str());
}


void scr_print_bottom(int row, int col, string msg)
{
    printf("%s\n", msg.c_str());
}


void scr_print_infos(string msg)
{
    printf("%s\n", msg.c_str());
}

#endif
