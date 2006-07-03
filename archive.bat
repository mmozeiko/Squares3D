@echo off
rmdir /S /Q Debug 1>nul 2>nul
rmdir /S /Q Release 1>nul 2>nul
rmdir /S /Q 3rdparty\Debug 1>nul 2>nul
rmdir /S /Q 3rdparty\Release 1>nul 2>nul
del /F /S /Q /A:H *.suo 1>nul 2>nul
del /F /S /Q *.ncb *.pdb *.ilk *.vcproj.*.user 1>nul 2>nul
rar a -ag -m5 -mdG -r -s -x*.rar Squares3D_
