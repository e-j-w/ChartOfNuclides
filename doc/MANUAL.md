# User manual

## Basic controls

The main chart view can be panned by clicking and dragging with a mouse/touchpad, and zoomed in/out using the mouse wheel or scrolling on the touchpad. The controls are basically identical to something like Google Maps.

Right clicking on selected text or on the chart of nuclides will open a menu allowing you to copy information to the clipboard.

### Keyboard/gamepad controls

The program is also designed to be fully navigable with a keyboard and/or gamepad.

| Key(s)             | Gamepad button(s)           | Action |
| :----------------- | :-------------------------- |:----- |
| Arrow keys         | Analog thumbsticks          | Pan chart view, navigate menus, scroll list views |
| W / A / S / D      | D-pad                       | Change selected nuclide (in chart and level list views) |
| + / -              | Left/right triggers         | Zoom in/out on chart |
| ] / [              | Left/right shoulder buttons | Cycle between view modes for the chart (half-life, 2+ energy, etc.), or between reaction datasets in the detail view |
| Enter              | A / Circle (right button)   | Select menu items |
| Alt                | X / Triangle (top button)   | Open menu |
| Escape / backspace | B / Cross (bottom button)   | Exit out of open menus, cancel selection, etc. |
| Ctrl+C             | N/A                         | Copy selected text to clipboard |
| Ctrl+F             | N/A                         | Search |
| F11                | Home button                 | Toggle fullscreen mode |
| P                  | N/A                         | Toggle debug overlay (shows FPS and other stats) |
| Ctrl+Q             | N/A                         | Quit application |

## Searching

The search interface is opened by clicking on the search button (magnifying glass icon) in the upper-right corner of the main chart view, or using the keyboard shortcut `Ctrl+F`. You can then type a search query using the keyboard, and search results will be shown below in real time. Click on a search result (or use the arrow keys to navigate to it and then select it with `Enter`) to navigate to the corresponding region of the chart or show the relevant level/gamma information.

Example search queries include:

| Search type        | Description                                         | Example search(es)           |
| :----------------- | :-------------------------------------------------- |:---------------------------- |
| Nuclide / isotope  | Search for a nuclide / isotope on the main chart    | `32Si`, `cobalt-60` (shows 60Co), `eu152` (shows 152Eu) |
| Level energy       | Energy (in keV) of an excited state in a nuclide. Can be combined with the nuclide name.  | `7654.07` (shows Holye state of 12C), `32Si 5502` (shows isomer of 32Si) |
| Gamma energy       | Energy (in keV) of a gamma-ray transition in a nuclide. Can be combined with the nuclide name.  | `2548 28Mg` (shows 4+ to 2+ transition in 28Mg) |
| Gamma-ray cascade  | Energies (in keV) of a sequence of gamma-ray transitions in a nuclide. Can be combined with the nuclide name.  | `263 685 1477` (shows isomeric cascades in 93Mo), `1274 2083` (shows 4+ to 2+ to 0+ cascade in 22Ne) |