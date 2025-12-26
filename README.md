# paul

**P**latform **A**gnostic **U**tility **L**ibraries

A collection of header-only, zero-dependency, platform-agnostic C libraries.

## Libraries

| Library | Min. C version | Notes |
|:---:|:---:|:---:|
| `paul_bitmap.h` | ?? | Bitmap manipulation utilities (optionally supports `paul_color`) |
| `paul_bignum.h` | ?? | Big number library. Big int, float (real), and complex |
| `paul_color.h` | ?? | Color manipulation utilities |
| `paul_ecs.h` | ?? | Barebones pure entity-component-system (uses sparse arrays) |
| `paul_list.h`¹ | ?? | Dynamic array (stretch-buffer) implementation |
| `paul_math.h`³ | C11* | Linear algebra, vector + matrix math |
| `paul_random.h`² | ?? | Pseudo-random number generation and noise functions |
| `paul_string.h` | C11 | Unified ascii+wide string type |
| `paul_table.h`⁴ | C11† | Lua-like hash table implementation |
| `paul_threads.h` | ?? | Thread pool + job queue |

### Legend

- **\***: Library depends on non-standard clang/gcc extensions (use clang or gcc)
- **†**: Library is C only and will not compile for C++
- **??**: I haven't determined the C standard yet (**TODO**: determine C versions lol)

- 1: Based on [stretchy buffer](https://github.com/nothings/stb/blob/master/deprecated/stretchy_buffer.h) by nothings (Sean Barrett)
- 2: Uses prng generation from [prng](https://github.com/rdadolf/prng/tree/master) by rdadolf (Bob Adolf)
- 3: Easing code taken from raylib's [reasing.h](https://github.com/raysan5/raylib/blob/master/examples/others/reasings.h) by raysan5 (Ramon Santamaria)
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
