# **Chart of Nuclides GUI**

Maintainer: Jonathan Williams

## Description

An offline viewer of ENSDF data, **extremely early in development and basically non-functional right now**.  Based on the command line program [LevelUp](https://github.com/e-j-w/LevelUp) with a UI implemented in [SDL3](https://github.com/libsdl-org/SDL).

The program parses plaintext ENSDF data files (available [here](https://www.nndc.bnl.gov/ensarchivals/)) into a binary database, eventually one will be able to query this for information on various nuclei (levels, cascades, gamma-rays).

The goal is to develop a program that will be useful in both professional (nuclear structure research) and educational contexts.

## Features

TBD

## How to Install

### Dependencies

* gcc
* make
* SDL3
* SDL_image
* SDL_ttf

The current version has been tested under Arch Linux as of May 2024.

### Instructions

TBD.

For now SDL3 and its libraries probably has to be compiled manually, as they aren't (yet) packaged for major Linux distros.