<h1 align="center">Chart of Nuclides</h1>

An offline viewer of isotope and nuclear structure data, presented in the familiar [Chart of Nuclides / Segrè chart](https://en.wikipedia.org/wiki/Table_of_nuclides) format. Uses nuclear structure data from [ENSDF](https://www.nndc.bnl.gov/ensdf/about.jsp) and isotopic abundance data from [NIST](https://www.nist.gov/pml/atomic-weights-and-isotopic-compositions-relative-atomic-masses). The goal is to make a simple, performant, and multiplatform tool that will be useful in both professional (nuclear structure research) and educational contexts.

## Screenshots

<div align = center><img src="https://github.com/e-j-w/ChartOfNuclides-flatpak/blob/5984c26073853c21183738be02e5ae6bc506e74d/assets/con_screenshot.png?raw=true" width="376" height="248" alt="main interface"><img src="https://github.com/e-j-w/ChartOfNuclides-flatpak/blob/5984c26073853c21183738be02e5ae6bc506e74d/assets/con_screenshot2.png?raw=true" width="376" height="248" alt="zoomed in interface"></div>

## Features

- Display nuclear chart with various color schemes: half-life/lifetime, E(2+), E(4+)/E(2+) ...
- Select individual nuclides to browse ground and excited state data, including:
  - Level energies, half-lives/lifetimes, decay modes, spin-parities ...
  - Gamma energies, branching fractions, multipolarities ...
- Runs locally, with no network connection needed.
- Written in [C99](https://en.wikipedia.org/wiki/C99) using [SDL](https://github.com/libsdl-org/SDL) (so you can browse nuclear half-lives using some of the same code that powers [Half-Life](https://www.pcgamingwiki.com/wiki/Half-Life#Middleware)). Fast hardware accelerated rendering using `SDL_Renderer`, HI-DPI scaling support. 
- More to come...

## Building and installing from source

The current version has been tested under Arch Linux and Debian 12 as of September 2024, though most recent Linux distros should work as well. The plan is to eventually support other platforms (Windows especially) once a stable SDL3 release is available.

### Using Flatpak

This will build and install a sandboxed [Flatpak](https://flatpak.org/) package. This is the recommended method for most users, as the Flatpak builder should automatically resolve all dependencies and download all neccessary data files. If you want to avoid installing or using Flatpak, you can do a [manual build](#manual-build) instead.

#### Flatpak build dependencies

On Arch Linux:

```
sudo pacman -Syu flatpak git
```

On Debian/Ubuntu:

```
sudo apt install flatpak git
```

#### Build and install

Setup `flatpak` with the Flathub repo and get the build metadata (from [this](https://github.com/e-j-w/ChartOfNuclides-flatpak) repo):

```
flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo
sudo flatpak install -y flathub org.flatpak.Builder
git clone https://github.com/e-j-w/ChartOfNuclides-flatpak
cd ChartOfNuclides-flatpak
mkdir flatpak_build
```

Build and install the application:

```
flatpak run org.flatpak.Builder --force-clean --sandbox --user --install --install-deps-from=flathub --ccache --repo=repo flatpak_build io.github.e_j_w.ChartOfNuclides.yml
```

Clean up the build to save disk space (optional):

```
rm -rf flatpak_build .flatpak-builder
```

To run the application (depending on your desktop environment, it should also be available in the application menu and/or via the application search interface, same as other Flatpak applications):

```
flatpak run io.github.e_j_w.ChartOfNuclides
```

Have fun! To uninstall the application:

```
flatpak uninstall io.github.e_j_w.ChartOfNuclides
```

### Manual build

For those who don't want to use Flatpak.

#### Build dependencies

* C compiler: gcc (or clang)
* GNU make
* SDL3, SDL3_image, SDL3_ttf
  * SDL3 should be built with `libdecor` on Linux when using GNOME Wayland, otherwise window decorations won't be present.

For now you'll probably have to manually compile SDL3 and its libraries, as they aren't (yet) packaged for major Linux distros.

#### Build the application

Install all build dependencies listed above, then build the application binaries using `make` (from the source tree root directory):

```
# using gcc:
make all -j
# alternatively, using clang:
make all -j CC=clang
```

Two executables will be built: `proc_data` (which generates the data package containing the nuclear structure database used by the main application), and `con` (the main application).  [Build the data files](#build-data-files) if necessary, then run the application:

```
./con
```

To install the application for all users, on a Linux system:

```
sudo make install-linux
```

To uninstall:

```
sudo make uninstall-linux
```

#### Build data files

In order for the application to run, it requires a data file (`con.dat`) containing the nuclear structure database and graphics/font resources.  The application will look for the data file in any of the following locations (in order of preference):

| Location Name            | Path |
| :----------------------- | :----- |
| User-specific data path  | `~/.local/share/con/` (Linux) |
| Executable directory     | The same directory that the executable was run from.  |
| System-wide data path    | `/usr/share/con/` (Linux) |


If you don't have the `con.dat` data file, it can be built from the original data files using the `proc_data` program (built during the [previous step](#build-the-application)).  First you must obtain the required data files:


<a id="data_sources"></a>
| Data       | Source location | Instructions |
| :--------- | :---------------| :----------- |
| Nuclear structure data    |  [ENSDF](https://www.nndc.bnl.gov/ensarchivals/) | Download the zip archive from the link in the 'Latest Dataset' section. Unzip it in the source tree into the directory `data/ensdf/` (such that the `ensdf` subdirectory contains the files `ensdf.001`, `ensdf.002`, etc.). |
| Isotope abundance data    | [NIST](https://www.nist.gov/pml/atomic-weights-and-isotopic-compositions-relative-atomic-masses) | This data is already provided in this repo (at `data/abundances.txt`).  If you want to retreive/update the data yourself, go to the source website, then under 'Search the Database', select 'All Elements', output type 'Linearized ASCII Output', with the option 'Most common isotopes'.  Select 'Get Data'.  Copy the resulting plaintext data into a text file, save the text file in the source tree under `data/abundances.txt`. |

Once the data files are properly set up, run: 

```
./proc_data
```

This will build the data package file `con.dat` in the same directory (you can then get rid of the original data files if you don't want them anymore).

## Using the program

Zoom and pan with the mouse, the controls are basically identical to Google Maps.

### Keyboard shortcuts

| Key                | Action |
| :----------------- | :----- |
| Arrow keys         | Pan chart view |
| W / A / S / D      | Pan selected nuclide (in chart and level list views) |
| + / -              | Zoom in/out on chart |
| ] / [              | Cycle between view modes for the chart (half-life, 2+ energy, etc.) |
| F7 / F8            | Adjust user interface size |
| Enter              | Show full list of levels/gammas for selected nuclide |
| Escape / backspace | Exit out of open menus, cancel selection, etc. |
| F11                | Toggle fullscreen mode |
| P                  | Toggle performance stats debug overlay |
| Ctrl+Q             | Quit application |

## Disclaimer

The author(s) make no guarantee of the accuracy or completeness of the information provided.  As stated in the [license](COPYING.md), this program is provided without any warranty.  Please file a bug report if you spot any inaccuracies (eg. the data reported by the program differs from the source ENSDF/NIST data).  Some inaccuracies are likely, since the program remains in active development.

## Credits

Developed and maintained by [Jonathan Williams](https://e-j-w.github.io/).

A slightly modified version of the [SDL_FontCache](https://github.com/grimfang4/SDL_FontCache) library by Jonathan Dearborn is used for fast text rendering.

The font used in this program (`data/font.ttf`) is a modified version of [Oxygen](https://github.com/KDE/oxygen-fonts) with some additional unicode glyphs from [Noto Sans](https://fonts.google.com/noto/specimen/Noto+Sans).  Both fonts are made available under the [Open Font License](https://openfontlicense.org/) (as described [here](https://fonts.google.com/specimen/Oxygen/about) and [here](https://fonts.google.com/noto/specimen/Noto+Sans/about)).