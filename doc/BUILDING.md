- [Linux](#linux)
- [Windows](#windows)

# Linux

The current code has been tested under Arch Linux and Debian 12 as of August 2025, though most recent Linux distros should work as well.

## Using Flatpak

This will build a sandboxed [Flatpak](https://flatpak.org/) package, which can then be installed on any Linux machine which already has `flatpak` (this is the way the [official releases](https://github.com/e-j-w/ChartOfNuclides/releases) are built). This is the recommended method for most users since the Flatpak builder should resolve all dependencies and download all neccessary data files, however the automatic build process can take some time. If you want to avoid that, you can do a [manual build](#manual-build) instead.

### Get Flatpak build dependencies

On Arch Linux:

```
sudo pacman -Syu flatpak git
```

On Debian/Ubuntu:

```
sudo apt install flatpak git
```

### Build and install

Setup `flatpak` with the Flathub repo and get the build metadata (from [this](https://github.com/e-j-w/ChartOfNuclides-flatpak) repo):

```
flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo
sudo flatpak install -y flathub org.flatpak.Builder
git clone https://github.com/e-j-w/ChartOfNuclides-flatpak
cd ChartOfNuclides-flatpak
```

Build the application (if this command fails with a free disk space error, run `ostree --repo=repo config set core.min-free-space-percent 0` and then try again):

```
flatpak run org.flatpak.Builder --force-clean --sandbox --user --install-deps-from=flathub --disable-rofiles-fuse --ccache --repo=repo flatpak_build io.github.e_j_w.ChartOfNuclides-master.yml
```

Then package the application into a [single file bundle](https://docs.flatpak.org/en/latest/single-file-bundles.html):

```
flatpak build-bundle repo ChartOfNuclides.flatpak io.github.e_j_w.ChartOfNuclides
```

The `ChartOfNuclides.flatpak` bundle produced here can be used to install the application on any Linux machine with `flatpak`.  To install, run:

```
flatpak install ChartOfNuclides.flatpak
```

Clean up the build to save disk space (optional):

```
rm -rf flatpak_build .flatpak-builder repo
```

The "Chart of Nuclides" application should now be available in your application menu and/or via the application search interface (depending on your desktop environment). Or you can run the application directly from the terminal:

```
flatpak run io.github.e_j_w.ChartOfNuclides
```

To uninstall the application, run:

```
flatpak uninstall io.github.e_j_w.ChartOfNuclides
```

Have fun!

## Manual build

Instructions to produce a 'native' build that doesn't use Flatpak.

### Build dependencies

* C compiler: gcc (or clang)
* GNU make
* [SDL3](https://github.com/libsdl-org/SDL/releases), [SDL3_image](https://github.com/libsdl-org/SDL_image/releases), [SDL3_ttf](https://github.com/libsdl-org/SDL_ttf/releases)
  * SDL3 should be built with `libdecor` on Linux when using GNOME Wayland, otherwise window decorations won't be present.

For now you may have to manually compile SDL3 and its libraries, as they aren't yet packaged for many major Linux distros. Specific releases of the SDL3 libraries that this code has been tested against are listed in the [Flatpak manifest](https://github.com/e-j-w/ChartOfNuclides-flatpak/blob/master/io.github.e_j_w.ChartOfNuclides-master.yml).

### Build the application

Install all build dependencies listed above, then build the application binaries using `make` (from the source tree root directory):

```
# using gcc:
make all -j
# alternatively, using clang:
make all -j CC=clang
```

Two executables will be built: `proc_data` (which generates the data package containing the nuclear structure database used by the main application), and `chart` (the main application).  [Build the data files](#build-data-file) if necessary, then run the application:

```
./chart
```

To install the application for all users, on a Linux system:

```
sudo make install-linux
```

To uninstall:

```
sudo make uninstall-linux
```

### Build data file

In order for the application to run, it requires a data file (`chart.dat`) containing the nuclear structure database and graphics/font resources.  The application will look for the data file in any of the following locations (in order of preference):

| Location Name            | Path |
| :----------------------- | :----- |
| User-specific data path  | `~/.local/share/chart/` (Linux) |
| Executable directory     | The same directory that the executable was run from.  |
| System-wide data path    | `/usr/share/chart/` (Linux) |


If you don't have the `chart.dat` data file, it can be built from the original data files using the `proc_data` program (built during the [previous step](#build-the-application)).  First you must obtain the required data files:

#### Acquire data files

<a id="data_sources"></a>
| Data       | Source location | Instructions |
| :--------- | :---------------| :----------- |
| Nuclear structure data    |  [ENSDF](https://www.nndc.bnl.gov/ensarchivals/) | Download the zip archive from the link in the 'Latest Dataset' section. Unzip it in the source tree into the directory `data/ensdf/` (such that the `ensdf` subdirectory contains the files `ensdf.001`, `ensdf.002`, etc.). |
| Mass data                 | [AME2020](https://amdc.impcas.ac.cn/web/masseval.html) | This data is already provided in this repo (at `data/masses.txt`).  If you want to retreive/update the data yourself, go to the source website, then download the first ASCII file (`mass_1.mas20`).  Save/rename this file at `data/masses.txt`. |
| Isotope abundance data    | [NIST](https://www.nist.gov/pml/atomic-weights-and-isotopic-compositions-relative-atomic-masses) | This data is already provided in this repo (at `data/abundances.txt`).  If you want to retreive/update the data yourself, go to the source website, then under 'Search the Database', select 'All Elements', output type 'Linearized ASCII Output', with the option 'Most common isotopes'.  Select 'Get Data'.  Copy the resulting plaintext data into a text file, save the text file in the source tree under `data/abundances.txt`. |

Once the data files are properly set up, run: 

```
./proc_data
```

This will build the data package file `chart.dat` in the same directory (you can then get rid of the original data files if you don't want them anymore).

# Windows

There are various guides online for building SDL3 programs in Windows.  In a Windows 10 VM, the following has worked for me:

* Install mingw-w64 and SDL3 via [MSYS2](https://www.msys2.org/).  Follow the [installation instructions](https://www.msys2.org/#installation), and then run mingw-w64 (`MSYS2 MinGW UCRT x64` in the Start Menu). Install mingw-w64 and the SDL3 libraries from the MSYS2 terminal:

```
pacman -S --needed base-devel pkgconf mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-sdl3 mingw-w64-ucrt-x86_64-sdl3-ttf mingw-w64-ucrt-x86_64-sdl3-image
```

Go to the source directory, and compile:

```
cd /c/path/to/ChartOfNuclides
make all -j
```

If necessary, build the data file (using the same [procedure](#build-data-file) as on Linux).

Check which DLLs are needed to run the application by running `ldd chart.exe`. Copy all of the listed DLLs from the MSYS2 distribution to the same directory as `chart.exe`. Then, it should be possible to run `chart.exe` from the file manager.
