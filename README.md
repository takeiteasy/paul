# paul

> [!WARNING]
> Work in progress

**P**latform **A**gnostic **U**tility **L**ibraries

A collection of small, header-only, platform-agnostic C libraries with no dependencies.

## Libraries

| Library | C version | Notes |
|:---|:---:|:---|
| `paul_autotile.h`* | C89 | Simple to implement autotiling |
| `paul_bitmap.h` | C89 | Bitmap manipulation and color utilities |
| `paul_ecs.h`⁑ | C89 | Barebones entity-component-system (uses sparse arrays) |
| `paul_list.h` | C89 | Dynamic array (stretch-buffer) implementation |
| `paul_math.h`§ | ? | Linear algebra, vector + matrix math |
| `paul_memory.h`⁑* | C99 | Cross-platform arena allocator implementation |
| `paul_mixer.h`* | C89 | Generic audio mixing (no audio playback, just mixing) |
| `paul_os.h` | C89 | OS abstractions and path manipulation |
| `paul_random.h` | C89 | Pseudo-random number generation and noise functions |
| `paul_string.h` | C89 | Unified ascii+wide string type |
| `paul_table.h`⁑*† | C11 | Lua-like table implementation |
| `paul_threads.h`⁑ | C89 | Polyfill for `pthreads` and C11 `threads.h` + thread pool/queue |
| `paul_vfs.h`* | C89 | Virtual filesystem library (for archives + filesystem) |

### Legend

- **\***: Library is incomplete or a stub
- **⁑**: Library supports clang/gcc blocks (`^(...){...}`) (optionally)
- **†**: Library doesn't support C++ (at all, won't build)
- **§**: Library depends on clang/gcc extensions
- **?**: I don't know (yet?)

## LICENSE

```
paul Copyright (C) 2025 George Watson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```
