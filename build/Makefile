PROJDIR := $(realpath $(CURDIR)/..)
SOURCEDIR := $(PROJDIR)/src
BUILDDIR := $(PROJDIR)/build
# Configuration filename
CONFFILE = sp2.cfg

# Decide whether the commands will be shwon or not
VERBOSE = TRUE

# target executable
TARGET=gl_configurator

DIRS = config cli
SOURCEDIRS = $(foreach dir, $(DIRS), $(addprefix $(SOURCEDIR)/, $(dir)))
TARGETDIRS = $(foreach dir, $(DIRS), $(addprefix $(BUILDDIR)/, $(dir)))

# Generate the GCC includes parameters by adding -I before each source folder
INCLUDES = $(foreach dir, $(SOURCEDIRS), $(addprefix -I, $(dir)))

# Add this list to VPATH, the place make will look for the source files
VPATH = $(SOURCEDIRS)

# Create a list of *.c sources in DIRS
SOURCES = $(foreach dir,$(SOURCEDIRS),$(wildcard $(dir)/*.c))

# Define objects for all sources
OBJS := $(subst $(SOURCEDIR),$(BUILDDIR),$(SOURCES:.c=.o))

# Define dependencies files for all objects
DEPS = $(OBJS:.o=.d)

# C compiler
CC=gcc

# OS specific part
ifeq ($(OS),Windows_NT)
	RM = del /F /Q 
	RMDIR = -RMDIR /S /Q
	MKDIR = -mkdir
	ERRIGNORE = 2>NUL || true
	SEP=\\
else
	RM = rm -rf 
	RMDIR = rm -rf 
	MKDIR = mkdir -p
	ERRIGNORE = 2>/dev/null
	SEP=/
endif

# Remove space after separator
PSEP = $(strip $(SEP))

# Hide or not the calls depending of VERBOSE
ifeq ($(VERBOSE),TRUE)
	HIDE =  
else
	HIDE = @
endif

# C flags
CFLAGS=-Wall -Wextra -O2 -g -DCONF_FILE=${CONFFILE}
# Linking flags
LDFLAGS=

# Define the function that will generate each rule
define generateRules
$(1)/%.o: %.c
	@echo Building $$@
	$(HIDE)$(CC) -c $$(INCLUDES) $$(CFLAGS) -o $$(subst /,$$(PSEP),$$@) $$(subst /,$$(PSEP),$$<) -MMD
endef

all: directories $(TARGET)

$(TARGET): $(OBJS)
	$(HIDE)echo Linking $@
	$(HIDE)$(CC) $(LDFLAGS) $(OBJS) -o $(TARGET)

# Include dependencies
-include $(DEPS)

# Generate rules
$(foreach targetdir, $(TARGETDIRS), $(eval $(call generateRules, $(targetdir))))

directories: 
	$(HIDE)$(MKDIR) $(subst /,$(PSEP),$(TARGETDIRS)) $(ERRIGNORE)

# Remove all objects, dependencies and executable files generated during the build
clean:
	$(HIDE)$(RMDIR) $(subst /,$(PSEP),$(TARGETDIRS)) $(ERRIGNORE)
	$(HIDE)$(RM) $(TARGET) $(ERRIGNORE)
	@echo Cleaning done ! 

test: $(TARGET)
	./$(TARGET) add "0/0" "name1" "value1"
	./$(TARGET) add "0/1-0/3" "nameRange" "valueRange"
	./$(TARGET) add "1/1-2/2" "nameRange2" "valueRange2"
	./$(TARGET) add "0/2" "nameRange" "valueNew"
	./$(TARGET) get "0/2" "nameRange"
	./$(TARGET) get "0/0" "nameNotExist"
	./$(TARGET) get "2/2" "nameSome"
	./$(TARGET) del "0/3" "nameRange"
	./$(TARGET) del "0/0" "nameNotExist"
	./$(TARGET) del "2/2" "nameSome"

test_get: $(TARGET)
	./$(TARGET) get "1/2" "name1"