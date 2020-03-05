if not "%1"=="all" goto cleanmain
cd manual
del *#marks#
cd NxDraw
del *#marks#
cd ..
cd NxStorage
del *#marks#
cd ..
cd tutorial
del *#marks#
cd ..
cd ..
cd src

move NxDraw\libpng\libpng.vcproj
rmdir /s /q NxDraw\libpng
mkdir NxDraw\libpng
move libpng.dsp NxDraw\libpng
move NxShared\zlib\zlib.vcproj
rmdir /s /q NxShared\zlib
mkdir NxShared\zlib
move zlib.dsp NxShared\zlib
cd ..

:cleanmain
cd src
call cleanup.bat
cd ..
del lib /q
call cleanproject manual\tutorial\NxDrawTest
call cleanproject samples\ball
call cleanproject samples\crossfade
call cleanproject samples\imageview
call cleanproject samples\rule
call cleanproject samples\FullScreen
call cleanproject samples\crossfaden
call cleanproject samples\rulen
call cleanproject samples\tilescroll
call cleanproject samples\screen
call cleanproject samples\alphamask
call cleanproject samples\LoadImage
call cleanproject samples\raster
call cleanproject samples\stretch
call cleanproject samples\mouse
call cleanproject samples\DIBImage
attrib -h samples\*.suo
del samples\*.suo
del samples\*.ncb
