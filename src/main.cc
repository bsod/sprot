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
#include "sysdep.h"

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

sysdep_t sysdep;

int
main(int argc,char* argv[])
{
  try
    {
      using namespace boost::program_options;

      using std::string;
      using std::vector;
      using std::cout;
      using std::endl;
      using std::ifstream;

      //
      // parse commandline
      //


      // Declare a group of options that will be 
      // allowed only on command line
      options_description generic("Generic options");
      generic.add_options()
	("version,v", "display version information and exit")
	("help,h", "display this help and exit")
	("list,l", "display contents of the template file")
	;
    
      // Declare a group of options that will be 
      // allowed both on command line and in
      // config file
      options_description config("Configuration");
      config.add_options()
	("template-file,t", value< string >(), "select which template file to use")
	;
      
      // Hidden options, will be allowed both on command line and
      // in config file, but will not be shown to the user.
      //
      // ** this is used for the input files.
      options_description hidden("");
      hidden.add_options()
	("input-file", value< vector<string> >(), "input file")
	;
      options_description cmdline_options;
      cmdline_options.add(generic).add(config).add(hidden);

      options_description config_file_options;
      config_file_options.add(config);

      options_description visible("Allowed options");
      visible.add(generic).add(config);
        
      positional_options_description p;
      p.add("input-file", -1);
        
      variables_map vm;
      store(command_line_parser(argc, argv).
	    options(cmdline_options).positional(p).run(), vm);

      // open config file with more default options
      {
	ifstream ifs;

	sysdep.open_config_file(ifs);

	if (ifs.is_open())
	  store(parse_config_file(ifs, config_file_options), vm);
      }

      notify(vm);


      //
      // do something with the options
      //


      if (vm.count("help")) 
	{
	  cout << 
	    "Generate text source files and projects from a standard template.\n"
	    "\n"
	    "Usage: sprot [-f FLAVOR] FILE...\n"
	    "       sprot -l\n"
	    "       sprot -p PROJECT NAME...\n"
	    "       sprot -b FILE...\n"
	    "       sprot -s SET NAME...\n"
	    "\n"
	       << visible << 
	    "\n"
	    "Please report bugs and ideas to <" << PACKAGE_BUGREPORT << ">.\n";

	  return 0;
	}
      
      if (vm.count("version")) 
	{
	  cout << argv[0] << " (" << PACKAGE_NAME << ") " << PACKAGE_VERSION << "\n"
	    "\n"
	    "Copyright 2009 Bert van der Weerd <" << PACKAGE_BUGREPORT << ">\n"
	    "\n"
	    "This is free software; see the source for copying conditions.  There is NO\n"
	    "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
	    ;

	  return 0;
	}

      if (vm.count("list"))
	{
	  sprot_list instance(vm["template-file"].as<string>());

	  return instance.run();
	}


      /*
      if (vm.count("input-file"))
	{
          sprot instance(vm["input-file"].as< vector<string> >());
	  return instance.run();
	}
      else
	cout << "No input files given." << endl;
      */

    }

  catch (std::exception& e)
    {
      std::cerr 
      	<< std::endl << std::endl
      	<< "Error: " << e.what() << " (std::exception)" << std::endl;
      return 1;
    }

  catch (...)
    {
      std::cerr 
      	<< std::endl << std::endl
      	<< "Error: " << "Unhandled exception of unknown type." << std::endl;
      return 1;
    }

  return 0;
}

