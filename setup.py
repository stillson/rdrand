from distutils.core import setup, Extension

setup(name='rdrand',
      version='0.2.3',
      description="Python interface to Intel hardware rng",
      long_description= \
      """python random.Random() subclass which uses a c
         extension to use the Intel processor extension
         to access a real random number generator.
      """,
      author="Chris Stillson",
      author_email="stillson@gmail.com",
      url='https://github.com/stillson/rdrand',
      license="New BSD license",
      ext_modules=[Extension('_rdrand', ['rdrand.c'])],
      py_modules = ['rdrand',],
)
