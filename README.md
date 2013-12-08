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
