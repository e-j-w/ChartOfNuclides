# Building from source

The current code has been tested under Arch Linux and Debian 12 as of September 2024, though most recent Linux distros should work as well. The plan is to eventually support other platforms (Windows especially) once a stable SDL3 release is available.

## Using Flatpak

This will build and install a sandboxed [Flatpak](https://flatpak.org/) package. This is the recommended method for most users, as the Flatpak builder should automatically resolve all dependencies and download all neccessary data files. If you want to avoid installing or using Flatpak, you can do a [manual build](#manual-build) instead.

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

Build and install the application (if this command fails with a free disk space error, run `ostree --repo=repo config set core.min-free-space-percent 0` and then try again):

```
flatpak run org.flatpak.Builder --force-clean --sandbox --user --install --install-deps-from=flathub --disable-rofiles-fuse --ccache --repo=repo flatpak_build io.github.e_j_w.ChartOfNuclides-master.yml
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

### Building a single file bundle

After building the application, a [single file bundle](https://docs.flatpak.org/en/latest/single-file-bundles.html) can be built using:

```
flatpak build-bundle repo ChartOfNuclides.flatpak io.github.e_j_w.ChartOfNuclides
```

This is useful for distributing the application in a semi-portable manner (note that the runtime dependencies will still need to be installed, so this is not suitable for offline installations).

## Manual build

For those who don't want to use Flatpak.

### Build dependencies

* C compiler: gcc (or clang)
* GNU make
* SDL3, SDL3_image, SDL3_ttf
  * SDL3 should be built with `libdecor` on Linux when using GNOME Wayland, otherwise window decorations won't be present.

For now you'll probably have to manually compile SDL3 and its libraries, as they aren't (yet) packaged for major Linux distros. Specific commits of the SDL3 pre-release libraries that this code has been tested against are listed in the [Flatpak manifest](https://github.com/e-j-w/ChartOfNuclides-flatpak/blob/master/io.github.e_j_w.ChartOfNuclides.yml).

### Build the application

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

### Build data files

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