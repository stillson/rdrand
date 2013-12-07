/*
 * Copyright (c) 2013, Chris Stillson <stillson@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

unsigned long int get_bits(void);

PyDoc_STRVAR(module_doc, "rdrand: Python interface to intel hardware rng\n");

//utility to return 64 random bytes
unsigned long int
get_bits(void)
{
    unsigned long int rando = 0;
    unsigned char err = 0;

    // Yes, this is inline assembly.
    // should never really fail, may have
    // to reexamine for future versions
    do
    {
        asm("rdrandq %0;\n\t"
            "setc %1"
            :"=a"(rando),"=qm"(err)
            :
            :);

    } while (err == 0);

    return rando;
}

static PyObject *
rdrand_get_bits(PyObject *self, PyObject *args)
{
    int num_bits, num_bytes, i;
    int num_quads, num_chars;
    unsigned char * data = NULL;
    unsigned long int rando;
    unsigned char last_mask, lm_shift;
    PyObject *result;

    if ( !PyArg_ParseTuple(args, "i", &num_bits) )
        return NULL;

    if (num_bits <= 0)
    {
        PyErr_SetString(PyExc_ValueError, "number of bits must be greater than zero");
        return NULL;
    }

    num_bytes   = num_bits / 8;
    lm_shift    = num_bits % 8;
    last_mask   = 0xff >> (8 - lm_shift);

    if (lm_shift)
        num_bytes++;

    num_quads   = num_bytes / 8;
    num_chars   = num_bytes % 8;
    data        = (unsigned char *)PyMem_Malloc(num_bytes);

    if (data == NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }

    for(i = 0; i < num_quads; i++)
    {
        rando = get_bits();
        bcopy((char*)&rando, &data[i * 8], 8);
    }

    if(num_chars)
    {
        rando = get_bits();
        bcopy((char*)&rando, &data[num_quads * 8], num_chars);
    }

    if (lm_shift != 0)
        data[num_bytes -1] &= last_mask;

    /* Probably hosing byte order. big deal it's hardware random, has no meaning til we assign it */
    result = _PyLong_FromByteArray(data, num_bytes, 1, 0);
    PyMem_Free(data);
    return result;
}

static PyMethodDef rdrand_functions[] = {
        {"rdrand_get_bits",       rdrand_get_bits,        METH_VARARGS, "rdrand_get_bits()"},
        {NULL,      NULL}   /* Sentinel */
};

PyMODINIT_FUNC
init_rdrand(void)
{
        PyObject *m;

        m = Py_InitModule3("_rdrand", rdrand_functions, module_doc);
        if (m == NULL)
            return;
}
