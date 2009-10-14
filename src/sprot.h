#ifndef INCLUDED_SPROT_H
#define INCLUDED_SPROT_H

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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <vector>
#include <string>

class sprot_file
{
 public:
  sprot_file(const std::string& flavor, const std::vector<std::string>& files,
	      const std::string& template_file);
  ~sprot_file();

  int run();

 private:
};

class sprot_project
{
 public:
  sprot_project(const std::string& project_name, const std::vector<std::string>& names,
		 const std::string& template_file);
  ~sprot_project();

  int run();
 private:
};

class sprot_list
{
 public:

  sprot_list(const std::string& template_file);
  ~sprot_list();

  int run();

 private:

  std::string filename;
};

class sprot_write_binary
{
 public:
  sprot_write_binary(const std::vector<std::string>& filenames);
  ~sprot_write_binary();

  int run();
 private:
};

class sprot_set
{
 public:
  sprot_set(const std::string& set_name, const std::vector<std::string>& names,
	    const std::string& template_file);
  ~sprot_set();

  int run();
 private:
};


#endif /* INCLUDED_SPROT_H */

