#include "sysdep.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

bool sysdep_can_create_dir_or_file(const std::string& dirname)
{
  struct stat statbuf;

  if (stat(dirname.c_str(),&statbuf) == -1)
    {
      if (errno == ENOENT) return true;
    }

  return false;
}

void sysdep_mkdir(const std::string& name)
{
  mkdir(name.c_str(),0755);
}

int  sysdep_create_subdirs(const std::string& full_filename)
{
  if (full_filename.find("/") == std::string::npos)
    // no slashes, no need to create the subdirs
    return 0;

  size_t found = full_filename.find_first_of("/");

  while (found!=std::string::npos)
    {
      std::string tmp = full_filename.substr(0,found);
      sysdep_mkdir(tmp);

      // find next '/'
      found = full_filename.find_first_of("/",found+1);
    }

  return 0;
}


std::string sysdep_get_homedir()
{
  return std::string(getenv("HOME"));
}
