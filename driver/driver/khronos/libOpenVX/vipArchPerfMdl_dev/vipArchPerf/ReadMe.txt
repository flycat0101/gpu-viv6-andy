Python call C/C++ code via SWIG:
1.Set up SWIG tool environment.
  path: D:\public\workspace\swigwin-3.0.12\swig.exe
2.Prepare C or C++ files.
  e.g. nnArchPerf.h and nnArchPerf.cpp
3.Prepare a SWIG interface file.
  e.g. nnArchPerf.i
4.Using the command to run SWIG will create two files. e.g. nnArchPerf_wrap.cxx and nnArchPerfModel.py
  4.1 swig.exe -c++ -python nnArchPerf.i
5.Prepare a configuration file. e.g. setup.py
6.Using python.distutils to generate the python dynamic lib.
  6.1 python setup.py build
7.Copy \\HW\projects.dev\arch\libNNArchPerf\libNNArchPerf\build\lib.win32-3.6\_nnArchPerfModel.cp36-win32.pyd and nnArchPerfModel.py to the folder "test"
8.Write your test.py in the folder "test" with nnArchPerfModel.py and _nnArchPerfModel.cp36-win32.pyd
8.1 You can see \\HW\projects.dev\arch\libNNArchPerf\libNNArchPerf\test\test.py as a sample.

NOTE:
1.When you update C or C++ code, you need to re-build VS project, then do some steps from item #4 to #7.


Excel call Python code via Xlwings:
1.Install python package "xlwings".
  pip install xlwings
2.At Excel side, we need to do the following steps:
  2.1 To open VBA press Alt+F11
  2.2 CKick "File" --> "import file" to import file "xlwings.bas".
      package "xlwings" path:C:\Program Files (x86)\Python36-32\Lib\site-packages\xlwings\xlwings.bas
  2.3 Write a VBA model to call python interface.
3.At Python side, we need to prepare python interface for VBA caller.