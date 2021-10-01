# Motif

This is Motif 2.1, the industry standard X11 widget toolkit developed
by The Open Group.  This repository aims to integrate fixes and
enhancements from the various Motif vendors and host new development
of portability and stability improvements while retaining API and ABI
compatibility with the 2.1 series.

## Installation

This version of Motif uses Advanced Software Technology (AST)
[NMAKE][1] as a replacement for IMAKE.

Motif may be compiled by running `nmake` within its root directory.

## Advantages of Motif 2.1

Motif 2.1 was the last formal release of Motif by The Open Group, and
was subjected to a rigorous standardization and quality assurance
process by a multitude of consortium members.  For developers, the
benefit is that applications written against the Motif 2.1 API have a
high likelihood of successfully compiling on a variety of UNIX
platforms.  The Open Group also took pains to ensure the stability of
the Motif 2.1 ABI throughout the series.

## Post-2.1 Motif Developments

Two of the foremost Motif vendors, [Imperial Software Technology][2]
(IST) and [Integrated Computer Solutions][3] (ICS), continued to
develop Motif after The Open Group ceased the release of new versions.

IST published a comprehensive [analysis][4] of the design issues found
in some of these later Motif releases.

### ICS Motif

ICS has [released][5] their own versions (2.2 and 2.3) of Motif that
include unique extensions.  If you are running a free UNIX-like system
such as Linux or one of the BSDs, this is most likely the version of
Motif packaged by your distribution.

### IST Motif

IST has [published][6] a number of refinements to the Motif 2.1
series, which this repository includes.

[1]: https://sr.ht/~lkujaw/ast/
[2]: http://www.ist-inc.com/
[3]: https://motif.ics.com/
[4]: http://www.motifdeveloper.com/tips/Motif22Review.pdf
[5]: https://sourceforge.net/projects/motif/
[6]: http://www.ist-inc.com/motif/download/index.html
