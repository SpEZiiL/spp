# Script Preprocessor #

![status][status-img]
[![version][version-img]][version-link]
[![lang: C][lang-c-img]][lang-c-link]

**spp** is a preprocessor for scripts.

**spp** is mainly supposed to be used to build big scripts that have been split
up into several pieces.

## Usage ##

The executable can be passed a single file argument.  
The entire file is processed and the output will be written to `stdout`.
If no argument is specified or `-` is passed down, **spp** will read `stdin`
instead.

## Directives ##

The preprocessor directives of **spp** look similiar to the directives of the
**C** and **C++** preprocessor.

**Synopsis:** `[whitespace]#<command>[<whitespace><argument>]`

_WHITESPACE_ may be either spaces or tabs.  
_COMMAND_ may be any characters BUT spaces and tabs.  
_ARGUMENT_ may be ANY character. Note that trailing whitespace will also count
to the argument.

The following commands are defined:

* `insert <file>`  
  Inserts contents of _FILE_ into this position.
* `include <file>`  
  Inserts contents of _FILE_ into this position after running **spp** through it.
* `ignore` and `end-ignore`  
  Delete this and the following lines from the final output until `end-ignore`
  is seen.
* `ignorenext`  
  Deletes this and the following line from the final output.

<!-- Shields -->

[status-img]: https://img.shields.io/badge/dynamic/json.svg?label=status&url=http%3A%2F%2Fspeziil.ddns.net%2Frepos%2Fspp.json&query=%24.status&colorB=brightgreen
[version-img]: https://img.shields.io/badge/dynamic/json.svg?label=version&url=http%3A%2F%2Fspeziil.ddns.net%2Frepos%2Fspp.json&query=%24.version&colorB=blue
[lang-c-img]: http://speziil.ddns.net/shields/lang/c/c11.svg

[version-link]: https://github.com/SpEZiiL/spp/releases/latest
[lang-c-link]: http://speziil.ddns.net/shields/lang/c/c11.html
