NAME = Starfield
TYPE = SHARED
APP_MIME_SIG = application/x-vnd.StarfieldScreensaver-AI
SRCS = Starfield.cpp
LIBS = $(STDCPPLIBS) be screensaver GL GLU glut
OPTIMIZE := FULL

DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
