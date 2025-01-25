<h1 align="center">Chart of Nuclides</h1>

<div align = center><img src="https://github.com/e-j-w/ChartOfNuclides-flatpak/blob/e439f2caf5547e928186a42330e6cc07dea57bc5/assets/con_screenshot.png?raw=true" width="188" height="124" alt="main interface"><img src="https://github.com/e-j-w/ChartOfNuclides-flatpak/blob/e439f2caf5547e928186a42330e6cc07dea57bc5/assets/con_screenshot2.png?raw=true" width="188" height="124" alt="zoomed in interface"><img src="https://github.com/e-j-w/ChartOfNuclides-flatpak/blob/e439f2caf5547e928186a42330e6cc07dea57bc5/assets/con_screenshot3.png?raw=true" width="188" height="124" alt="level list view"><img src="https://github.com/e-j-w/ChartOfNuclides-flatpak/blob/e439f2caf5547e928186a42330e6cc07dea57bc5/assets/con_screenshot4.png?raw=true" width="188" height="124" alt="alternate color scheme view"></div>

An offline viewer of isotope and nuclear structure data in the [Chart of Nuclides / Segrè chart](https://en.wikipedia.org/wiki/Table_of_nuclides) format. Uses nuclear structure data from [ENSDF](https://www.nndc.bnl.gov/ensdf/about.jsp) and isotopic abundance data from [NIST](https://www.nist.gov/pml/atomic-weights-and-isotopic-compositions-relative-atomic-masses). A full list of features is [here](doc/FEATURES.md).

## Quick installation

Get the latest [release](https://github.com/e-j-w/ChartOfNuclides/releases) and follow the installation instructions there. Only Linux is supported for now.

## Building from source

Detailed instructions for building from source are [here](doc/BUILDING.md).

## Using the program

Zoom and pan with the mouse, the controls are basically identical to Google Maps.

### Keyboard shortcuts

This program is also designed to be fully navigable with a keyboard and/or gamepad.

| Key                | Gamepad button              | Action |
| :----------------- | :-------------------------- |:----- |
| Arrow keys         | Analog thumbsticks          | Pan chart view, navigate menus |
| W / A / S / D      | D-pad                       | Pan selected nuclide (in chart and level list views) |
| + / -              | Left/right triggers         | Zoom in/out on chart |
| ] / [              | Left/right shoulder buttons | Cycle between view modes for the chart (half-life, 2+ energy, etc.), or between reaction datasets in the detail view |
| Enter              | A / Circle (right button)   | Select menu items |
| Alt                | X / Triangle (top button)   | Open menu |
| Escape / backspace | B / Cross (bottom button)   | Exit out of open menus, cancel selection, etc. |
| Ctrl+F             | N/A                         | Search |
| F11                | Home button                 | Toggle fullscreen mode |
| P                  | N/A                         | Toggle debug overlay |
| Ctrl+Q             | N/A                         | Quit application |


## Disclaimer

The author(s) cannot guarantee the accuracy or completeness of the information provided.  As stated in the [license](COPYING.md), this program is provided without any warranty.  Please file a bug report if you spot any inaccuracies (eg. the data reported by the program differs from the source ENSDF/NIST data).  Some inaccuracies are likely, since the program remains in active development.

## Credits

Developed and maintained by [Jonathan Williams](https://e-j-w.github.io/).

A slightly modified version of the [SDL_FontCache](https://github.com/grimfang4/SDL_FontCache) library by Jonathan Dearborn is used for fast text rendering.

The font used in this program (`data/font.ttf`) is a modified version of [Oxygen](https://github.com/KDE/oxygen-fonts) with some additional unicode glyphs from [Noto Sans](https://fonts.google.com/noto/specimen/Noto+Sans).  Both fonts are made available under the [Open Font License](https://openfontlicense.org/) (as described [here](https://fonts.google.com/specimen/Oxygen/about) and [here](https://fonts.google.com/noto/specimen/Noto+Sans/about)).