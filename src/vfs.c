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

#ifdef PLATFORM_WINDOWS
#include <io.h>
#include <dirent.h>
#define F_OK 0
#define access _access
#define chdir _chdir
#else
#include <unistd.h>
#endif

bool jeff_set_working_dir(const char *path) {
    return chdir(path) == 0;
}

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
    return !access(path, F_OK);
#endif
}

unsigned char *vfs_read(const char *filename, size_t *size) { // must free() after use
#ifndef JEFF_NO_VFS
    return NULL;
#else
    unsigned char *result = NULL;
    size_t sz = -1;
    FILE *fh = fopen(filename, "rb");
    if (!fh)
        goto BAIL;
    fseek(fh, 0, SEEK_END);
    if (!(sz = ftell(fh)))
        goto BAIL;
    fseek(fh, 0, SEEK_SET);
    if (!(result = malloc(sz * sizeof(unsigned char))))
        goto BAIL;
    if ((fread(result, sz, 1, fh) != sz)) {
        free(result);
        goto BAIL;
    }
BAIL:
    if (fh)
        fclose(fh);
    if (size)
        *size = sz;
    return result;
#endif
}

bool vfs_write(const char *filename, unsigned char *data, size_t size) {
#ifndef JEFF_NO_VFS
    return false;
#else
    bool result = false;
    FILE *fh = fopen(filename, "wb");
    if (!fh)
        return false;
    if ((fwrite(data, size, 1, fh) != size))
        goto BAIL;
    result = true;
BAIL:
    if (fh)
        fclose(fh);
    return result;
#endif
}
