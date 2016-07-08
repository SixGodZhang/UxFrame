all:
	cl.exe /D MYTEST /EHsc /MT /I ../../3rdparty/curl-7.25.0/include /c /nologo HttpDownloader.cpp
	link.exe /nologo /SUBSYSTEM:CONSOLE /LIBPATH:"../../3rdparty/curl-7.25.0/lib/Release" HttpDownloader.obj

clean:
	del /S *.obj *.exe

