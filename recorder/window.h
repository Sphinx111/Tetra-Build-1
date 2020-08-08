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
#ifndef WINDOW_H
#define WINDOW_H
#include <cstdarg>
#include <string>

using namespace std;

void scr_init(int line_length, int max_bottom_lines);
void scr_update(string info);
void scr_clear();
void scr_print_infos(string msg);
void scr_print_sds(string msg);

#define WITH_NCURSES

#endif /* WINDOW_H */
