# paul

**P**latform **A**gnostic **U**tility **L**ibraries

A collection of header-only, platform-agnostic C libraries with no dependencies. Targets: Windows, macOS, and linux.

## Libraries

| Library | C version | Notes |
|:---|:---:|:---|
| `paul_bitmap.h` | ?? | Bitmap manipulation utilities (optionally supports `paul_color`) |
| `paul_color.h` | ?? | Color manipulation utilities |
| `paul_ecs.h` | ?? | Barebones pure entity-component-system (uses sparse arrays) |
| `paul_list.h`¹ | ?? | Dynamic array (stretch-buffer) implementation |
| `paul_math.h`* | ?? | Linear algebra, vector + matrix math |
| `paul_os.h` | ?? | OS abstractions and path manipulation |
| `paul_random.h`² | ?? | Pseudo-random number generation and noise functions |
| `paul_string.h` | ?? | Unified ascii+wide string type |
| `paul_threads.h`§ | ?? | Thread pool + job queue |

### Legend

- **\***: Library depends on clang/gcc extensions
- **§**: Library depends on C11 threads or pthreads/Win32 threads
- **??**: I haven't determined the C standard yet

- 1: Based on [stretchy buffer](https://github.com/nothings/stb/blob/master/deprecated/stretchy_buffer.h) by nothings (Sean Barrett)
- 2: Uses prng generation from [prng](https://github.com/rdadolf/prng/tree/master) by rdadolf (Bob Adolf)

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
