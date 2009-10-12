// TODO
// bug 1) what if the blob name in a %binary blob% statement doesn't exist?


#include "sprot.h"
#include "sysdep.h"

#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>

#include <cassert>
#include <cctype>
#include <ctime>



enum prev_type { 
  prev_was_none, 
  prev_was_template, 
  prev_was_project, 
  prev_was_file, 
  prev_was_binary ,
  prev_was_snippet,
  prev_was_blob
};


const char* sprot_datafile::parse_error::what() const throw()
{
  std::stringstream err_msg;
  
  if (line != 0)
    err_msg << line << ": error : " << msg;
  else
    err_msg << "error :" << msg;
  
  return err_msg.str().c_str();
}


// we need by_line to have our own operator>>()
struct by_line : std::string { };
std::istream& operator>>(std::istream &is, by_line &to_get)
{
  std::getline(is, to_get);
  return is;
}



static std::stringstream file_lines;

static void normal_line(const std::string& line)
{
  file_lines << line << std::endl;
}



static std::string parse_argument(std::string::iterator i, 
				  const std::string& line, 
				  const int lineno,
				  const std::string& what)
{
  std::string argument;

  // parse required whitespace
  if (i == line.end() || !isspace(*i)) 
    throw sprot_datafile::parse_error("Whitespace expected after `" + what + "' keyword", lineno);

  while (isspace(*i)) 
    { 
      ++i;
      if (i == line.end()) 
	throw sprot_datafile::parse_error("The `" + what + "' statement requires an argument", lineno);
    }

  // parse argument
  for (;;)
    {
      if (*i == '%') 
	{
	  if (i+1 != line.end() && *(i+1) == '%') { argument += '%'; ++i; }
	  else break;
	}
      else
	argument += *i;

      ++i;

      if (i == line.end()) 
	throw sprot_datafile::parse_error("The `" + what 
					  + "' statement must be terminated with a `%' character", 
					  lineno);
    }

  if (argument.length() == 0)
    throw sprot_datafile::parse_error("Argument is required for " + what, lineno);

  // trim right whitespace
  for (std::string::reverse_iterator rit = argument.rbegin(); rit < argument.rend(); rit++)
    if (!isspace(*rit))
      {
	argument = argument.substr(0, argument.size() - (rit-argument.rbegin()));
	break;
      }

  return argument;
}




// TODO: should be a private member function
static void insert_prev_stmt(sprot_datafile& datafile,
			     prev_type prev_stmt,
			     int prev_stmt_lineno,
			     std::string& prev_stmt_argument,
			     std::string& prev_project_name,
			     std::string& prev_alias
			     )
{

  switch (prev_stmt)
    {

    case prev_was_template:
      {
	sprot_template tmp;
	tmp.line = prev_stmt_lineno;
	tmp.content = file_lines.str();

	// extract the flavor, if there is any

	std::string::iterator p = prev_stmt_argument.begin();

	// the argument must begin with a '.'
	if (*p != '.')
	  throw sprot_datafile::parse_error(
	    "The template argument must begin with a period, as is signifies a file extension", 
	    prev_stmt_lineno);

	if (prev_stmt_argument.size() == 1)
	  throw sprot_datafile::parse_error(
	    "The template argument cannot be a single dot, it should be a file extension",
	    prev_stmt_lineno);

	while(p != prev_stmt_argument.end() && !isspace(*p)) 
	  ++p;

	if (p == prev_stmt_argument.end())
	  { tmp.name = prev_stmt_argument; tmp.flavor=""; }
	else
	  {
	    tmp.name = prev_stmt_argument.substr(0,p-prev_stmt_argument.begin());
	    while (isspace(*p)) ++p;
	    tmp.flavor = prev_stmt_argument.substr(p-prev_stmt_argument.begin(),std::string::npos);
	  }

	
	// prevent insertion of doubles
	for (std::vector<sprot_template>::iterator i = datafile.templates.begin(); 
	     i != datafile.templates.end(); 
	     i++)
	  {
	    if (i->name == tmp.name && i->flavor == tmp.flavor) {
	      std::stringstream tmp2; tmp2 << i->line;

	      throw sprot_datafile::parse_error(
		"Template `" + tmp.name + "' already defined on line " + tmp2.str(),
		tmp.line);
	    }
	  }
	
	datafile.templates.push_back(tmp);
      }
      break;


    case prev_was_project:
      {
	sprot_project tmp;
	tmp.line = prev_stmt_lineno;
	tmp.name = prev_stmt_argument;
	
	// prevent insertion of doubles
	for (std::vector<sprot_project>::iterator i = datafile.projects.begin(); 
	     i != datafile.projects.end(); 
	     i++)
	  {
	    if (i->name == tmp.name) {
	      std::stringstream tmp2; tmp2 << i->line;

	      throw sprot_datafile::parse_error(
		"Project `" + tmp.name + "' already defined on line " + tmp2.str(),
		tmp.line); 
	    }
	  }
	
	datafile.projects.push_back(tmp);
      }
      break;


    case prev_was_file:
      {
	sprot_file tmp;
	tmp.line = prev_stmt_lineno;
	tmp.name = prev_stmt_argument;
	tmp.content = file_lines.str();
	tmp.binary = false;
	tmp.alias = prev_alias;

	// find the project

	bool found_project = false;
	for(std::vector<sprot_project>::iterator i = datafile.projects.begin(); 
	    i!=datafile.projects.end();
	    i++)
	  {
	    if (i->name == prev_project_name)
	      {
		found_project = true;

		// find if the file was not already inserted before
		for(std::vector<sprot_file>::iterator j=i->files.begin(); j!=i->files.end();j++)
		  {
		    if (j->name == tmp.name) {
		      std::stringstream tmp2; tmp2 << j->line;
		      
		      throw sprot_datafile::parse_error(
			"File `" + tmp.name + "' already defined on line " + tmp2.str(),
			tmp.line);
		    }
		  }

		// add it to the project
		i->files.push_back(tmp);
		break;
	      }
	  }

	if (!found_project) {
	  throw sprot_datafile::parse_error(
	    "You must declare a %project% first before declaring a %file%",
	    tmp.line);
	}
      }
      break;


    case prev_was_binary:
      {
	sprot_file tmp;
	tmp.line = prev_stmt_lineno;
	tmp.name = prev_stmt_argument;
	tmp.content = file_lines.str();
	tmp.binary = true;
	tmp.alias = prev_alias;

	// find the project

	bool found_project = false;
	for(std::vector<sprot_project>::iterator i = datafile.projects.begin(); 
	    i!=datafile.projects.end();
	    i++)
	  {
	    if (i->name == prev_project_name)
	      {
		found_project = true;

		// find if the file was not already inserted before
		for(std::vector<sprot_file>::iterator j=i->files.begin(); j!=i->files.end();j++)
		  {
		    if (j->name == tmp.name) {
		      std::stringstream tmp2; tmp2 << j->line;

		      throw sprot_datafile::parse_error(
			"Binary `" + tmp.name + "' already defined on line " + tmp2.str(),
			tmp.line);
		    }
		  }

		// add it to the project
		i->files.push_back(tmp);
		break;
	      }
	  }

	if (!found_project)
	  throw sprot_datafile::parse_error(
	    "You must declare a %project% first before declaring a %binary%",
	    tmp.line);
      }
      break;


    case prev_was_snippet:
      {
	sprot_snippet tmp;

	tmp.line = prev_stmt_lineno;
	tmp.name = prev_stmt_argument;
	tmp.content = file_lines.str();

	// prevent insertion of doubles
	for (std::vector<sprot_snippet>::iterator i = datafile.snippets.begin(); 
	     i != datafile.snippets.end(); 
	     i++)
	  {
	    if (i->name == tmp.name) {
	      std::stringstream tmp2; tmp2 << i->line;
	      
	      throw sprot_datafile::parse_error(
		"Snippet `" + tmp.name + "' already defined on line " + tmp2.str(),
		tmp.line); 
	    }
	  }

	datafile.snippets.push_back(tmp);
      }
      break;


    case prev_was_blob:
      {
	sprot_blob tmp;

	tmp.line = prev_stmt_lineno;
	tmp.name = prev_stmt_argument;
	tmp.content = file_lines.str();

	// prevent insertion of doubles
	for (std::vector<sprot_blob>::iterator i = datafile.blobs.begin(); 
	     i != datafile.blobs.end(); 
	     i++)
	  {
	    if (i->name == tmp.name) {
	      std::stringstream tmp2; tmp2 << i->line;

	      throw sprot_datafile::parse_error(
		"Blob `" + tmp.name + "' already defined on line " + tmp2.str(),
		tmp.line); 
	    }
	  }

	datafile.blobs.push_back(tmp);
      }
      break;


    case prev_was_none:
      break;
    } 

  return;			// this function is one big switch statement
}



void sprot_datafile::parse(const std::string& filename)
{

  // open file into 'ifs'
  std::ifstream ifs(filename.c_str());
  
  if (!ifs) 
    throw parse_error("Cannot open template file for reading");

  // parse 'ifs' line by line into 'v'
  std::vector<std::string> v;

  std::copy(std::istream_iterator<by_line>(ifs),
	    std::istream_iterator<by_line>(),
	    std::back_inserter(v));

  ifs.close();

  // done reading the file into memory, now actually parse it

  file_lines.str("");
  prev_type prev_stmt = prev_was_none;
  int prev_stmt_lineno = 0;
  std::string prev_stmt_argument;
  std::string prev_project_name;
  std::string prev_alias;

  // parse them lines
  int lineno = 0;

  for (std::vector<std::string>::iterator j = v.begin(); j != v.end(); j++)
    {
      std::string line = *j; ++lineno;

      std::string::iterator i = line.begin();

      // empty line?
      if (i == line.end()) {
	normal_line(line); 
	continue; 
      }

      if (*i == '%')
	{
	  // possible start of 'template' 'project' or 'file' statements
	  ++i;
	  if (i == line.end()) { normal_line(line); goto next_line; }
	  if (*i == '%') { normal_line(line); goto next_line; }
	  
	  while (isspace(*i)) 
	    { 
	      ++i;
	      if (i == line.end()) { normal_line(line); goto next_line; }
	    }

	  switch (*i) 
	    {


	    case 't':		// template
	      if (line.compare(i-line.begin(),8,"template") == 0)
		{
		  i += 8;
		  
		  std::string argument = parse_argument(i, line, lineno, "template");

		  // insert the previous statement into datafile structure
		  insert_prev_stmt(*this,
				   prev_stmt,
				   prev_stmt_lineno,
				   prev_stmt_argument,
				   prev_project_name,
				   prev_alias
				   );

		  prev_stmt = prev_was_template;
		  prev_stmt_lineno = lineno;
		  prev_stmt_argument = argument;
		  file_lines.str("");

		}
	      else
		normal_line(line);
	      break;



	    case 'p':		// project
	      if (line.compare(i-line.begin(),7,"project") == 0)
		{
		  i += 7;

		  std::string argument = parse_argument(i, line, lineno, "project");

		  // insert the previous statement into datafile structure
		  insert_prev_stmt(*this,
				   prev_stmt,
				   prev_stmt_lineno,
				   prev_stmt_argument,
				   prev_project_name,
				   prev_alias
				   );

		  prev_stmt = prev_was_project;
		  prev_stmt_lineno = lineno;
		  prev_stmt_argument = argument;
		  file_lines.str("");

		  prev_project_name = argument;
		}
	      else
		normal_line(line);
	      break;



	    case 'f':		// file
	      if (line.compare(i-line.begin(),4,"file") == 0)
		{
		  i += 4;

		  std::string argument = parse_argument(i, line, lineno, "file");

		  // insert the previous statement into datafile structure
		  insert_prev_stmt(*this,
				   prev_stmt,
				   prev_stmt_lineno,
				   prev_stmt_argument,
				   prev_project_name,
				   prev_alias
				   );

		  // optionally parse 'snippet <ident> <filename>' syntax
		  if (argument.compare(0,7,"snippet") == 0)
		    {
		      std::string::iterator s = argument.begin() + 7;

		      if (s == argument.end() || !isspace(*s))
				throw parse_error("whitespace required after 'snippet' modifier",lineno);

		      while (s != argument.end() && isspace(*s)) ++s;

		      if (s == argument.end()) throw parse_error("snippet identifier expected",lineno);

		      size_t ident_begin_pos = s - argument.begin();
			  std::string::iterator ident_begin_pos_iter = s;

		      while (s != argument.end() && !isspace(*s)) ++s;

		      if (s == argument.end()) 
				throw parse_error("whitespace required after snippet identifier",lineno);

			  std::string ident = argument.substr(ident_begin_pos, s-ident_begin_pos_iter);
		      
		      while (s!=argument.end() && isspace(*s)) ++s;
		      
		      if (s == argument.end()) 
				throw parse_error("filename argument required after snippet identifier",lineno);

		      std::string fname = argument.substr(s-argument.begin(),std::string::npos);


		      prev_stmt = prev_was_file;
		      prev_stmt_lineno = lineno;
		      prev_stmt_argument = fname;
		      prev_alias = ident;
		      file_lines.str("");
		    }
		  else
		    {
		      prev_stmt = prev_was_file;
		      prev_stmt_lineno = lineno;
		      prev_stmt_argument = argument;
		      prev_alias = std::string();
		      file_lines.str("");
		    }
		}
	      else
		normal_line(line);
	      break;



	    case 'b':		// binary
	      if (line.compare(i-line.begin(),6,"binary") == 0)
		{
		  i += 6;

		  std::string argument = parse_argument(i, line, lineno, "binary");

		  // insert the previous statement into datafile structure
		  insert_prev_stmt(*this,
				   prev_stmt,
				   prev_stmt_lineno,
				   prev_stmt_argument,
				   prev_project_name,
				   prev_alias
				   );

		  // optionally parse 'blob <ident> <filename>' syntax
		  if (argument.compare(0,4,"blob") == 0)
		    {
		      std::string::iterator s = argument.begin() + 4;

		      if (s == argument.end() || !isspace(*s))
				throw parse_error("whitespace required after 'blob' modifier",lineno);

		      while (s != argument.end() && isspace(*s)) ++s;

		      if (s == argument.end()) throw parse_error("'blob' identifier expected",lineno);

		      size_t ident_begin_pos = s - argument.begin();
			  std::string::iterator ident_begin_pos_iter = s;

		      while (s != argument.end() && !isspace(*s)) ++s;

		      if (s == argument.end()) 
				throw parse_error("whitespace required after blob identifier",lineno);

			  std::string ident = argument.substr(ident_begin_pos, s-ident_begin_pos_iter);
		      
		      while (s!=argument.end() && isspace(*s)) ++s;
		      
		      if (s == argument.end()) 
				throw parse_error("filename argument required after blob identifier",lineno);

		      std::string fname = argument.substr(s-argument.begin(),std::string::npos);

		      
		      prev_stmt = prev_was_binary;
		      prev_stmt_lineno = lineno;
		      prev_stmt_argument = fname;
		      prev_alias = ident;
		      file_lines.str("");
		    }
		  else
		    {
		      prev_stmt = prev_was_binary;
		      prev_stmt_lineno = lineno;
		      prev_stmt_argument = argument;
		      prev_alias = std::string();
		      file_lines.str("");
		    }
		}
	      else if (line.compare(i-line.begin(),4,"blob") == 0) // blob
		{
		  i += 4;

		  std::string argument = parse_argument(i, line, lineno, "blob");

		  // insert the previous statement into datafile structure
		  insert_prev_stmt(*this,
				   prev_stmt,
				   prev_stmt_lineno,
				   prev_stmt_argument,
				   prev_project_name,
				   prev_alias
				   );

		  prev_stmt = prev_was_blob;
		  prev_stmt_lineno = lineno;
		  prev_stmt_argument = argument;
		  file_lines.str("");
		}
	      else
		normal_line(line);
	      break;



	    case 'e':		// end
	      if (line.compare(i-line.begin(),3,"end") == 0)
		{
		  i += 3;
		  
		  // parse optional white until '%' sign
		  while (isspace(*i)) 
		    { 
		      ++i;
		      if (i == line.end()) 
			{ 
			  goto throw_error_0;
			}
		    }
		  if (*i != '%')
		    {
		    throw_error_0:
		      throw parse_error("The `end' statement must terminate with a '%' sign", lineno);
		    }

		  // insert the previous statement into datafile structure
		  insert_prev_stmt(*this,
				   prev_stmt,
				   prev_stmt_lineno,
				   prev_stmt_argument,
				   prev_project_name,
				   prev_alias
				   );

		  prev_stmt = prev_was_none;
		  prev_stmt_lineno = lineno;
		  prev_stmt_argument = "";
		  file_lines.str("");
		}
	      else
		normal_line(line);
	      break;



	    case 'v':		// variable
	      if (line.compare(i-line.begin(),8,"variable") == 0)
		{
		  i += 8;

		  std::string argument = parse_argument(i, line, lineno, "variable");

		  // now that we got the argument, parse it into variable name and variable content

		  std::string::iterator p = argument.begin();

		  while (p != argument.end() && !isspace(*p))
		    ++p;
		  
		  if (p == argument.end())
		    { 
		    throw_error_1: 
		      throw parse_error("%variable% with name but no value", lineno); 
		    }

		  std::string varname = argument.substr(0,p-argument.begin());

		  while (p != argument.end() && isspace(*p))
		    ++p;

		  if (p == argument.end()) 
		    { 
		      goto throw_error_1; 
		    } 

		  std::string varvalue = argument.substr(p-argument.begin(),std::string::npos);

		  
		  // okay we got them in 'varname' and 'varvalue'
		  var[varname] = varvalue;

		  // insert the previous statement into datafile structure
		  insert_prev_stmt(*this,
				   prev_stmt,
				   prev_stmt_lineno,
				   prev_stmt_argument,
				   prev_project_name,
				   prev_alias
				   );

		  prev_stmt = prev_was_none;
		  prev_stmt_lineno = lineno;
		  prev_stmt_argument = "";
		  file_lines.str("");
		}
	      else
		normal_line(line);
	      break;


	    case 's':		// snippet
	      if (line.compare(i-line.begin(),7,"snippet") == 0)
		{
		  i += 7;

		  std::string argument = parse_argument(i, line, lineno ,"snippet");

		  // insert the previous statement into datafile structure
		  insert_prev_stmt(*this,
				   prev_stmt,
				   prev_stmt_lineno,
				   prev_stmt_argument,
				   prev_project_name,
				   prev_alias
				   );

		  prev_stmt = prev_was_snippet;
		  prev_stmt_lineno = lineno;
		  prev_stmt_argument = argument;
		  file_lines.str("");
		}
	      else
		normal_line(line);
	      break;

	    default:
	      normal_line(line);
	    }
	}
      else
	{
	  normal_line(line);
	}
    next_line: ; 
    }
  
  return;
}



// Generating the project

void
sprot_datafile::generate_project(const sprot_project& project_data, const std::string& name)
{
  subst_init(name);
  sysdep_mkdir(name);

  for (std::vector<sprot_file>::const_iterator i = project_data.files.begin(); 
       i != project_data.files.end();
       i++)
    {
      std::string filename = name + "/" + subst(i->name);
      sysdep_create_subdirs(filename);

      if (!i->binary)
	{
	  std::ofstream ofs(filename.c_str());

	  if (i->alias != "")
	    {
	      // find snippet with alias i->alias
	      for( std::vector<sprot_snippet>::iterator j= snippets.begin(); j!=snippets.end(); j++)
		{
		  if (j->name == i->alias) 
		    {
		      ofs << subst(j->content);
		      break;
		    }
		}
	    }
	  else
	    ofs << subst(i->content);

	  ofs.close();
	}
      else
	{
	  std::ofstream ofs(filename.c_str(),std::ios::binary);
	  
	  std::stringstream iss;

	  if (i->alias != "")
	    {
	      // find blob with alias i->alias
	      for( std::vector<sprot_blob>::iterator j= blobs.begin(); j!=blobs.end(); j++)
		{
		  if (j->name == i->alias) 
		    {
		      iss.str(j->content);
		      break;
		    }
		}
	    }
	  else
	    iss.str(i->content);


	  for (;;)
	    {
	      int i; iss >> std::hex >> i;

	      if (iss.eof()) break;

	      char c = (unsigned char) i;
	      ofs.write(&c,1);
	    }

	  ofs.close();
	}
    }

  return;
}

// Generating the file from the template

void
sprot_datafile::generate_template(const sprot_template& template_data, const std::string& name)
{
  subst_init(name);
  
  std::string filename = name + template_data.name;
  sysdep_create_subdirs(filename);

  std::ofstream ofs(filename.c_str());

  ofs << subst(template_data.content);

  ofs.close();
}







//
// The code that performs the actual variable expansion
//



void sprot_datafile::subst_init(const std::string& name)
{

  // set date related variables
  time_t rawtime;
  struct tm* timeinfo;
  char buf[80];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buf,sizeof buf, "%d", timeinfo);
  var["day"] = std::string(buf);

  strftime(buf,sizeof buf, "%B", timeinfo);
  var["month"] = std::string(buf);

  strftime(buf,sizeof buf, "%m", timeinfo);
  var["month_number"] = std::string(buf);

  strftime(buf,sizeof buf, "%Y", timeinfo);
  var["year"] = std::string(buf);
  

  // unchangable builtins
  var["name"] = name;

  std::string tmp = name;
  std::transform(tmp.begin(),tmp.end(),tmp.begin(), toupper);
  var["NAME"] = tmp;
}

std::string sprot_datafile::subst(const std::string& s)
{
  std::string t;

  for (std::string::const_iterator i = s.begin(); i != s.end(); i++)
    {
      switch (*i)
	{

	case '%':
	  {
	    if (i+1 == s.end())
	      t += '%';
	    else if (i[1] == '%') {
	      ++i; 
	      t += '%';
	    }
	    else {
	      // its a command of some sort
	      // TODO

	      t += *i;
	    }
	  }
	  break;

	case '$':
	  {
	    if (i+1 == s.end()) { t += '$'; return t; }
	    if (i[1] == '$')    { ++i; t+= '$'; break; }
	
	    // its a variable substitution
	    if (i[1] == '(')
	      {
		i+=2;
		
		std::string::const_iterator p = i;

		while (i!=s.end() && *i != ')')
		  ++i;

		if (i == s.end()) { t += "$("; t += s.substr(p-s.begin()); return t; }
		
		assert(*i == ')');

		std::string varname = s.substr(p-s.begin(),i-p);

		// hardcoded and user variables
		if (var.find(varname) != var.end())
		  {
		    t += var[varname];
		  }
		else
		  {
		    // is it a snippet?
		    bool found = false;

		    for (std::vector<sprot_snippet>::iterator i = snippets.begin(); 
			 i != snippets.end(); 
			 i++)
		      {
			if (varname == i->name)
			  {
			    found = true;

			    // snippets must be interpreted not verbatim copied, 
			    // so call subst() recursively

			    t += subst(i->content);

			    break;
			  }
		      }

		    if (found) break;

		    // unknown varable. simply echo it back unchanged
		    // TODO option -i implements question 'enter value for $('+varname+'): '
		    t += "$(" + varname + ")";
		  }


		break;
	      }

	    t += *i;
	  }
	  break;

	default:
	  t += *i;
	}
    }

  return t;
}


// eof
