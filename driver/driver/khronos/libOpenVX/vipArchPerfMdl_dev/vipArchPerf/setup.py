from distutils.core import setup, Extension

nnArchPerfModel_module = Extension('_nnArchPerfModel',
                              sources=['nnArchPerf_wrap.cxx','nnArchPerf.cpp']
                              )
setup(
    name = 'nnArchPerfModel',
    version = '0.1',
    author = 'SWIG Docs',
    description = 'Simple swig example from docs',
    ext_modules = [nnArchPerfModel_module],
    py_modules = ['nnArchPerfModel'],

    )
