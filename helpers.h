//
//  helpers.h
//  re3-installer
//
//  Created by Tatsh on 2020-10-09.
//

#ifndef helpers_h
#define helpers_h

#include <stdbool.h>

bool is_file(const char *);
bool is_directory(const char *);
bool is_iso(const char *);
bool ends_with_dll(const char *);
bool ends_with_exe(const char *);
bool ends_with_url(const char *);

#endif /* helpers_h */
