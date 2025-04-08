## Introduction

This is a tiny emacs-like editor simplified from <https://github.com/torvalds/uemacs>.


## Goal

- Be more compatible with GNU emacs.
- Be tiny. (by removing unecessary parts like the script engine)


## Miscellaneous

We can make `me` the default `editor` of the system with `update-alternatives`.

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
