rdrand
======

python interface to intel hardware RNG

to use:

python setup.py install

    >>from rdrand import RdRandom
    >>r = RdRandom()

RdRandom is a subclass of random.Random, provides everthing that random.Random 
provides, but uses inline assembly to access the hardware RNG. should be
a cryptographically secure drop in replacement for random.Random (if you
trust Intel...)

added function r.getrandombytes(i) where i is a positive it. Returns a string
of length i filled with random bytes.
