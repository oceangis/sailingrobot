###############################################################################
#
# Makefile for building the current sensor tests for ASPire
#
# This makefile cannot be run directly. Use the master makefile instead.
#
###############################################################################


###############################################################################
# Files
###############################################################################

# Source files
MAIN_CURRENT_SENSOR_TESTS 	= Tests/IntegrationTests/CurrentSensorTest.cpp

SRC 					= $(MAIN_CURRENT_SENSOR_TESTS) $(CORE_SRC) $(HW_SERVICES_ALL_SRC) \
							$(CAN_SERVICES_SRC) $(HW_NODES_ALL_SRC) $(HW_NODES_ASPIRE_SRC)


# Object files
OBJECTS = $(addprefix $(BUILD_DIR)/, $(SRC:.cpp=.o))


###############################################################################
# Rules
###############################################################################

all: $(CURRENT_SENSOR_TEST_EXEC) stats

# Link and build
$(CURRENT_SENSOR_TEST_EXEC): $(OBJECTS)
	rm -f $(OBJECT_FILE)
	@echo -n " " $(OBJECTS) >> $(OBJECT_FILE)
	@echo Linking object files
	$(CXX) $(LDFLAGS) @$(OBJECT_FILE) -Wl,-rpath=./ -o $@ $(LIBS)

# Compile CPP files into the build folder
$(BUILD_DIR)/%.o:$(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo Compiling CPP File: $@
	@$(CXX) -c $(CPPFLAGS) $(INC_DIR) -o ./$@ $< $(DEFINES) $(LIBS)

stats:$(CURRENT_SENSOR_TEST_EXEC)
	@echo Final executable size:
	$(SIZE) $(CURRENT_SENSOR_TEST_EXEC)
