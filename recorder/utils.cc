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
#include "utils.h"


std::string format_str(const char *fmt, ...)
{
    // variadic format function to string
    const std::size_t BUF_LEN = 8192;
    char buf[BUF_LEN];

    va_list args;
    va_start(args, fmt);

    vsnprintf(buf, BUF_LEN - 1, fmt, args);

    va_end(args);

    return std::string(buf);
}
