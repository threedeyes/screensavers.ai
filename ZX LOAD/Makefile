NAME = ZX-LOAD
TYPE = SHARED
SRCS = ZXSpectrumScreenSaver.cpp ZXSpectrumConfigView.cpp ZXSpectrumGLView.cpp ZXArtLoader.cpp main.cpp
LIBS = be GL screensaver curl
OPTIMIZE = FULL
WARNINGS = ALL

## include the makefile-engine
DEVEL_DIRECTORY := \
        $(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
