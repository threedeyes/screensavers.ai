NAME = 3D-Pipes
TYPE = SHARED
APP_MIME_SIG = application/x-vnd.3DPipesScreensaver-AI
SRCS = 3d_pipes.cpp
LIBS = $(STDCPPLIBS) be screensaver GL GLU
OPTIMIZE := FULL

DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
