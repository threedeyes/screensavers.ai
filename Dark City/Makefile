NAME = DarkCity
TYPE = SHARED
APP_MIME_SIG = application/x-vnd.DarkCityScreensaver-AI
SRCS = dark_city.cpp
LIBS = $(STDCPPLIBS) be screensaver GL GLU glut
OPTIMIZE := FULL

DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
