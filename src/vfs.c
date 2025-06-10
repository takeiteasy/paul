/* jeff/vfs.c -- https://github.com/takeiteasy/jeff

 Copyright (C) 2024  George Watson

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

// virtual filesystem (registered directories and/or compressed zip archives).
// - rlyeh, public domain.
//
// - note: vfs_mount() order matters (the most recent the higher priority).

#include "jeff.h"
#undef SWAP
#define ZIP_C
#include "zip.h"
#define DIR_C
#include "dir.h"

#ifdef PLATFORM_WINDOWS
#include <io.h>
#include <dirent.h>
#define F_OK 0
#define access _access
#define chdir _chdir
#else
#include <unistd.h>
#endif

unsigned char* read_file(const char *path, size_t *length) {
    unsigned char *result = NULL;
    size_t sz = -1;
    FILE *fh = fopen(path, "rb");
    if (!fh)
        goto BAIL;
    fseek(fh, 0, SEEK_END);
    sz = ftell(fh);
    fseek(fh, 0, SEEK_SET);

    result = malloc(sz * sizeof(unsigned char));
    fread(result, sz, 1, fh);
    fclose(fh);

BAIL:
    if (length)
        *length = sz;
    return result;
}

static bool file_exists(const char *path) {
    return !access(path, F_OK);
}

typedef struct vfs_dir {
    char* path;
    // const
    zip* archive;
    int is_directory;
    struct vfs_dir *next;
} vfs_dir;

// TODO: Thread safety
static vfs_dir *dir_head = NULL;

bool jeff_set_working_dir(const char *path) {
    return chdir(path) == 0;
}

void vfs_mount(const char *path) {
    // TODO: Check if path already mounted
    
    zip *z = NULL;
    int is_directory = ('/' == path[strlen(path)-1]);
    if( !is_directory ) z = zip_open(path, "rb");
    if( !is_directory && !z ) return;

    vfs_dir *prev = dir_head, zero = {0};
    *(dir_head = REALLOC(0, sizeof(vfs_dir))) = zero;
    dir_head->next = prev;
    dir_head->path = STRDUP(path);
    dir_head->archive = z;
    dir_head->is_directory = is_directory;
}

static bool vfs_has(vfs_dir *dir, const char *filename) {
    if (dir->is_directory) {
        char buf[512];
        snprintf(buf, sizeof(buf), "%s%s", dir->path, filename);
        if (file_exists(buf))
            return true;
    } else {
        int index = zip_find(dir->archive, filename);
        if (index != -1)
            return true;
    }
    return false;
}

bool vfs_exists(const char *filename) {
    vfs_dir *dir = dir_head;
    if (!dir)
        return file_exists(filename);
    while (dir) {
        if (vfs_has(dir, filename))
            return true;
        dir = dir->next;
    }
    return false;
}

unsigned char *vfs_read(const char *filename, size_t *size) { // must free() after use
    vfs_dir *dir = dir_head;
    if (!dir)
        return read_file(filename, size);
    while (dir) {
        if (vfs_has(dir, filename)) {
            if (dir->is_directory)
                return read_file(filename, size);
            else {
                int index = zip_find(dir->archive, filename);
                unsigned char *data = zip_extract(dir->archive, index);
                if (size)
                    *size = zip_size(dir->archive, index);
                return data;
            }
        }
        dir = dir->next;
    }
    if (size)
        *size = 0;
    return NULL;
}

bool vfs_write(const char *filename, unsigned char *data, size_t size) {
    // TODO: Write to all mounts or first mount? overwrite existing?
    return false;
}
