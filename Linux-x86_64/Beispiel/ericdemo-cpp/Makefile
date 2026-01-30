# Bayerisches Landesamt fuer Steuern, Muenchen - Elster ERiC
# (c) 2012, LfSt Bayern, Muenchen, IuK 111

# Diese Variable muss eventuell angepasst werden
ERIC_INSTALL=../..
ERIC_INCLUDE=$(ERIC_INSTALL)/include

INC=-I$(ERIC_INCLUDE)

CXXFLAGS=-m64 -std=c++11 -g $(INC)
LDFLAGS=-m64 -ldl

REL=ericdemo/Release
DEB=ericdemo/Debug

SOURCE=datensatzleser.cpp ericdemo.cpp ericdekodierung.cpp \
	callbackhandler.cpp ericpuffer.cpp ericsystemsteuerung.cpp \
	ericvorgang.cpp ericzertifikat.cpp eric.cpp system.cpp

OBJECTS=$(SOURCE:%.cpp=$(DEB)/%.o)

.PHONY: all
all: $(REL) $(DEB) $(REL)/ericdemo $(DEB)/ericdemo

$(DEB):
	mkdir $(DEB)

$(REL):
	mkdir $(REL)

$(DEB)/%.o: ericdemo/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(DEB)/ericdemo: $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)

$(REL)/ericdemo: $(DEB)/ericdemo
	strip -o $@ $<

# Automatische Abhaengigkeiten

$(DEB)/%.d: ericdemo/%.cpp $(DEB)
	$(CXX) -MM $(INC) $< | sed "s;$(notdir $*).o:;$(DEB)/$*.o $(DEB)/$*.d:;" > $@

.PHONY: clean
clean:
	rm -f $(DEB)/*.o $(DEB)/ericdemo $(REL)/ericdemo $(DEB)/*.d

-include $(SOURCE:%.cpp=$(DEB)/%.d)
