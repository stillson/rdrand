RDRAND
------

A module to use Intel's hardware RNG with python's random class

USAGE
=====


| # easy_install rdrand
| # python

>>> from rdrand import RdRandom
>>> r = RdRandom()

>>>from rdrand import RdSeedom
>>>s = RdSeedom()

At this point, ``r`` and ``s`` will behave just like ``random``

``RdRandom`` is a subclass of ``random.Random``, and behaves like ``random.Random``, but it uses inline assembly to access the hardware RNG using the RdRand instruction. This should be a cryptographically secure drop in replacement for ``random`` with a prediction complexity bound of O(2^128), if the Intel random number generator is valid. No mitigation is done to modify the output of the hardware to prevent problems with Intel's implementation. Caveat Emptor.

``RdSeedom`` is a subclass of ``random.Random``, and behaves like ``random.Random``, but it uses inline assembly to access the hardware RNG using the RdSeed instruction. This should be a cryptographically secure drop in replacement for ``random`` returning full entropy bits, if the Intel random number generator is valid. No mitigation is done to modify the output of the hardware to prevent problems with Intel's implementation. Caveat Emptor.

Also, both RdRandom and RdSeedom include the function ``r.getrandombytes(i)`` where ``i`` is a positive int. This returns a string of length ``i`` filled with random bytes, which is ideal for generating a key or using directly in a protocol.

Please note, as with any security solution, it is possible to subvert this. Please understand the full context before deploying. I am not liable for misuse or clever hackers.

Works with 32 and 64 bit builds of python.

Works with python2 and python3.

planned for version 1.5: whitening added, written in python

Planned for version 2: whitening added, backwards compatible with 1.5, but written in C.

