#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sysdep.h"

#include <windows.h>
#include <shlobj.h>

bool sysdep_can_create_dir_or_file(const std::string& dirname)
{
  if (GetFileAttributesA(dirname.c_str()) == INVALID_FILE_ATTRIBUTES) 
    return true;

  return false;
}

void sysdep_mkdir(const std::string& name)
{
  CreateDirectoryA(name.c_str(), NULL);
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
  char buf[MAX_PATH];

#if 1  // works under win98se
  if (SUCCEEDED(SHGetSpecialFolderPathA(NULL, buf, CSIDL_PERSONAL,FALSE)))
#else  // newer version for NT only
  if (SUCCEEDED(SHGetFolderPath(NULL,CSIDL_PERSONAL,NULL,0,buf)))
#endif
    {
      return std::string(buf);
    }
  else
    {
      return std::string();
    }
}
