
from random import Random
from _rdrand import rdrand_get_bits


class RdRandom(Random):
    """Alternate random number generator using intel's rdrand instructions
     to access the hardware random number generator.
     Not available on all systems (see os.urandom() for details).
    """

    def random(self):
        """Get the next random number in the range [0.0, 1.0)."""
        return (1.0 * rdrand_get_bits(16)) / (2 ** 16)

    def getrandbits(self, k):
        """getrandbits(k) -> x.  Generates a long int with k random bits."""
        if k <= 0:
            raise ValueError('number of bits must be greater than zero')
        if k != int(k):
            raise TypeError('number of bits should be an integer')
        return rdrand_get_bits(k)

    def _stub(self, *args, **kwds):
        "Stub method.  Not used for a system random number generator."
        return None
    seed = jumpahead = _stub

    def _notimplemented(self, *args, **kwds):
        "Method should not be called for a system random number generator."
        raise NotImplementedError('System entropy source does not have state.')
    getstate = setstate = _notimplemented

