# Bayerisches Landesamt fuer Steuern, Muenchen - Elster ERiC
# (c) 2024, LfSt Bayern, Muenchen, IuK 111

# Diese Variable muss eventuell angepasst werden
ERIC_INSTALL=../..
ERIC_INCLUDE=$(ERIC_INSTALL)/include

INC=-I$(ERIC_INCLUDE)

CXXFLAGS=-m64 -std=c++14 -g $(INC)
LDFLAGS=-m64 -ldl

REL=ottodemo/Release
DEB=ottodemo/Debug

SOURCE=ottodemo.cpp Arguments.cpp OttoWrapper.cpp

OBJECTS=$(SOURCE:%.cpp=$(DEB)/%.o)

.PHONY: all
all: $(REL) $(DEB) $(REL)/ottodemo $(DEB)/ottodemo

$(DEB):
	mkdir $(DEB)

$(REL):
	mkdir $(REL)

$(DEB)/%.o: ottodemo/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(DEB)/ottodemo: $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)

$(REL)/ottodemo: $(DEB)/ottodemo
	strip -o $@ $<

# Automatische Abhaengigkeiten

$(DEB)/%.d: ottodemo/%.cpp $(DEB)
	$(CXX) -MM $(INC) $< | sed "s;$(notdir $*).o:;$(DEB)/$*.o $(DEB)/$*.d:;" > $@

.PHONY: clean
clean:
	rm -f $(DEB)/*.o $(DEB)/ottodemo $(REL)/ottodemo $(DEB)/*.d

-include $(SOURCE:%.cpp=$(DEB)/%.d)

