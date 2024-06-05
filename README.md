# **Chart of Nuclides GUI**

Maintainer: Jonathan Williams

## Description

An offline viewer of ENSDF data, **extremely early in development and basically non-functional right now**.  The UI is implemented in [SDL](https://github.com/libsdl-org/SDL) (so you can browse nuclear half-lives using some of the same code that powers [Half-Life](https://www.pcgamingwiki.com/wiki/Half-Life#Middleware)).

The program parses plaintext ENSDF data files (available [here](https://www.nndc.bnl.gov/ensarchivals/)) into a binary database, eventually one will be able to query this for information on various nuclei (levels, cascades, gamma-rays).

The goal is to develop a performant and multiplatform tool that will be useful in both professional (nuclear structure research) and educational contexts.

## Features

TBD

## Installing from source

### Build dependencies

* gcc (or clang if the Makefile is edited)
* make
* SDL3
* SDL_image
* SDL_ttf

The current version has been tested under Arch Linux as of May 2024.

### Build instructions

The program relies on additional data files, which can be obtained from **TBD**.  The embedded font used needs to support special UTF-8 characters in order to display info such as decay modes correctly.

Install all build dependencies listed above.  For now SDL3 and its libraries probably have to be compiled manually, as they aren't (yet) packaged for major Linux distros.

First, run `make` in the source root directory. Two executables will be built: `proc_data` (which generates the data package `con.dat` used by the main application), and `con` (the main application). Generate the packaged data file (`con.dat`) by running: 

```
./proc_data
```

This will build the data package file `con.dat`. Then, the main application can be run:

```
./con
```

### Keyboard shortcuts

| Key        | Action |
| :--------- | :----- |
| F11        | Toggle fullscreen mode |
| P          | Toggle performance stats debug overlay |

## Disclaimer

The author(s) make no guarantee of the accuracy or completeness of the information provided.  As stated in the [license](COPYING.md), this program is provided without any warranty.  Please file a bug report if you spot any inaccuracies (where the data reported by the program differs from the ENSDF data).

## Acknowledgements

A slightly modified version of the [SDL_FontCache](https://github.com/grimfang4/SDL_FontCache) library by Jonathan Dearborn is used for text rendering.