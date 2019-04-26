
set projdir=%1
set projdir=%projdir:"=%
set thirddir=%projdir%..\..\KRThird
set config=%2
set config=%config:"=%
set platform=%3
set outdir=%4
set outdir=%outdir:"=%
set batchdir=%5
set batchdir=%batchdir:"=%
set path=%path%;%batchdir%

if "%config%" equ "Debug" ( 
set d=d
kcopy "%thirddir%\curl\lib\%platform%\libcurl_debug.dll" "%outdir%libcurl_debug.dll"
) else (
kcopy "%thirddir%\curl\lib\%platform%\libcurl.dll" "%outdir%libcurl.dll"
)

kcopy "%thirddir%\zlib\%platform%\lib\zlib%d%.dll" "%outdir%zlib%d%.dll"
kcopy "%thirddir%\ken-res-loader\lib\%platform%\ken-res-loader%d%.dll" "%outdir%ken-res-loader%d%.dll"
kcopy "%thirddir%\ken-res-loader\lib\%platform%\libpng16%d%.dll" "%outdir%libpng16%d%.dll"
kcopy "%thirddir%\ken-res-loader\lib\%platform%\libvorbis%d%.dll" "%outdir%libvorbis%d%.dll"
kcopy "%thirddir%\ken-res-loader\lib\%platform%\libvorbisfile%d%.dll" "%outdir%libvorbisfile%d%.dll"
kcopy "%thirddir%\ken-res-loader\lib\%platform%\jpeg%d%.dll" "%outdir%jpeg%d%.dll"
kcopy "%MYSQL_C_DIR%\lib\libmysql.dll" "%outdir%libmysql.dll"
kcopy "%thirddir%\fftw-3.3.5-dll-%platform%\libfftw3f-3.dll" "%OutDir%libfftw3f-3.dll"