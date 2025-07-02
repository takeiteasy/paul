# paul

> [!WARNING]
> Work in progress

**P**latform **A**gnostic **U**tility **L**ibrary

## Goals

- [ ] Cross platform (see: [hal](https://github.com/takeiteasy/hal))
    - [X] Mac
    - [ ] Windows
    - [ ] Linux
    - [ ] Web
    - [ ] iOS
    - [ ] Android
- [X] (Optional) Scene framework
- [X] (Optional) Default config file + cli arguments
- [X] PRNG + noise
- [X] (Optional) Input handling (see: [sokol_input](https://github.com/takeiteasy/sokol_input))
    - [X] Key mapping
    - [X] Gamepads
    - [ ] Input recording + playback
- [X] Timers + events
    - [ ] Async
- [ ] Audio mixer + playback (see: [sokol_mixer](https://github.com/takeiteasy/sokol_mixer))
- [ ] Networking
    - [ ] TCP
    - [ ] UDP
    - [ ] HTTP
- [ ] Asset loading
    - [X] Images (.png, .jpg, .tga, .bmp, .psd, .hdr, .pic, .pnm, .qoi) (see: [sokol_image](https://github.com/takeiteasy/sokol_image))
    - [X] Audio (.wav, .qoa, .ogg, .mp3, .flac) (see: [sokol_mixer](https://github.com/takeiteasy/sokol_mixer))
    - [ ] Fonts (truetype, fontstash)
    - [ ] Models (.obj, .iqm)
        - [ ] Materials (.mtl)
    - [ ] Video (.mpg)
    - [X] Virtual filesystem (.zip)
    - [ ] Cache
        - [ ] Per scene
        - [ ] Global
        - [ ] Async
    
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
