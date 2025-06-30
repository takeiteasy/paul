/* paul/vfs.c -- https://github.com/takeiteasy/paul

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

#include "paul.h"
#ifndef PAUL_NO_VFS
#undef SWAP
#define ZIP_C
#include "zip.h"
#include "garry.h"
#include "table.h"
#endif

#ifndef PAUL_NO_VFS
typedef struct vfs_dir {
    const char *path;
    zip *archive;
} vfs_dir;

// TODO: Thread safety
static struct {
    table_t vfs;
} state;

void init_vfs(void) {
    state.vfs = table_new();
}
#endif

bool paul_vfs_mount(const char *src, const char *dst) {
#ifndef PAUL_NO_VFS
    // TODO: test + sanitize dst
    if (table_has(&state.vfs, dst)) {
        fprintf(stderr, "[MOUNT ERROR] Entry already mounted at \"%s\"\n", dst);
        return false;
    }
    const char *ext = hal_file_extension(src);
    vfs_dir *d = NULL;
    if (ext && !strncmp(".zip", ext, 4)) {
        if (hal_file_exists(src)) {
            d = malloc(sizeof(vfs_dir));
            d->path = NULL;
            if (!(d->archive = zip_open(src, "rb"))) {
                free(d);
                fprintf(stderr, "[MOUNT ERROR] Failed to read .zip archive at \"%s\"\n", src);
                return false;
            }
            table_set(&state.vfs, dst, (void*)d);
        } else {
            fprintf(stderr, "[MOUNT ERROR] No .zip archive at \"%s\"\n", src);
            return false;
        }
    } else {
        if (hal_dir_exists(src)) {
            d = malloc(sizeof(vfs_dir));
            d->path = strdup(src);
            d->archive = NULL;
            table_set(&state.vfs, dst, (void*)d);
        } else {
            fprintf(stderr, "[MOUNT ERROR] No directory at \"%s\"\n", src);
            return false;
        }
    }
    return true;
#else
    return false;
#endif
}

bool paul_vfs_unmount(const char *name) {
    if (table_has(&state.vfs, name))
        return false;
    vfs_dir *dir = NULL;
    table_get(&state.vfs, name, (void**)&dir);
    if (dir->archive)
        zip_close(dir->archive);
    else
        free((void*)dir->path);
    free(dir);
    table_del(&state.vfs, name);
    return true;
}

int _delete(table_pair_t *pair, void *userdata) {
    vfs_dir *dir = (vfs_dir*)pair->value;
    if (dir->archive)
        zip_close(dir->archive);
    else
        free((void*)dir->path);
    free(dir);
    return 1;
}

void paul_vfs_unmount_all(void) {
    table_each(&state.vfs, _delete, NULL);
    table_free(&state.vfs);
    state.vfs = table_new();
}

unsigned char *paul_vfs_read(const char *filename, size_t *size) { // must free() after use
#ifndef PAUL_NO_VFS
    return NULL;
#else
    return hal_read_file(filename, size);
#endif
}

bool paul_vfs_write(const char *filename, void *data, size_t size, bool overwrite_existing) {
#ifndef PAUL_NO_VFS
    return NULL; // TODO
#else
    return hal_write_file(filename, (const char*)data, size, overwrite_existing);
#endif
}

void paul_vfs_glob(const char *glob, paul_glob_callback_t callback) {
    // TODO
}
