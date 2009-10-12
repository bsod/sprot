#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define _(msg) msg /* no i18n yet */


#include "sprot.h"
#include "sysdep.h"
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iomanip>

#include <cassert>

#include <getopt.h>




// default template file
#if _WIN32
#define DEFAULT_TEMPLATE_FILE "default.sprot"
#else
#define DEFAULT_TEMPLATE_FILE ".default.sprot"
#endif



// option variables
bool        option_list = false;
bool        option_write_binary = false;
std::string option_template_file;
bool        option_project_mode = false;
std::string option_project_name; 
std::string option_flavor;
// snippet mode
bool option_snippet_mode = false;
std::string option_snippet_name;



static void list_sprotfile_contents(sprot_datafile& datafile);

static int  generate_project_initial(sprot_datafile& datafile, 
				     const std::string& project, 
				     const std::string& name);
static int  generate_file_initial(sprot_datafile& datafile, 
				  const std::string& flavor, 
				  const std::string& name);
static int generate_snippet_to_stdout(sprot_datafile& datafile,
				      const std::string& snippet_name);

static int write_binary(const std::string& filename);


static bool option_do_git = false;

void initialize_git_repository(const std::string& project_name, const std::string& user_name, const std::string& user_email)
{
  if (!option_do_git) return;

  chdir(project_name.c_str());

  system("git init");
  system("git add .");
  std::string s = std::string("git config user.name \"")+user_name+"\"";
  system(s.c_str());

  s = std::string("git config user.email \"")+user_email+"\"";
  system(s.c_str());

  system("git commit -mInitRev");
}


// program entry point
int main(int argc, char* argv[])
{
  int option_index = 0, c, i;
  char* progname = argv[0];
  static const char* getopt_string = "gvhlt:p:f:bs:";
  static struct option long_options[] =
    {
      {"version",       0,0,'v'},
      {"help",          0,0,'h'},
      {"list",          0,0,'l'},
      {"template-file", 1,0,'t'},
      {"project",       1,0,'p'},
      {"flavor",        1,0,'f'},
      {"write-binary",  0,0,'b'},
      {"snippet",       1,0,'s'},
      {"git",           0,0,'g'},
      {0,0,0,0}
    };

  // select default template file in homedir
  std::string homedir = sysdep_get_homedir();
  option_template_file = homedir + "/" + DEFAULT_TEMPLATE_FILE;
  
  while ((c = getopt_long(argc, argv, getopt_string, long_options, &option_index)) != EOF)
    {
      switch (c)
	{
	case 'v':		/* version */
	  std::cout 
	    << progname
	    << " (GNU "
	    << PACKAGE_NAME 
	    << ") "
	    << PACKAGE_VERSION
	    << "\n\n"
	    << _(
		 "Copyright (C) 2009 Bert van der Weerd.\n"
		 "This is free software; see the source for copying conditions.  There is NO\n"
		 "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
		 "\nThis program is dedicated to the loving memory of KaleGek, my friend.\n"
		 )
	    ;
	  return 0;
	                  
	case 'h':		/* help */
	help:
	  std::cout 
	    << 
	    "Generate text source files and projects from a standard template.\n"
	    "\n"
	    "Usage: sprot [-f FLAVOR] FILE...\n"
	    "       sprot -l\n"
	    "       sprot -p PROJECT NAME...\n"
	    "       sprot -b FILE...\n"
	    "       sprot -s SNIPPET\n"
	    "\n"
	    "  -g, --git                     Create git repository, set $(name) and $(email), check in files.\n"
	    "\n"
	    "  -b,--write-binary             Write input files to standard output as a hexadecimal dump\n"
	    "  -f FLAVOR, --flavor=FLAVOR    Specify template flavor for a specific template extension\n"
	    "  -h, --help                    Display this help and exit\n"
	    "  -l, --list                    Display contents of the template file\n"
	    "  -p PROJECT, --project=PROJECT Select PROJECT from template file and generate it under name of FILE\n"
            "  -s SNIPPET, --snippet=SNIPPET Print SNIPPET on stdout.\n"
	    "  -t FILE, --template-file=FILE Use FILE as template file (works with any sprot invocation)\n"
	    "  -v, --version                 Display version information and exit\n"
	    "\n"
	    "Please report bugs and ideas to <"
	    << PACKAGE_BUGREPORT 
	    << ">.\n"
	    ;
	  return 0;

	case 'g':
	  option_do_git = true;
	  break;
	                  
	case 'l':		// list
	  option_list = true;
	  break;

	case 'b':
	  option_write_binary = true;
	  break;

	case 't':
	  option_template_file = std::string(optarg);
	  break;
	  
	case 'p':
	  option_project_mode = true;
	  option_project_name = std::string(optarg);
	  break;

	case 's':
	  option_snippet_mode = true;
	  option_snippet_name = std::string(optarg);

	case 'f':
	  option_flavor = std::string(optarg);
	  break;

	default:		/* getopt() returned invalid value */
	  return 1;
	}
    }


  if (option_list && option_write_binary)
    {
      std::cerr << progname << _(": Cannot specify option -b and -l at the same time.") << std::endl;
      return 1;
    }

  if (option_write_binary)
    {
      if (optind == argc)
	{
	  std::cerr << progname << _(": Please specify one or more files to write out as binary.") << std::endl;
	  return 1;
	}
      else
	for (int i = optind; i < argc; i++)
	  if (write_binary(argv[i]) != 0) return 1;

      return 0;
    }


  if (option_list) {
    if (optind != argc) 
      std::cerr 
	<< progname 
	<< _(": Cannot have arguments when specifying -l option (list template file)")
	<< std::endl;
    else 
      {

	sprot_datafile datafile;

	try 
	  {
	    datafile.parse(option_template_file);
	  }
	catch (std::exception& ex)
	  {
	    std::cerr << option_template_file << ": " << ex.what() << std::endl;
	    return 1;
	  }

	list_sprotfile_contents(datafile);

      }
    return 0;
  }


  if (optind == argc && !option_snippet_mode) goto help;


  // read the sprot datafile
  sprot_datafile datafile;

  try 
    {
      datafile.parse(option_template_file);
    }
  catch (std::exception& ex)
    {
      std::cerr << option_template_file << ": " << ex.what() << std::endl;
      return 1;
    }

  // the actual work
  if (option_project_mode)
    {
      for(i = optind; i < argc; i++)
	{
	  int retval = generate_project_initial(datafile,option_project_name,argv[i]);
	  if (retval) return retval;
	}
    }

  else if (option_snippet_mode)
    {
      if (option_snippet_name == "") 
	{
	  std::cerr << progname << ": The snippet name cannot be empty." << std::endl;
	  return 1;
	}
      int retval = generate_snippet_to_stdout(datafile,option_snippet_name);
      return retval;
    }
  else
    {
      for (i = optind; i < argc; i++)
	{
	  int retval = generate_file_initial(datafile,option_flavor,argv[i]);
	  if (retval) return retval;
	}
    }

  return 0;
}







//
// List the contents of the sprot template file
//


void list_sprotfile_contents(sprot_datafile& datafile)
{



  std::cout
    << std::endl
    << "Listing contents of sprot template file: `" << option_template_file << "'" << std::endl
    << std::endl
    << "The template file contains " << datafile.templates.size() 
    << " templates and " << datafile.projects.size() << " projects." << std::endl
    << "There are " << datafile.snippets.size() << " snippets and " 
    << datafile.blobs.size() << " blobs defined." << std::endl
    << std::endl << std::endl;


  // list templates


  if (datafile.templates.size() != 0)
    {

      std::cout
	<< "Listing templates (with optional flavors):" << std::endl << std::endl;


      std::vector<std::string> ext;
      
      for (std::vector<sprot_template>::iterator i = datafile.templates.begin(); i!=datafile.templates.end();i++)
	{
	  for (std::vector<std::string>::iterator j = ext.begin(); j != ext.end(); j++)
	    {
	      if (i->name == *j) goto found;
	    }

	  // not found so insert it
	  ext.push_back(i->name);
	found:;
	}

      // now sort it
      std::sort(ext.begin(),ext.end());
      
      // now print the extensions in order, with optionally the flavors behind them
      {
	for (std::vector<std::string>::iterator i = ext.begin(); i != ext.end(); i++)
	  {
	    std::vector<std::string> flavor;

	    for(std::vector<sprot_template>::iterator j = datafile.templates.begin(); j != datafile.templates.end(); j++)
	      {
		if (*i == j->name) flavor.push_back(j->flavor);
	      }

		std::sort(flavor.begin(),flavor.end());
		
		if (flavor.size() == 1 && flavor[0] == "")
		  std::cout 
		    << "   template: " << std::setfill(' ') << std::setw(6) << std::left << *i << "   (no flavor)"
		    << std::endl;
		else
		  {
		    std::cout
		      << "   template: " << std::setfill(' ') << std::setw(6) << std::left << *i << "  ";
		    
		    for(std::vector<std::string>::iterator k=flavor.begin(); k!=flavor.end(); k++)
		      std::cout << " [" << (*k==""?"no flavor":*k) << "]";
	      
		    std::cout << std::endl;
		  }
	  }
      }
    }



  // list projects


  if (datafile.projects.size() != 0)
    {

      std::cout 
	<< std::endl << "Listing projects:" << std::endl << std::endl;


      std::vector<std::string> proj;

      for (std::vector<sprot_project>::iterator i=datafile.projects.begin(); i!=datafile.projects.end();i++)
	proj.push_back(i->name);

      std::sort(proj.begin(),proj.end());

      {
	for(std::vector<std::string>::iterator i=proj.begin(); i!=proj.end(); i++)
	  {
	    // find the project
	    for (std::vector<sprot_project>::iterator j=datafile.projects.begin(); j!=datafile.projects.end(); j++)
	      {
		if (j->name == *i)
		  {
		    std::vector<std::string> files;
		  
		    for (std::vector<sprot_file>::iterator k=j->files.begin(); k!=j->files.end(); k++)
		      files.push_back(k->name);
		    
		    std::sort(files.begin(),files.end());
		    
		  std::cout 
		    << "   project:  " << *i << " (containing " << files.size() << " files)" << std::endl
		    << "     files:  ";
		  
		  for (std::vector<std::string>::iterator l=files.begin();l!=files.end();l++)
		    {
		      if (l != files.begin()) std::cout << "             ";
		      std::cout << *l;
		      
		      for (std::vector<sprot_file>::iterator m=j->files.begin();m!=j->files.end();m++)
			{
			  if (m->name == *l && m->alias!="") { 
			    std::cout << "   -->  ";
			    if (m->binary) std::cout << "blob"; else std::cout << "snippet";
			    std::cout << " " << m->alias; 
			  }
			  if (m->name == *l && m->binary) { 
			    std::cout << " (binary)"; 
			  }
			}

		      std::cout << std::endl;
		    }
		  
		  std::cout << std::endl << std::endl;
		  }
	      }
	  }
      }
    }


  // list snippets

  if (datafile.snippets.size() > 0)
    {
      std::cout 
	<< std::endl << "Listing snippets:" << std::endl << std::endl;

      std::vector<std::string> snippets;

      for (std::vector<sprot_snippet>::iterator i=datafile.snippets.begin(); i!=datafile.snippets.end();i++)
	snippets.push_back(i->name);

	  std::sort(snippets.begin(),snippets.end());

      for (std::vector<std::string>::iterator j=snippets.begin(); j!=snippets.end(); j++)
	std::cout << "    snippet: " << *j << std::endl;
    }
  

  // list blobs
  
  if (datafile.blobs.size() > 0)
    {
      std::cout
	<< std::endl << "Listing blobs:" << std::endl << std::endl;

      std::vector<std::string> blobs;

      for (std::vector<sprot_blob>::iterator i=datafile.blobs.begin(); i!=datafile.blobs.end();i++)
	blobs.push_back(i->name);
      
	  std::sort(blobs.begin(),blobs.end());
	  
      for (std::vector<std::string>::iterator j=blobs.begin(); j!=blobs.end(); j++)
	std::cout << "       blob: " << *j << std::endl;
    }
  

  std::cout 
    << std::endl << std::endl;
}










//
// generate a project
//




static int generate_project_initial(sprot_datafile& datafile, const std::string& project, const std::string& name)
{
  // first sanity check, does 'name' specify a valid, non-existent directory?
  if (name.find_first_of("\\/") != std::string::npos)
    {
      std::cerr 
	<< name << ": error: The project name cannot be a path, and will always be created in the current directory." << std::endl;
      return 11;
    }
  // next, does the directory already exists?
  if (sysdep_can_create_dir_or_file(name) != true)
    {
      std::cerr
	<< name << ": error: There is already a directory or file with that name. "
	           "sprot wont overwrite it. rename or remove if needed."
	<< std::endl;
      return 12;
    }

  // another sanity check, does the project exist in the datafile?
  for (std::vector<sprot_project>::iterator i = datafile.projects.begin(); 
       i != datafile.projects.end(); 
       i++)
    {
      if (i->name == project)
	{

	  // This function call does the whole file generating thing,
	  // after that, we initialize the git repository.
	  
	  datafile.generate_project(*i, name);
	  std::string user_name = datafile.var["author"];
	  std::string user_email = datafile.var["email"];
	  initialize_git_repository(name,user_name,user_email);
	  return 0;


	}
    }

  std::cerr << "error: Project `" << project << "' not found in the template file." << std::endl;
  return 1;
}











//
// generate a file from a template
//





static int generate_file_initial(sprot_datafile& datafile, const std::string& flavor, const std::string& name)
{

  // first sanity check, does the name specify a valid simple name without slashes etc
  if (name.find_first_of("\\/") != std::string::npos)
    {
      std::cerr
	<< name << ": error: The destination file cannot have a path, "
	"it must be a single name and will be created in the current direcotry." << std::endl;
      return 13;
    }


  // next, does it already exist?
  if (sysdep_can_create_dir_or_file(name) != true)
    {
      std::cerr
	<< name << ": error: Sorry but this file already exists!"
	" sprot will not overwrite existing files. please rename or remove offending file." << std::endl;
      return 14;
    }


  bool ext_found = false;
  bool flavor_found = false;
  std::string filename, ext;


  // another sanity check, does extension and desired flavor exist?
  for (std::vector<sprot_template>::iterator i = datafile.templates.begin(); i != datafile.templates.end(); i++)
    {
      // first split off the extension from the filename
      size_t pos = name.find_last_of(".");

      if (pos == std::string::npos) 
	{
	  std::cerr 
	    << name 
	    << ": error: This filename does not specify an extension, which makes it impossible to match it to a template.." 
	    << std::endl;
	  return 15;
	}

      filename = name.substr(0,pos);
      ext = name.substr(pos);

      // now we got the extension, lets find it in our template list

      if (i->name == ext) 
	{
	  ext_found = true;

	  if (i->flavor == flavor)
	    {
	      flavor_found = true;
	    }

	  if (ext_found && flavor_found)
	    { 
	      datafile.generate_template(*i,filename);
	      return 0;
	    }
	}
    }

  
  if (ext_found && !flavor_found)
    {
      std::cerr
	<< name << ": error: The extension `"<<ext<<"' was found, but not with the matching flavor `"<<flavor<<"'."
	<< std::endl;
      return 17;
    }

  assert(!ext_found);

  std::cerr
    << name << ": error: There was no template found for the `" << ext << "' extension."
    << std::endl;
  return 16;
}



//
// Writing the snippet to STDOUT
//


static int generate_snippet_to_stdout(sprot_datafile& datafile, const std::string& snippet_name)
{
  if (datafile.snippets.size() > 0)
    {
      for (std::vector<sprot_snippet>::iterator i=datafile.snippets.begin(); i!=datafile.snippets.end();i++)
	{
	  if (i->name == snippet_name)
	    {

	      //	      std::cout << "FOUND SNIPPET '" << snippet_name << "' in datafile." << std::endl;

	      datafile.subst_init("");
	      std::cout << datafile.subst(i->content);


	      return 0;
	    }
	}

      std::cout << "Snippet '" << snippet_name << "' not found." << std::endl;
    }
  else
    std::cout << "Datafile does not contain any snippets." << std::endl;

  return 1;
}

//
// Writing a binary
//


static int write_binary(const std::string& filename)
{
  const int rowlen = 25;	// number of hex bytes on one line

  std::ifstream ifs(filename.c_str(),std::ios::binary);

  if (!ifs) { std::cerr << filename << _(": Unable to open file for reading") << std::endl; }

  char c;

  int pos = 1;

  while (ifs.read(&c,1))
    {
      unsigned char uc = (unsigned char) c;

      std::cout << std::setfill('0') << std::setw(2) << std::hex << (unsigned)uc;

      if (pos == rowlen) { std::cout << std::endl; pos = 1; } else { std::cout << ' '; ++pos; }
    }

  std::cout << std::endl;

  ifs.close();

  return 0;
}


