from setuptools import setup, Extension, find_packages

setup(name='rdrand',
      version='0.1',
      description="python interface to intel hardware rng",
      author="Chris Stillson",
      author_email="stillson@gmail.com",
      license="New BSD license",
      ext_modules=[Extension('_rdrand', ['rdrand.c'])],
      py_modules = ['rdrand',],
)
