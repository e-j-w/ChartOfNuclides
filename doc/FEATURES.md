# Feature list

The overall goal of this project is to make a simple, performant, and multiplatform tool that will be useful in both professional (nuclear structure research) and educational contexts. Currently implemented features include:

- Display the chart of nuclides with various color schemes: half-life / lifetime, decay mode, E(2+) ...
- Select individual nuclides to browse ground and excited state data, including:
  - Level energies, half-lives/lifetimes, decay modes, spin-parities ...
  - Gamma energies, branching fractions, multipolarities ...
  - Atomic masses.
- Fast, multithreaded search interface for nuclide, level, gamma, and gamma cascade data.
- Runs locally, with no network connection needed.
- Mouse, keyboard, and gamepad (!) support.
- Written in portable [C99](https://en.wikipedia.org/wiki/C99) using [SDL](https://github.com/libsdl-org/SDL) (so you can browse nuclear half-lives using some of the same code that powers [Half-Life](https://www.pcgamingwiki.com/wiki/Half-Life#Middleware)). Fast hardware accelerated rendering using `SDL_Renderer`, native HI-DPI scaling support.