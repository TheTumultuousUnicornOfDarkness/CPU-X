
This is the README file for my program, "bandwidth".

Bandwidth is a benchmark that attempts to measure
memory bandwidth. In December 2010 (and as of
release 0.24), I extended 'bandwidth' to measure 
network bandwidth as well.

Bandwidth is useful because both memory bandwidth
and network bandwidth need to be measured to
give you a clear idea of what your computer(s) can do.
Merely relying on specs does not give a full picture
and indeed specs can be misleading.

--------------------------------------------------
MEMORY BANDWIDTH 

My program bandwidth performs sequential and random
reads and writes of varying sizes. This permits 
you to infer from the graph how each type of memory 
is performing. So for instance when bandwidth
writes a 256-byte chunk, you know that because
caches are normally write-back, this chunk
will reside entirely in the L1 cache. Whereas
a 512 kB chunk will mainly reside in L2.

You could run a non-artificial benchmark and 
observe that a general performance number is lower 
on one machine or higher on anotehr, but that may
conceal the cause. 

So the purpose of this program is to help you 
pinpoint the cause of a performance problem,
or to affirm a general impression about a memory-
intensive program. 

It also tells you the best-case scenario e.g.
the maximum bandwidth achieved using sequential,
128-bit memory accesses.

Release 1.3:
        - Added CSV output. Updated ARM code for Raspberry π 3.
Release 1.2:
        - Added ARM code back in.
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

Total run time for the default speed, which
has 5 seconds per test, is about 35 minutes.

--------------------------------------------------
NETWORK BANDWIDTH (beginning with release 0.24)

In mid-December 2010, I extended bandwidth to measure
network bandwidth, which is useful for testing
your home or workplace network setup, and in theory
could be used to test machines across the Internet.

Release 0.25 adds:
	- Bidirectional network bandwidth testing.
	- Specifiable port# (default is 49000).

In the graph:
	- Sent data appears as a solid line.
	- Received data appears as a dashed line.

The network test is pretty simple. It sends chunks
of data of varying sizes to whatever computers
(nodes) that you specify. Each of those must be
running 'bandwidth' in transponder mode.

The chunks of data range of 32 kB up to 32 MB.
These are actually send as a stream of 1 or more
32 kB sub-chunks.

Sample output:
	output/Network-Linux2.6-Celeron-2.8GHz-32bit-loopback.bmp
	output/Network-MacOSX32-Corei5-2.4GHz-64bit-loopback.bmp
	output/Network-Mac64-Linux32.bmp

How to start a transponder:
	./bandwidth-mac64 --transponder

Example invocation of the test leader:
	./bandwidth64 --network 192.168.1.104

I've tested network mode on:
	Linux 32-bit
	Mac OS/X 32- and 64-bit
	Win/Cygwin 32-bit.

--------------------------------------------------
This program is provided without any warranty
and AS-IS. See the file COPYING for details.

Zack Smith
1@zsmith.co
March 2013

