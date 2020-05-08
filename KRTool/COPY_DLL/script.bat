
set projdir=%~1
set thirddir=%projdir%..\..\KRThird
set configdir=%~2
set platform=%~3
set outdir=%~4
set batchdir=%~5
set path=%path%;%batchdir%

if "%configdir%" equ "Debug" ( 
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
kcopy "%MYSQL_C_DIR%\lib\libmariadb.dll" "%outdir%libmariadb.dll"
kcopy "%thirddir%\fftw-3.3.5-dll-%platform%\libfftw3f-3.dll" "%OutDir%libfftw3f-3.dll"
kcopy "%thirddir%\angle\lib\%platform%\%configdir%\libEGL.dll" "%OutDir%libEGL.dll"
kcopy "%thirddir%\angle\lib\%platform%\%configdir%\libGLESv2.dll" "%OutDir%libGLESv2.dll"