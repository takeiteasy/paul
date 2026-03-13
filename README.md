# paul

**P**latform **A**gnostic **U**tility **L**ibraries

A collection of header-only, zero-dependency, platform-agnostic C libraries.

## Libraries

| Library | Min. C version | Notes |
|:---:|:---:|:---:|
| `paul_ecs.h` | C99 | Barebones pure entity-component-system (uses sparse arrays) |
| `paul_list.h`³ | ?? | Dynamic array (stretch-buffer) implementation |
| `paul_os.h` | C11 | Path and file library for Windows, Posix, and MacOS |
| `paul_format.h` | C11 | Common Lisp `format` port |
| `paul_random.h`¹ | C99 | Pseudo-random number generation and noise functions |
| `paul_shell.h`² | C99 | Embeddable bourne-like shell |
| `paul_string.h` | C11 | Unified ascii+wide string type |
| `paul_table.h`⁴ | C11† | Lua-like hash table implementation |
| `paul_threads.h` | C99 | Thread pool + job queue for Windows and pthread |

### Legend

- **†**: Library is C only and will not compile for C++
- 1: Uses prng generation from [prng](https://github.com/rdadolf/prng/tree/master) by rdadolf (Bob Adolf)
- 2: Based on [psh](https://github.com/proh14/psh) by proh14 (Hoorad Farrokh)
- 3: Based on [stretchy buffer](https://github.com/nothings/stb/blob/master/deprecated/stretchy_buffer.h) by nothings (Sean Barrett)
- 4: Based on [imap](https://github.com/billziss-gh/imap) by billziss-gh (Bill Zissimopoulos)

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
