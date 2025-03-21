## Introduction

This is a tiny emacs-like editor simplified from
	<https://github.com/torvalds/uemacs>


## Goal

- Be more compatible with GNU emacs on basic operations.
- Remove unnecessary parts to make it tiny.


## Miscellaneous

To get the priority information:
```sh
update-alternatives --display editor
```

```sh
sudo update-alternatives --install /usr/bin/editor editor /usr/bin/em 100
```

If the priority 100 is still too low, set it manually:
```sh
sudo update-alternatives --set editor /usr/bin/em
```
