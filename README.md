# MyOS
Yet another homebrew OS

"MyOS" is a hobby OS I've started working on. It is very basic and has only ever executed one program. It will (perhaps) become a more feature-rich OS in the distant future.

The name is temporary and will be changed when I feel this project is large enough to warrant a decent name.

![Screenshot with test image](https://raw.githubusercontent.com/coderTrevor/MyOS/master/Media/Screenshots/MyOS_Graphical1.png "MyOS Screenshot")


## Motivation
First-things-first: I have no illusions about the fact that nobody will ever use this as their main operating system, or probably even use it at all. In the history of computing, only one hobby OS has ever achieved that, and today, Linux still faces many challenges despite how many contributors it has gathered.

Mostly I wanted to make an OS for the following reasons:
* To see how hard it is to make one & to say that I've done it. I've really just always wanted to make an OS.
* To get more experience with kernel programming, so I can become a better contributor to the ReactOS project. Currently I've contributed some code to the ROS NTFS driver via two Google Summer of Code's.
* To experiment with debugging methods that will allow for fast OS development on real hardware (again, with the goal of bringing some of these methods to ReactOS).
* Perhaps, possibly, to investigate SMP and 64-bit kernels. I haven't decided if I'm going in that direction or not yet.

## Goals
I'll be happy when it can run its own port of Doom. In fact, I've been toying around with the idea of making the shell Doom itself. I'd call it "Shotgun Shell." That might be novel enough to get it some attention.

## Features, Limitations, & Quirks
There's not much here beyond what's provided with tutorials like those from the OSDev wiki and Brokenthorn. I haven't even finished all of those tutorials. The following is implemented:
* It will launch an .exe, provided the .exe is built to target MyOS (native app, based at 0x8000).
* Paging is enabled
* A GDT is used
* The kernel is mapped to the upper-half of memory (and can be remapped easily)
* A do-little command line shell is provided
* Minimal support for COM1 serial input and output (Currently commented-out)
* Basic interrupt support, mostly just used by the keyboard and NIC drivers
* A very minimal RTL 8139 NIC driver (only tested with QEMU)
* Some network protocols are minimally implemented, like ARP, IP, UDP, and a DHCP client
* Filesystem support is only offered through a TFTP client with a hardcoded server IP
* Can be built with MSVC 2015 (which makes it somewhat rare in the hobby OS world)
* The OS builds as a .dll file. This file is multiboot-compliant and can be loaded by Qemu with the -kernel option, or you can boot it with GRUB.
* It has graphical support (if available, provided either by Grub or a Bochs Graphics Adapter) and can display a bitmap.
* It has some very basic sound support and will play .voc files.
* It can run a collection of shell commands from a batch file.

## Other Notes
The code style is very messy and inconsistent. Partly this is because I've copy-pasted code from different tutorials when I started this project. Also, I still haven't decided on what style I want to use for the project as a whole. When I do, I'll fix the inconsistencies.

Porting the code to be buildable from a gcc cross-compiler in Linux is probably a task I'll tackle soon. I want to make the code portable before the project gets too big.

Targetting x64 doesn't work, make sure you build with x86. I haven't removed x64 as a target because I might want to use it later.

If you have come here to learn something, you should know that I'm as amateur as it gets when it comes to OS programming.

## Acknowledgements
In addition to CC0 code copied from the [OSDev wiki](https://wiki.osdev.org/Expanded_Main_Page) and [forums](http://forum.osdev.org/) (thanks!) and some public domain code from [BrokenThorn Entertainment](http://www.brokenthorn.com/Resources/OSDevIndex.html) (also thanks!).
This project utilizes a modified version of [SDL_picofont](http://nurd.se/~noname/?section=sdl_picofont) by Fredrik Hultin with patches from Saul D "KrayZier" Beniquez (thanks also!). The latter is released under GPLv2.
I also use some code from SDL. CRT.c contains much of the code from the SDL file SDL_stdlib.c, by Sam Lantinga. See CRT.c (or SDL_stdlib.c) for licensing info.

## Troubleshooting
If you can't build the source with MSVC, make sure you've selected x86 as the target platform. MSVC selects x64 by default.
