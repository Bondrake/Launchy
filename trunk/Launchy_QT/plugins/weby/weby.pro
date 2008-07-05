 TEMPLATE      = lib
 CONFIG       += plugin debug_and_release
 VPATH 		  += ../../src/
 INCLUDEPATH += ../../src/
 UI_DIR		   = ../../plugins/weby/
 FORMS		   = dlg.ui
 HEADERS       = plugin_interface.h weby.h gui.h globals.h
 SOURCES       = plugin_interface.cpp weby.cpp gui.cpp
 TARGET		   = weby
 
 win32 {
 	CONFIG -= embed_manifest_dll
	LIBS += shell32.lib
%	LIBS += user32.lib
%	LIBS += Gdi32.lib
%	LIBS += comctl32.lib
}
 
 *:debug {
        CONFIG -= release
	DESTDIR = ../../debug/plugins/
 }
 *:release {
	DESTDIR = ../../release/plugins/
	%QMAKE_CXXFLAGS += /Ox /Ob2 /Oi /Oy /GT /GA  /WX
 }

unix {
 target.path = /usr/lib/launchy/plugins/
 INSTALLS += target

 icon.path = /usr/share/launchy/plugins/icons/
 icon.files = weby.ico
 INSTALLS += icon
}
