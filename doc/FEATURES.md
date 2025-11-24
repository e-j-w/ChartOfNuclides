# Feature list

Currently implemented features include:

- Display the chart of nuclides with various color schemes: half-life / lifetime, decay mode, E(2+) ...
- Select individual nuclides to browse ground and excited state data, including:
  - Level energies, half-lives/lifetimes, decay modes, spin-parities ...
  - Gamma energies, branching fractions, multipolarities, mixing ratios, internal conversion coefficients ...
  - Atomic masses, separation energies, Q-values.
- Search interface for nuclide, level, gamma, and gamma cascade data.
- Runs locally, doesn't connect to the internet at all.
- Mouse, keyboard, and gamepad (!) support.

# Technical info and other musings

The overall goal of this project is to make a simple, performant, and multiplatform tool that will be useful in both professional (nuclear structure research) and educational contexts. The reason I'm making this is because there's basically no way to browse isotope data that's both usable offline and suitable for a normal person to use.

The program is written in portable [C99](https://en.wikipedia.org/wiki/C99) using [SDL](https://github.com/libsdl-org/SDL) (so you can browse nuclear half-lives using some of the same code that powers [Half-Life](https://www.pcgamingwiki.com/wiki/Half-Life#Middleware)). It uses several of SDL's built-in features including fast hardware accelerated rendering and native HI-DPI scaling support.

The nuclear structure data comes from [ENSDF](https://www.nndc.bnl.gov/ensdfarchivals/) and [AME2020](https://amdc.impcas.ac.cn/web/masseval.html). The program uses an [overcomplicated parser](../data_processor/proc_data_parser.c) (>4.5k SLOC and counting) to pack the original plaintext source data into a single-file binary database (~70 MB) at build time. This database is fully loaded into memory when running the program, which allows for extremely fast retrieval/search of data. The search interface is multithreaded, allowing for multiple searches of the database to be performed simultaneously.