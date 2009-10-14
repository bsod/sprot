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


#include "sprot.h"

#include <algorithm>
#include <complex>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/random.hpp>
#include <boost/thread.hpp>

namespace ublas = boost::numeric::ublas;


//
// sprot_list - List contents of template file
//

sprot_list::sprot_list(const std::string& template_file)
  : filename(template_file)
{
}

sprot_list::~sprot_list()
{
}

int sprot_list::run()
{
  std::cout << "Listing contents of sprot template file `" << filename << "'" << std::endl;

  return 0;
}
