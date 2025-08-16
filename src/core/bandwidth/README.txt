
This is the README file for my program "bandwidth".

Bandwidth is a benchmark that measures memory bandwidth. 

This utility is useful because memory bandwidth needs to
measured to give you a clear idea of what your computer
is capable of. 

Merely relying on specs and marketing materials does not 
provide a full or even true picture of how the hardware
performs in real life.

--------------------------------------------------

My program "bandwidth" performs sequential and random
reads and writes and copies of varying sizes. This permits 
you to infer from the graph how well each type of memory 
is performing. So for instance when bandwidth
writes a 256-byte chunk, you know that because
caches are normally write-back, this chunk
will reside entirely in the L1 cache. Whereas
a 256-kilobyte chunk will mainly reside in L2.

You could run a non-artificial benchmark and 
observe that a general performance number is lower 
on one machine or higher on another, but that will
conceal the cause(s). 

So the purpose of this program is to help you hone in
on one cause of good or bad system performance.

This utility also attempts to show you the best-case scenario,
such as the maximum bandwidth achievable using sequential read
accesses, even if in the real world few programs or libraries
achieves that.

Release 1.12:
	- RISC-V support. Apple M2 support. Better OO organization. Fixed ARM bug.
Release 1.11:
	- AVX-512 support.
Release 1.10:
	- ARM 64 support, ARM 32 refinements. Apple M1 support.
Release 1.9:
	- More object-oriented improvements. Fixed Windows 64-bit support. Removed Linux framebuffer test.
Release 1.8:
	- More object-oriented improvements. Windows 64-bit supported.
Release 1.7:
	- Separated object-oriented C (OOC) from bandwidth app.
Release 1.6:
	- Converted the code to my conception of object-oriented C.
Release 1.5:
	- Fixed AVX bug. Added --nice mode and CPU temperature monitoring (OS/X only).
Release 1.4:
        - Added randomized 256-bit AVX reader & writer tests (Intel64 only).
Release 1.3:
        - Added CSV output. Updated ARM code for Raspberry π 3.
Release 1.2:
        - Put 32-bit ARM code back in.
Release 1.1:
	- Added larger font.
Release 1.0:
	- Moved graphing into BMPGraphing module.
	- Finally added LODS benchmarking, which
	  proves how badly lodsb/lodsw/lodsd/lodsq
	  perform.
	- Added switches --faster and --fastest.
Release 0.32:
	- Improved AVX support.
Release 0.31:
	- Adds cache detection for Intel 32-bit CPUs
	- Adds a little AVX support.
	- Fixes vector-to/from-main transfer bugs.
Release 0.30 adds cache detection for Intel 64-bit CPUs.
Release 0.29 improved graph granularity with more
	128-byte tests and removes ARM support.
Release 0.28 added a proper test of CPU features e.g. SSE 4.1.
Release 0.27 added finer-granularity 128-byte tests.
Release 0.26 fixed an issue with AMD processors.
Release 0.25 maked network bandwidth bidirectional.
Release 0.24 added network bandwidth testing.

Release 0.23 added:
	- Mac OS/X 64-bit support.
	- Vector-to-vector register transfer test.
	- Main register to/from vector register transfer test.
	- Main register byte/word/dword/qword to/from 
	  vector register test (pinsr*, pextr* instructions).
	- Memory copy test using SSE2.
	- Automatic checks under Linux for SSE2 & SSE4.

Release 0.22 added:
	- Register-to-register transfer test.
	- Register-to/from-stack transfer tests.

Release 0.21 added:
	- Standardized memory chunks to always be
	  a multiple of 256-byte mini-chunks.
	- Random memory accesses, in which each 
	  256-byte mini-chunk accessed is accessed 
	  in a random order, but also, inside each 
	  mini-chunk the 32/64/128 data are accessed
	  pseudo-randomly as well. 
	- Now 'bandwidth' includes chunk sizes that 
	  are not powers of 2, which increases 
	  data points around the key chunk sizes 
	  corresponding to common L1 and L2 cache 
	  sizes.
	- Command-line options:
		--fast for 0.25 seconds per test.
		--slow for 20 seconds per test.
		--title for adding a graph title.

Release 0.20 added graphing, with the graph
stored in a BMP image file. It also adds the
--slow option for more precise runs.

Release 0.19 added a second 128-bit SSE writer
routine that bypasses the caches, in addition
to the one that doesn't.

Release 0.18 was my Grand Unified bandwidth
benchmark that brought together support for
four operating systems:
	- Linux
	- Windows Mobile
	- 32-bit Windows
	- Mac OS/X 64-bit
and two processor architectures:
	- x86
	- Intel64
I've written custom assembly routines for
each architecture.

--------------------------------------------------
This program is provided without any warranty
and AS-IS. See the file GPL.txt for details.

Zack Smith
1@zsmith.co

