RDRAND

A module to use Intel's hardware RNG with python's random class

USAGE

#easy_install rdrand
#python

>>>from rdrand import RdRandom
>>>r = RdRandom()


At this point, ``r`` will behave just like ``random``

``RdRandom`` is a subclass of ``random.Random``, and behaves like ``random.Random``,
 but it uses inline assembly to access the hardware RNG. This should be
a cryptographically secure drop in replacement for ``random``, if the Intel random number
generator is valid. No mitigation is done to modify the output of the hardware to prevent problems with Intel's implementation. Caveat Emptor.

Also, it includes the function ``r.getrandombytes(i)`` where ``i`` is a positive int. This returns a string
of length ``i`` filled with random bytes, which is ideal for generating a key or using directly in a protocol.

Please note, as with any security solution, it is possible to subvert this. Please understand the full context before deploying. I am not liable for misuse or clever hackers.
