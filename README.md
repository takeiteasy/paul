# jeff

> [!WARNING]
> Work in progress

## Goals

- [ ] Cross platform
    - [X] Mac, Windows, Linux
    - [ ] Web
    - [ ] iOS
    - [ ] Android
- [X] Easy to use scene system
- [X] Optional config + cli arguments
- [X] PRNG + noise functions
- [X] Optional input handling
    - [X] Key mapping (**WIP**)
    - [ ] Integrate events + input
    - [ ] Gamepads
- [X] Timers + events
- [ ] Audio manager
- [ ] Generate simple meshes (quad, plane, cube, sphere)
- [ ] Basic networking (TCP/UDP/HTTP)
- [ ] Asset loading
    - [X] Images (.png, .jpg, .tga, .bmp, .psd, .hdr, .pic, .pnm, .qoi)
        - [ ] Export image
    - [X] Audio (.wav, .qoa, .ogg, .mp3, .flac)
        - [ ] Export wave
    - [ ] Fonts (truetype, opentype, aat)
    - [ ] Models (.obj, .iqm, .fbx, .dae, .gltf)
        - [ ] Meshes (.obj, .iqm, .fbx, .dae, .gltf)
        - [ ] Materials (.mtl)
        - [ ] Animations (.iqm, .fbx, .dae, .gltf)
        - [ ] Scenes (.fbx, .dae, .gltf)
    - [X] Virtual filesystem (.zip)
    - [ ] Async asset cache

## Platform support

| Platform                       | Android | iOS | Windows | macOS | Linux | Web |
| ------------------------------ |:-------:|:---:|:-------:|:-----:|:-----:|:---:|
| Accelerometer                  |         |     |         |       |       |     |
| Audio recording                |         |     |         |       |       |     |
| Barometer                      |         |     |         |       |       |     |
| Battery                        |         |     |         |       |       |     |
| Bluetooth                      |         |     |         |       |       |     |
| Brightness                     |         |     |         |       |       |     |
| Call                           |         |     |         |       |       |     |
| Camera (taking picture)        |         |     |         |       |       |     |
| Compass                        |         |     |         |       |       |     |
| CPU count                      |         |     |         |       |       |     |
| Devicename                     |         |     |         |       |       |     |
| Email (open mail client)       |         |     |         |       |       |     |
| Flash                          |         |     |         |       |       |     |
| GPS                            |         |     |         |       |       |     |
| Gravity                        |         |     |         |       |       |     |
| Gyroscope                      |         |     |         |       |       |     |
| Humidity                       |         |     |         |       |       |     |
| IR Blaster                     |         |     |         |       |       |     |
| Keystore                       |         |     |         |       |       |     |
| Light                          |         |     |         |       |       |     |
| Maps                           |         |     |         |       |       |     |
| Native file chooser            |         |     |         |       |       |     |
| Notifications                  |         |     |         |       |       |     |
| Orientation                    |         |     |         |       |       |     |
| Proximity                      |         |     |         |       |       |     |
| Screenshot                     |         |     |         |       |       |     |
| SMS (send messages)            |         |     |         |       |       |     |
| Spatial Orientation            |         |     |         |       |       |     |
| Speech to text                 |         |     |         |       |       |     |
| Storage Path                   |         |     |         |       |       |     |
| Temperature                    |         |     |         |       |       |     |
| Text to speech                 |         |     |         |       |       |     |
| Unique ID                      |         |     |         |       |       |     |
| Vibrator                       |         |     |         |       |       |     |
| Voip                           |         |     |         |       |       |     |
| Wifi                           |         |     |         |       |       |     |

### Window support (sokol_app.h)

| Platform            | Windows | macOS | Linux |  iOS  | Android |  HTML5 |
|---------------------|:-------:|:-----:|:-----:|:-----:|:-------:|:------:|
| gl 4.x              | YES     | YES   | YES   | ---   | ---     |  ---   |
| gles3/webgl2        | ---     | ---   | YES(2)| YES   | YES     |  YES   |
| metal               | ---     | YES   | ---   | YES   | ---     |  ---   |
| d3d11               | YES     | ---   | ---   | ---   | ---     |  ---   |
| noapi               | YES     | TODO  | TODO  | ---   | TODO    |  ---   |
| KEY_DOWN            | YES     | YES   | YES   | SOME  | TODO    |  YES   |
| KEY_UP              | YES     | YES   | YES   | SOME  | TODO    |  YES   |
| CHAR                | YES     | YES   | YES   | YES   | TODO    |  YES   |
| MOUSE_DOWN          | YES     | YES   | YES   | ---   | ---     |  YES   |
| MOUSE_UP            | YES     | YES   | YES   | ---   | ---     |  YES   |
| MOUSE_SCROLL        | YES     | YES   | YES   | ---   | ---     |  YES   |
| MOUSE_MOVE          | YES     | YES   | YES   | ---   | ---     |  YES   |
| MOUSE_ENTER         | YES     | YES   | YES   | ---   | ---     |  YES   |
| MOUSE_LEAVE         | YES     | YES   | YES   | ---   | ---     |  YES   |
| TOUCHES_BEGAN       | ---     | ---   | ---   | YES   | YES     |  YES   |
| TOUCHES_MOVED       | ---     | ---   | ---   | YES   | YES     |  YES   |
| TOUCHES_ENDED       | ---     | ---   | ---   | YES   | YES     |  YES   |
| TOUCHES_CANCELLED   | ---     | ---   | ---   | YES   | YES     |  YES   |
| RESIZED             | YES     | YES   | YES   | YES   | YES     |  YES   |
| ICONIFIED           | YES     | YES   | YES   | ---   | ---     |  ---   |
| RESTORED            | YES     | YES   | YES   | ---   | ---     |  ---   |
| FOCUSED             | YES     | YES   | YES   | ---   | ---     |  YES   |
| UNFOCUSED           | YES     | YES   | YES   | ---   | ---     |  YES   |
| SUSPENDED           | ---     | ---   | ---   | YES   | YES     |  TODO  |
| RESUMED             | ---     | ---   | ---   | YES   | YES     |  TODO  |
| QUIT_REQUESTED      | YES     | YES   | YES   | ---   | ---     |  YES   |
| IME                 | TODO    | TODO? | TODO  | ???   | TODO    |  ???   |
| key repeat flag     | YES     | YES   | YES   | ---   | ---     |  YES   |
| windowed            | YES     | YES   | YES   | ---   | ---     |  YES   |
| fullscreen          | YES     | YES   | YES   | YES   | YES     |  ---   |
| mouse hide          | YES     | YES   | YES   | ---   | ---     |  YES   |
| mouse lock          | YES     | YES   | YES   | ---   | ---     |  YES   |
| set cursor type     | YES     | YES   | YES   | ---   | ---     |  YES   |
| screen keyboard     | ---     | ---   | ---   | YES   | TODO    |  YES   |
| swap interval       | YES     | YES   | YES   | YES   | TODO    |  YES   |
| high-dpi            | YES     | YES   | TODO  | YES   | YES     |  YES   |
| clipboard           | YES     | YES   | YES   | ---   | ---     |  YES   |
| MSAA                | YES     | YES   | YES   | YES   | YES     |  YES   |
| drag'n'drop         | YES     | YES   | YES   | ---   | ---     |  YES   |
| window icon         | YES     | YES(1)| YES   | ---   | ---     |  YES   |

```
(1) macOS has no regular window icons, instead the dock icon is changed
(2) supported with EGL only (not GLX)
```
    
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
