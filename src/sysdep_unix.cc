/*
 * Copyright 2009 Bert van der Weerd <bert@superstring.nl>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "sysdep.h"

#include <stdlib.h>

// this function opens "~/.sprot/sprot.cfg"
void sysdep_t::open_config_file(std::ifstream& ifs)
{
  std::string path = "";

  path += getenv("HOME");
  path += "/.sprot/sprot.cfg";

  ifs.open(path.c_str());
}
