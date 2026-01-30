#include "ericdekodierung.h"

#include "anwendungsfehler.h"
#include "datensatzleser.h"
#include "eric.h"
#include "ericpuffer.h"
#include "ericzertifikat.h"
#include "system.h"


EricDekodierung::EricDekodierung(const Eric& eric) : ericAdapter(eric)
{ }

EricDekodierung::~EricDekodierung()
{ }

void EricDekodierung::leseDatensatz(const std::string &dateiName)
{
    Datensatzleser leser;
    leser.lese(dateiName, verschluesselteDaten);
}

int EricDekodierung::ausfuehren( const System::KommandozeilenParser &argParser,
                               const EricZertifikat *zertifikat,
                               std::string &ergebnis ) const
{
    System::titelZeile("Entschluesselung der Daten aus der Datei \"" + argParser.getDatensatzDatei() + "\"");

    if (zertifikat == nullptr)
    {
        throw Anwendungsfehler("FEHLER: Zur Entschluesslung muss ein Zertifikat angegeben werden");
    }

    std::cout << "Entschluessele mit Zertifikat: " << zertifikat->getPfad() << std::endl;

    EricPuffer ergebnisPuffer(ericAdapter);

    const int rc = ericAdapter.EricDekodiereDaten(
        zertifikat->getHandle(), zertifikat->getPin(),
        verschluesselteDaten.c_str(), ergebnisPuffer.handle() );

    ergebnis.assign(ergebnisPuffer.inhalt(),ergebnisPuffer.laenge());

    return rc;
}
