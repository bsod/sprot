#ifndef INCLUDED_SPROT_H
#define INCLUDED_SPROT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <exception>
#include <string>
#include <vector>
#include <map>


//
// these classes define the template datafile structure
//


struct sprot_template { int line; std::string name; std::string content; std::string flavor; };
struct sprot_file     { int line; std::string name; std::string content; 
  bool binary; std::string alias; };
struct sprot_project  { int line; std::string name; std::vector<sprot_file> files; };

struct sprot_snippet  { int line; std::string name; std::string content; };
struct sprot_blob     { int line; std::string name; std::string content; };


//
// The main class that defines what goes in a sprot template file and
// the basic parse() and generate() functions to use it.
//

struct sprot_datafile 
{
  std::vector<sprot_template> templates;
  std::vector<sprot_project>  projects;
  std::vector<sprot_snippet>  snippets;
  std::vector<sprot_blob>     blobs;

  // variables and content substitution
  std::map<std::string,std::string> var;

  struct parse_error : public std::exception 
  {
    int line;
    std::string msg;
    
    parse_error(const std::string& msg, const int line = 0)
    {
      this->line = line;
      this->msg = msg;
    }

    // needed because we derive from std::exception
    // http://www.cplusplus.com/doc/tutorial/exceptions.html
    virtual ~parse_error() throw() {}

    virtual const char* what() const throw();
  };
  

  // parse the template file
  void parse(const std::string& filename);

  // generate files and projects
  void generate_template(const sprot_template& template_data, const std::string& file_name);
  void generate_project(const sprot_project& project_data, const std::string& project_name);


private:

public:
  void subst_init(const std::string& name);
  std::string subst(const std::string& str);
};



#endif /* INCLUDED_SPROT_H */
