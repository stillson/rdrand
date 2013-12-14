/*
 * Copyright (c) 2013, Chris Stillson <stillson@gmail.com>
 * All rights reserved.
 *
 * portions of this code
 * Copyright � 2012, Intel Corporation.  All rights reserved.
 * (Namely, the cpuid checking code)
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

#ifdef __i386__
#define IS32BIT 1
#else
#define IS64BIT 1
#endif

#ifdef __GNUC__
#define USING_GCC 1
#elif __clang__
#define USING_CLANG 1
#else
#error Only support for gcc or clang currently
#error if you port to another compiler, please
#error send back the patch to https://github.com/stillson/rdrand
#endif

#if IS32BIT
unsigned long long int get_bits(void);
#else
unsigned long int get_bits(void);
#endif
int RdRand_cpuid(void);

PyDoc_STRVAR(module_doc, "rdrand: Python interface to intel hardware rng\n");

/*! \brief Queries cpuid to see if rdrand is supported
 *
 * rdrand support in a CPU is determined by examining the 30th bit of the ecx
 * register after calling cpuid.
 *
 * \return bool of whether or not rdrand is supported
 */

/*! \def RDRAND_MASK
 *    The bit mask used to examine the ecx register returned by cpuid. The
 *   30th bit is set.
 */
#define RDRAND_MASK   0x40000000

# define __cpuid(x,y) asm("cpuid":"=a"(x[0]),"=b"(x[1]),"=c"(x[2]),"=d"(x[3]):"a"(y))

void
cpuid(unsigned int op, unsigned int reg[4])
{

#if USING_GCC && IS64BIT
    __cpuid(reg, op);
#else
    asm volatile("pushl %%ebx      \n\t" /* save %ebx */
                 "cpuid            \n\t"
                 "movl %%ebx, %1   \n\t" /* save what cpuid just put in %ebx */
                 "popl %%ebx       \n\t" /* restore the old %ebx */
                 : "=a"(reg[0]), "=r"(reg[1]), "=c"(reg[2]), "=d"(reg[3])
                 : "a"(op)
                 : "cc");
#endif
}

int
RdRand_cpuid(void)
{
    unsigned int info[4] = {-1, -1, -1, -1};

    /* Are we on an Intel processor? */
    cpuid(0, info);

    if ( memcmp((void *) &info[1], (void *) "Genu", 4) != 0 ||
        memcmp((void *) &info[3], (void *) "ineI", 4) != 0 ||
        memcmp((void *) &info[2], (void *) "ntel", 4) != 0 )
        return 0;

    /* Do we have RDRAND? */
    cpuid(1, info);

    int ecx = info[2];
    if ((ecx & RDRAND_MASK) == RDRAND_MASK)
        return 1;
    else
        return 0;
}

#if IS64BIT
//utility to return 64 random bits
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
#if USING_GCC
        asm volatile(".byte 0x48; .byte 0x0f; .byte 0xc7; .byte 0xf0; setc %1":"=a"(rando), "=qm"(err));

#elif USING_CLANG
        asm("rdrandq %0;\n\t" "setc %1" :"=a"(rando),"=qm"(err) : :);
#endif

    } while (err == 0);

    return rando;
}
#elif IS32BIT
unsigned long long int
get_bits(void)
{
    unsigned long long int rando = 0;
    unsigned char err = 0;
    unsigned int rando1, rando2;
    union{
       unsigned long long int rando;
       struct {
          unsigned int rando1;
          unsigned int rando2;
       } i;
    } un;

    // Yes, this is inline assembly.
    // should never really fail, may have
    // to reexamine for future versions
    do
    {
#if USING_GCC
        asm volatile(".byte 0x0f; .byte 0xc7; .byte 0xf0; setc %1":"=a"(un.i.rando1), "=qm"(err));
#elif USING_CLANG
        asm("rdrandw %0;\n\t" "setc %1" :"=a"(un.i.rando1),"=qm"(err) : :);
#endif
    } while (err == 0);

    do
    {
#if USING_GCC
        asm volatile(".byte 0x0f; .byte 0xc7; .byte 0xf0; setc %1":"=a"(un.i.rando2), "=qm"(err));
#elif USING_CLANG
        asm("rdrandw %0;\n\t" "setc %1" :"=a"(un.i.rando2),"=qm"(err) : :);
#endif
    } while (err == 0);

    return un.rando;
}
#endif

static PyObject *
rdrand_get_bits(PyObject *self, PyObject *args)
{
    int num_bits, num_bytes, i;
    int num_quads, num_chars;
    unsigned char * data = NULL;
#if IS32BIT
    unsigned long long int rando;
#else
    unsigned long int rando;
#endif
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

static PyObject *
rdrand_get_bytes(PyObject *self, PyObject *args)
{
    int num_bytes, i;
    int num_quads, num_chars;
    unsigned char * data = NULL;
    unsigned long int rando;
    PyObject *result;

    if ( !PyArg_ParseTuple(args, "i", &num_bytes) )
        return NULL;

    if (num_bytes <= 0)
    {
        PyErr_SetString(PyExc_ValueError, "number of bytes must be greater than zero");
        return NULL;
    }

    data        = (unsigned char *)PyMem_Malloc(num_bytes);
    num_quads   = num_bytes / 8;
    num_chars   = num_bytes % 8;

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

    /* Probably hosing byte order. big deal it's hardware random, has no meaning til we assign it */
    result = Py_BuildValue("s#", data, num_bytes);
    PyMem_Free(data);
    return result;
}

static PyMethodDef rdrand_functions[] = {
        {"rdrand_get_bits",       rdrand_get_bits,        METH_VARARGS, "rdrand_get_bits()"},
        {"rdrand_get_bytes",      rdrand_get_bytes,       METH_VARARGS, "rdrand_get_bytes()"},
        {NULL,      NULL}   /* Sentinel */
};

PyMODINIT_FUNC
init_rdrand(void)
{
        PyObject *m;

        // I need to verify that cpu type can do rdrand
        if (RdRand_cpuid() != 1)
        {
            PyErr_SetString(PyExc_SystemError,
                        "CPU doesn't have random number generator");
            return;
        }

        m = Py_InitModule3("_rdrand", rdrand_functions, module_doc);
        if (m == NULL)
            return;
}
