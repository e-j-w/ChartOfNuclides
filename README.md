<h1 align="center">Chart of Nuclides</h1>

<div align = center><img src="https://github.com/e-j-w/ChartOfNuclides-flatpak/blob/master/assets/con_screenshot.png?raw=true" width="188" height="124" alt="main interface"><img src="https://github.com/e-j-w/ChartOfNuclides-flatpak/blob/master/assets/con_screenshot2.png?raw=true" width="188" height="124" alt="zoomed in interface"><img src="https://github.com/e-j-w/ChartOfNuclides-flatpak/blob/master/assets/con_screenshot3.png?raw=true" width="188" height="124" alt="level list view"><img src="https://github.com/e-j-w/ChartOfNuclides-flatpak/blob/master/assets/con_screenshot5.png?raw=true" width="188" height="124" alt="alternate color scheme view"></div>

An offline viewer of isotope and nuclear structure data in the [Chart of Nuclides / Segrè chart](https://en.wikipedia.org/wiki/Table_of_nuclides) format. Uses nuclear structure data from [ENSDF](https://www.nndc.bnl.gov/ensdf/about.jsp), masses from [AME2020](https://amdc.impcas.ac.cn/web/masseval.html), and isotopic abundance data from [NIST](https://www.nist.gov/pml/atomic-weights-and-isotopic-compositions-relative-atomic-masses). A full list of features is [here](doc/FEATURES.md).

## Quick installation (Linux)

Get the latest [release](https://github.com/e-j-w/ChartOfNuclides/releases) and follow the installation instructions there. Support for other platforms is planned for the future.

## Using the program

See the [user manual](doc/MANUAL.md) for documentation.

## Building from source

Detailed build instructions are [here](doc/BUILDING.md).

## Disclaimer

The author(s) cannot guarantee the accuracy or completeness of the information provided.  As stated in the [license](COPYING.md), this program is provided without any warranty.  Please file a bug report if you spot any inaccuracies (eg. the data reported by the program differs from the [original source data](doc/BUILDING.md#acquire-data-files)).  Some inaccuracies are likely, since the program remains in active development.

## Credits

Developed and maintained by [Jonathan Williams](https://e-j-w.github.io/).

A modified version of the [SDL_FontCache](https://github.com/grimfang4/SDL_FontCache) library by Jonathan Dearborn is used for fast text rendering.

The font used in this program (`data/font.ttf`) is a modified version of [Oxygen](https://github.com/KDE/oxygen-fonts) with some additional unicode glyphs from [Noto Sans](https://fonts.google.com/noto/specimen/Noto+Sans).  Both fonts are made available under the [Open Font License](https://openfontlicense.org/) (as described [here](https://fonts.google.com/specimen/Oxygen/about) and [here](https://fonts.google.com/noto/specimen/Noto+Sans/about)).