call "D:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
cd fftw-3.3.5-dll-x86
lib /machine:x86 /def:libfftw3f-3.def
lib /machine:x86 /def:libfftw3-3.def
lib /machine:x86 /def:libfftw3l-3.def
cd ../fftw-3.3.5-dll-x64
lib /machine:x64 /def:libfftw3f-3.def
lib /machine:x64 /def:libfftw3-3.def
lib /machine:x64 /def:libfftw3l-3.def
pause