<div align = center><img src="https://raw.githubusercontent.com/e-j-w/ChartOfNuclides/master/data/icon.svg" width="150" height="150" alt="icon">

<b>Chart of Nuclides GUI</b>

</div>

## Description

**NOTE: This is early in development and has limited functionality right now.**

An offline viewer of isotope and nuclear structure data, presented in the familiar [Chart of Nuclides / Segrè chart](https://en.wikipedia.org/wiki/Table_of_nuclides) format.  The UI is implemented in [SDL](https://github.com/libsdl-org/SDL) (so you can browse nuclear half-lives using some of the same code that powers [Half-Life](https://www.pcgamingwiki.com/wiki/Half-Life#Middleware)).  The program parses various plaintext nuclear structure data (listed [here](#collect-data)) into a binary database, eventually one will be able to query this for information on various nuclei (levels, cascades, gamma-rays).

The goal is to develop a performant and multiplatform tool that will be useful in both professional (nuclear structure research) and educational contexts.

## Features

- Select individual nuclides to browse ground and excited state data, including:
  - Level energies, half-lives, decay modes...
  - Gamma energies, branching fractions, multipolarities...
- Runs locally, with no network connection needed.
- Fast hardware accelerated rendering using `SDL_Renderer`, HI-DPI scaling support. 
- More to come...

## Building and installing from source

### Build dependencies

* C compiler: gcc (or clang)
* GNU make
* SDL3, SDL_image, SDL_ttf

The current version has been tested under Arch Linux as of July 2024.  For now you'll probably have to manually compile SDL3 and its libraries, as they aren't (yet) packaged for major Linux distros.

### Build the application

Install all build dependencies listed above, then build the application binaries using `make` (from the source tree root directory):

```
# using gcc:
make all -j
# alternatively, using clang:
make all -j CC=clang
```

Two executables will be built: `proc_data` (which generates the data package containing the nuclear structure database used by the main application), and `con` (the main application).  [Build the data files](#build-data-files) if neccessary, then run the application:

```
./con
```

Have fun!


### Build data files

In order for the application to run, it requires a data file (`con.dat`) containing the nuclear structure database and graphics/font resources.  The application will look for the data file in any of the following locations (in order of preference):

| Location Name            | Path |
| :----------------------- | :----- |
| User-specific data path  | `~/.local/share/con/` (Linux) |
| Executable directory     | The same directory that the executable was run from.  |
| System-wide data path    | `/usr/share/con/` (Linux) |


If you don't have the `con.dat` data file, it can be built from the original data files using the `proc_data` program (built during the [previous step](#build-the-application)).  First you must obtain the required data files:

| Data       | Source location | Instructions |
| :--------- | :---------------| :----------- |
| Nuclear structure data    |  [ENSDF](https://www.nndc.bnl.gov/ensarchivals/) | Download the zip archive from the link in the 'Latest Dataset' section. Unzip it in the source tree into the directory `data/ensdf/` (such that the `ensdf` subdirectory contains the files `ensdf.001`, `ensdf.002`, etc.). |
| Isotope abundance data    | [NIST](https://www.nist.gov/pml/atomic-weights-and-isotopic-compositions-relative-atomic-masses) | Under 'Search the Database', select 'All Elements', output type 'Linearized ASCII Output', with the option 'Most common isotopes'.  Select 'Get Data'.  Copy the resulting plaintext data into a text file, save the text file in the source tree under `data/abundances.txt`. |

Once the data files are properly set up, run: 

```
./proc_data
```

This will build the data package file `con.dat` in the same directory (you can then get rid of the original data files if you don't want them anymore).

## Using the program

Zoom and pan with the mouse, the controls are basically identical to something like Google Maps.

### Keyboard shortcuts

| Key                | Action |
| :----------------- | :----- |
| Arrow keys / WASD  | Pan chart view |
| +/-                | Zoom in/out on chart |
| F                  | Show full list of levels/gammas for selected nuclide |
| Escape / backspace | Exit out of open menus, cancel selection, etc. |
| F11                | Toggle fullscreen mode |
| P                  | Toggle performance stats debug overlay |

## Disclaimer

The author(s) make no guarantee of the accuracy or completeness of the information provided.  As stated in the [license](COPYING.md), this program is provided without any warranty.  Please file a bug report if you spot any inaccuracies (eg. the data reported by the program differs from the source ENSDF/NIST data).  Some inaccuracies are expected since the program remains in active development.

## Credits

Developed and maintained by [Jonathan Williams](https://e-j-w.github.io/).

A slightly modified version of the [SDL_FontCache](https://github.com/grimfang4/SDL_FontCache) library by Jonathan Dearborn is used for fast text rendering.

The font used in this program (`data/font.ttf`) is a modified version of [Oxygen](https://github.com/KDE/oxygen-fonts) with some additional unicode glyphs from [Noto Sans](https://fonts.google.com/noto/specimen/Noto+Sans).  Both fonts are made available under the [Open Font License](https://openfontlicense.org/) (as described [here](https://fonts.google.com/specimen/Oxygen/about) and [here](https://fonts.google.com/noto/specimen/Noto+Sans/about)).