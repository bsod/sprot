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

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <fstream>

// this function opens "$(My Documents)/sprot/sprot.cfg"
void sysdep_t::open_config_file(std::ifstream& ifs)
{
  TCHAR szPath[MAX_PATH];

  if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, szPath)))
    {
      if (PathAppend(szPath, TEXT("sprot")) == FALSE) return;
      if (PathAppend(szPath, TEXT("sprot.cfg")) == FALSE) return;

      ifs.open(szPath);		// trust the right open() is called depending on the TCHAR typedef
    }
}
