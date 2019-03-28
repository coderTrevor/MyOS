# MyOS
Yet another homebrew OS

"MyOS" is a hobby OS I've started working on. It is very basic and has only ever executed one program. It will (perhaps) become a more feature-rich OS in the distant future.

The name is temporary and will be changed when I feel this project is large enough to warrant a decent name.

I'm mostly just putting this on github so I can put it out of my mind for a while. It's taken me about a month to get it this far and I haven't had time for much else (like school) while I've been obsessing over it.

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
* The paging code triple-faults if optimizations are enabled. I have no idea why that is yet.
* The OS builds as a .dll file. This file is multiboot-compliant and can be loaded by Qemu with the -kernel option, or you can boot it with GRUB.

## Other Notes
The code style is very messy and inconsistent. Partly this is because I've copy-pasted code from different tutorials when I started this project. Also, I still haven't decided on what style I want to use for the project as a whole. When I do, I'll fix the inconsistencies.

Porting the code to be buildable from a gcc cross-compiler in Linux is probably the next task I'll tackle. I want to make the code portable before the project gets much bigger.

Targetting x64 doesn't work, make sure you build with x86.

If you have come here to learn something, you should know that I'm as amateur as it gets when it comes to OS programming.
