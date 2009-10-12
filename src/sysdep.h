#ifndef INCLUDED_SYSDEP_H
#define INCLUDED_SYSDEP_H

#include <string>

bool        sysdep_can_create_dir_or_file(const std::string& dirname); 
void        sysdep_mkdir(const std::string& name);
int         sysdep_create_subdirs(const std::string& full_filename);
std::string sysdep_get_homedir();

#endif /* INCLUDED_SYSDEP_H */


