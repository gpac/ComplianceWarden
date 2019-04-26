#------------------------------------------------------------------------------
# Stuff mostly project agnostic

BIN?=bin

all: everything

$(BIN)/%.exe:
	@mkdir -p $(dir $@)
	$(CXX) -o "$@" $^ $(LDFLAGS)

$(BIN)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o "$@" "$<"
	@$(CXX) $(CXXFLAGS) -c -o "$@.deps" "$<" -MP -MM -MT "$@"
	@$(CXX) $(CXXFLAGS) -c "$<" -E | wc -l > "$@.lines" # keep track of the code mass

VERSION?=HEAD
$(BIN)/%/version.cpp.o: CXXFLAGS+=-DVERSION="\"$(VERSION)\""

clean:
	rm -rf $(BIN)

-include $(shell test -d $(BIN) && find $(BIN) -name "*.deps")
