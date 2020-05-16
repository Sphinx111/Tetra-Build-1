#include "window.h"
#include "recorder.h"
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
    
    // for (size_t cnt = 0; cnt < cid_list.size(); cnt++)                    // update caller id list
    // {
    //     if (cid_list[cnt].m_data_received > 0.)
    //     {
    //         mvwprintw(wn_bottom, cnt + 2, 2, "CID [Usage] = %06d [%02d] (%.0f kB) file %s, SSI =",
    //                   cid_list[cnt].m_cid,
    //                   cid_list[cnt].m_usage_marker,
    //                   cid_list[cnt].m_data_received,
    //                   cid_list[cnt].m_file_name[cid_list[cnt].m_usage_marker].c_str());
    //     }
    //     else
    //     {
    //         mvwprintw(wn_bottom, cnt + 2, 2, "CID [Usage] = %06d [%02d], SSI =",
    //                   cid_list[cnt].m_cid,
    //                   cid_list[cnt].m_usage_marker);
    //     }

    //     for (size_t idx = 0; idx < cid_list[cnt].m_ssi.size(); idx++)       // print cid ssi
    //     {
    //         wprintw(wn_bottom, " %08d", cid_list[cnt].m_ssi[idx].ssi);
    //     }
    // }

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
 * Functions with no ncurses
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
    
    // for (size_t cnt = 0; cnt < cid_list.size(); cnt++)                    // update caller id list
    // {
    //     if (cid_list[cnt].m_data_received > 0.)
    //     {
    //         mvwprintw(wn_bottom, cnt + 2, 2, "CID [Usage] = %06d [%02d] (%.0f kB) file %s, SSI =",
    //                   cid_list[cnt].m_cid,
    //                   cid_list[cnt].m_usage_marker,
    //                   cid_list[cnt].m_data_received,
    //                   cid_list[cnt].m_file_name[cid_list[cnt].m_usage_marker].c_str());
    //     }
    //     else
    //     {
    //         mvwprintw(wn_bottom, cnt + 2, 2, "CID [Usage] = %06d [%02d], SSI =",
    //                   cid_list[cnt].m_cid,
    //                   cid_list[cnt].m_usage_marker);
    //     }

    //     for (size_t idx = 0; idx < cid_list[cnt].m_ssi.size(); idx++)       // print cid ssi
    //     {
    //         wprintw(wn_bottom, " %08d", cid_list[cnt].m_ssi[idx].ssi);
    //     }
    // }
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
