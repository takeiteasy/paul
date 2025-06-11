/* jeff/vfs.c -- https://github.com/takeiteasy/jeff

 Copyright (C) 2025  George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include "jeff.h"
#ifndef JEFF_NO_VFS
#undef SWAP
#define ZIP_C
#include "zip.h"
#define DIR_C
#include "dir.h"
#include "garry.h"
#include "table.h"
#endif

#ifndef JEFF_NO_VFS
typedef struct vfs_dir {
    const char *path;
    zip *archive;
} vfs_dir;

// TODO: Thread safety
static table_t _vfs = {0};

void init_vfs(void) {
    _vfs = table_new();
}
#endif

void vfs_mount(const char *path) {
#ifndef JEFF_NO_VFS
#endif
}

bool vfs_exists(const char *filename) {
#ifndef JEFF_NO_VFS
    return false;
#else
    return paul_path_exists(filename);
#endif
}

unsigned char *vfs_read(const char *filename, size_t *size) { // must free() after use
#ifndef JEFF_NO_VFS
    return NULL;
#else
    return paul_read_file(filename, size);
#endif
}

bool vfs_write(const char *filename, unsigned char *data, size_t size, bool overwrite) {
#ifndef JEFF_NO_VFS
    return false;
#else
    return paul_write_file(filename, data, size, overwrite);
#endif
}
