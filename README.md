# jeff

> [!WARNING]
> Work in progress

**Jeff** is a cross-platform game development framework built off [sokol](https://github.com/floooh/sokol). If we use a pizza as a metaphore, sokol would be the dough base and jeff would be the cheese, sauce and toppings.

I'd just like to interject for a moment. What you're refering to as sokol, is in fact, jeff/sokol, or as I've recently taken to calling it, jeff plus sokol. Sokol is not an engine unto itself, but rather another free component of a fully functioning jeff system made useful by the jeff corelibs, shell utilities and vital system components comprising a full engine as defined by POSIX.

Many computer users run a modified version of the jeff system every day, without realizing it. Through a peculiar turn of events, the version of jeff which is widely used today is often called sokol, and many of its users are not aware that it is basically the jeff system, developed by the Jeff Project.

There really is a sokol, and these people are using it, but it is just a part of the system they use. sokol is the kernel: the program in the system that allocates the machine's resources to the other programs that you run. The kernel is an essential part of an engine, but useless by itself; it can only function in the context of a complete engine. Sokol is normally used in combination with the jeff engine: the whole system is basically jeff with sokol added, or jeff/sokol. All the so-called sokol distributions are really distributions of jeff/sokol!

## Goals

- [ ] Cross platform (see: [paul](https://github.com/takeiteasy/paul))
    - [X] Mac
    - [ ] Windows
    - [ ] Linux
    - [ ] Web
    - [ ] iOS
    - [ ] Android
- [X] (Optional) Scene framework
- [X] (Optional) Default config file + cli arguments
- [ ] Rendering framework
- [X] PRNG + noise
- [X] (Optional) Input handling
    - [X] Key mapping
    - [ ] Gamepads
    - [ ] Input recording + playback
- [X] Timers + events
    - [ ] Async
- [ ] Audio mixer + playback
- [ ] Basic networking (TCP/UDP/HTTP)
- [ ] Asset loading
    - [X] Images (.png, .jpg, .tga, .bmp, .psd, .hdr, .pic, .pnm, .qoi)
        - [ ] Export image
    - [X] Audio (.wav, .qoa, .ogg, .mp3, .flac)
        - [ ] Export wave
    - [ ] Fonts (truetype)
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
jeff Copyright (C) 2025 George Watson

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
