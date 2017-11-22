
# Copyright (c) 2013, Chris Stillson <stillson@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

from random import Random
import _rdrand

if not _rdrand.HAS_RAND == 1:
    print( "This module requires a cpu which supports the" +\
          " RdRand instruction")
    raise SystemError

rdrand_get_bits = _rdrand.rdrand_get_bits
rdrand_get_bytes = _rdrand.rdrand_get_bytes

if _rdrand.HAS_SEED == 1:
    rdseed_get_bits = _rdrand.rdseed_get_bits
    rdseed_get_bytes = _rdrand.rdseed_get_bytes


class BaseRandom(Random):
    """Base class for alternate random number generator using
     Intel's RdRand or RdSeed instructions to access the
     hardware random number generator. Not available on all
     systems (see os.urandom() for details).
    """

    get_bits = None
    get_bytes = None

    def random(self):
        """Get the next random number in the range [0.0, 1.0)."""
        return (1.0 * self.get_bits(52)) / (2 ** 52)

    def getrandbytes(self, k):
        if k <= 0:
            raise ValueError('number of bytes must be greater than zero')
        if k != int(k):
            raise TypeError('number of bytes should be an integer')
        return self.get_bytes(k)

    def getrandbits(self, k):
        """getrandbits(k) -> x.  Generates a long int with k random bits."""
        if k <= 0:
            raise ValueError('number of bits must be greater than zero')
        if k != int(k):
            raise TypeError('number of bits should be an integer')
        return self.get_bits(k)

    def _stub(self, *args, **kwds):
        "Stub method.  Not used for a system random number generator."
        return None
    seed = jumpahead = _stub

    def _notimplemented(self, *args, **kwds):
        "Method should not be called for a system random number generator."
        raise NotImplementedError('System entropy source does not have state.')
    getstate = setstate = _notimplemented



if _rdrand.HAS_RAND == 1:
    class RdRandom(BaseRandom):
        get_bits = rdrand_get_bits
        get_bytes = rdrand_get_bytes

if _rdrand.HAS_SEED == 1:
    class RdSeedom(BaseRandom):
        get_bits = rdseed_get_bits
        get_bytes = rdseed_get_bytes