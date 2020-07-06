# MyOS
Yet another homebrew OS

"MyOS" is my hobby OS. It's very basic but it can run some programs. It will (perhaps) become a more feature-rich OS in the distant future.

The name is temporary and will be changed when I feel this project is large enough to warrant a decent name.

![Screenshot with test image](https://raw.githubusercontent.com/coderTrevor/MyOS/master/Media/Screenshots/MyOS_Graphical1.png "MyOS Screenshot")

![Doom running on MyOS](https://github.com/coderTrevor/My-Doom/blob/c60abc6bc10bd30e733e29056e232c54e1dc283c/screenshots/Doom%20on%20MyOS.gif?raw=true "Doom!")

![GUI of MyOS](https://raw.githubusercontent.com/coderTrevor/MyOS/master/Media/Screenshots/MyOS_GUI.gif "MyOS GUI")


## Motivation
I have no illusions about the fact that nobody will ever use this as an operating system, but developing it is fun. :)

Mostly I wanted to make an OS for the following reasons:
* To see how hard it is to make one & to say that I've done it. I've really just always wanted to make an OS.
* To get more experience with kernel programming, so I can become a better contributor to the ReactOS project. Currently I've contributed some code to the ROS NTFS driver via two Google Summer of Code's.
* To experiment with debugging methods that will allow for fast OS development on real hardware (again, with the goal of bringing some of these methods to ReactOS).
* Perhaps, possibly, to investigate SMP and 64-bit kernels. I haven't decided if I'm going in that direction or not yet.

## Long-Term Goals
- [x]~~I'll be happy when it can run its own port of Doom.~~ Oh SNAP! It runs Doom now! In fact, I've been toying around with the idea of making the shell Doom itself. I'd call it "Shotgun Shell." That might be novel enough to get it some attention.
- [ ] Should run SDL apps (like Doom and my NES emulator) in their own GUI windows
- [ ] Should work on real hardware (presently only tested with Qemu and VirtualBox)

## Short-Term Goals
- [ ] Add support for reading a CD and produce an ISO which includes all relevant programs, making it very easy for someone else to check out the OS (on a Virtual Machine)
- [ ] Improve the MSVC files / build process and bundle all necessary libs and apps to make it relatively easy for someone else to build the OS with Visual Studio. Presently the project files take the paths of my development machine for granted.
- [ ] Include a debugging stub.
- [ ] Fix debug build

## Features, Limitations, & Quirks
This started out as a culmination of tutorials like those from the OSDev wiki and Brokenthorn, and has grown from there. I haven't even finished all of those tutorials. The following is implemented:
* It will launch an .exe, provided the .exe is built to target MyOS (native app, based at 0x8000).
* Paging is enabled
* A GDT is used
* The kernel is mapped to the upper-half of memory (and can be remapped easily)
* Malloc() and free() are implemented.
* A do-little command line shell is provided
* Minimal support for COM1 serial input and output (Currently commented-out)
* Basic interrupt support, mostly just used by the keyboard and NIC drivers
* A very minimal RTL 8139 NIC driver (only tested with Qemu)
* A virtio-net driver
* An e1000 driver (only working in Qemu)
* Some network protocols are minimally implemented, like ARP, IP, UDP, and a DHCP client
* Filesystem support is only offered through a TFTP client. The TFTP server IP is determined via DHCP.
* Can be built with MSVC 2015 (which makes it somewhat rare in the hobby OS world)
* The OS builds as a .dll file. This file is multiboot-compliant and can be loaded by Qemu with the -kernel option, or you can boot it with GRUB.
* It has graphical support (if available, provided either by Grub or a Bochs Graphics Adapter) and can display a bitmap.
* It has the ability to set a background image.
* It has some very basic SoundBlaster 16 support and will play .voc files.
* It can run a collection of shell commands from a batch file.
* It totally runs Doom! Doooooooooooom!!!!

## Other Notes
The code style is somewhat messy and inconsistent. Partly this is because MyOS started its life as copy-pasted code from different tutorials and the styles of each part have been carried forward. Also, I still haven't decided on what style I want to use for the project as a whole. When I do, I'll fix the inconsistencies.

Porting the code to be buildable from a gcc cross-compiler in Linux is probably a task I'll tackle soon. I want to make the code portable before the project gets too big.

Targetting x64 doesn't work, make sure you build with x86. I haven't removed x64 as a target because I might want to use it later.

If you have come here to learn something, you should know that I'm as amateur as it gets when it comes to OS programming.

## Acknowledgements
I would very much like to thank the following individuals and organizations who've made their source code available! This code has found it's way into MyOS:
* CC0 code copied from the [OSDev wiki](https://wiki.osdev.org/Expanded_Main_Page) and [forums](http://forum.osdev.org/)
* Some public domain code from [BrokenThorn Entertainment](http://www.brokenthorn.com/Resources/OSDevIndex.html)
* This project utilizes a modified version of [SDL_picofont](http://nurd.se/~noname/?section=sdl_picofont) by Fredrik Hultin with patches from Saul D "KrayZier" Beniquez, released under GPLv2.
* I also use some code from [SDL](https://www.libsdl.org/). CRT.c contains much of the code from the SDL file SDL_stdlib.c, by Sam Lantinga. See CRT.c (or SDL_stdlib.c) for licensing info. 
* Marco Paland's [printf](https://github.com/mpaland) implementation, made available via an MIT license.

Thank you all for making this available!

## Troubleshooting
If you can't build the source with MSVC, make sure you've selected x86 release as the target platform. MSVC selects x64 by default. Note that the project is in flux and it's not easy to build right now (see short-term goals).
