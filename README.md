## Introduction

This is a tiny emacs-like editor modified from
[uEmacs/PK 4.0](https://github.com/torvalds/uemacs).


## Goal

- Be more compatible with GNU emacs.
- Be tiny. (by removing unecessary parts like the script engine)


## Miscellaneous

We can make `me` (**M**odified micro **E**macs) the default `editor` of the
system with `update-alternatives`.

To get the priority of `editor`:
```sh
update-alternatives --display editor
```

```sh
sudo update-alternatives --install /usr/bin/editor editor /usr/bin/me 100
```

If the priority 100 is still too low, set it manually:
```sh
sudo update-alternatives --set editor /usr/bin/me
```

## Debug

There is a program for getting raw input from terminal, which is useful for
debugging.  Build it like this:

```sh
make showkeys
```
