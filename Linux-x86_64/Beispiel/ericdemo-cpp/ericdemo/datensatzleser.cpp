#include "datensatzleser.h"
#include "anwendungsfehler.h"
#include <fstream>
#include "system.h" 

using std::string;
using std::ifstream;

void Datensatzleser::lese(const string& dateiName, string& xmlDatensatz)
{
    // Oeffne die Datei zum Lesen.
    ifstream datei(
#ifdef WINDOWS_MSVC
        System::kod::toUtf16(dateiName)
#else
       dateiName.c_str()
#endif
    );
    if (! datei)
    {
        throw Anwendungsfehler(string("Kann Datei nicht oeffnen: ") + dateiName);
    }

    while(datei)
    {
        string zeile;
        getline(datei, zeile);
        xmlDatensatz += zeile + "\n";
    }

    if (! datei.eof())
    {
        throw Anwendungsfehler(string("Fehler bei Lesen der Datei ") + dateiName);
    }
}
