#include "ericvorgang.h"

#include <memory>
#include <list>
#include <iostream>
#include <sstream>

#include "anwendungsfehler.h"
#include "datensatzleser.h"
#include "eric.h"
#include "ericpuffer.h"
#include "ericzertifikat.h"
#include "system.h"

using std::string;
using std::list;


namespace
{

/** @brief Dies sind die Standardeinstellungen des Beispiels. */
eric_druck_parameter_t holeDruckeinstellungen()
{
    eric_druck_parameter_t druckEinstellungen = {};
    druckEinstellungen.version     = 4;
    druckEinstellungen.vorschau    = 0;
    druckEinstellungen.duplexDruck = 0;
    druckEinstellungen.pdfName     = "ericprint.pdf";
    druckEinstellungen.fussText    = nullptr;
    druckEinstellungen.pdfCallback = nullptr;
    druckEinstellungen.pdfCallbackBenutzerdaten = nullptr;
    return druckEinstellungen;
}

uint32_t holeBearbeitungsFlags(const System::KommandozeilenParser &argParser)
{
    uint32_t bearbeitung = ERIC_VALIDIERE;
    if (argParser.getDatensatzSenden())
    {
        bearbeitung |= ERIC_SENDE;
        if (!argParser.getHatTransferHandle())
        { // Ein Tranferhandle ist nur für die Datenabholung relevant, die keinen Druck bietet
            bearbeitung |= ERIC_DRUCKE;
        }
    }

    return bearbeitung;
}

} // anonymous namespace


EricVorgang::EricVorgang(const Eric& eric) : ericAdapter(eric)
{ }

EricVorgang::~EricVorgang()
{ }

void EricVorgang::leseDatensatz(const std::string &dateiName)
{
    Datensatzleser leser;
    leser.lese(dateiName, xmlDaten);
}

int EricVorgang::ausfuehren( const System::KommandozeilenParser &argParser, const EricZertifikat *zertifikat,
                             std::string &ergebnis, std::string &antwort, EricTransferHandle &transferHandle ) const
{
    System::titelZeile("Lese die Datensatzdatei \"" + argParser.getDatensatzDatei() + "\" mit Datenartversion \"" + argParser.getDatenartVersion() + "\" ein");

    const uint32_t bearbeitungsFlags = holeBearbeitungsFlags(argParser);
    const bool sende =  bearbeitungsFlags & ERIC_SENDE;
    const bool drucke =  bearbeitungsFlags & ERIC_DRUCKE;

    std::string titelText = sende ? "Sende" : "Validiere";

    if (drucke)
    {
        titelText += " und drucke";
    }
    titelText += " den Datensatz";
    System::titelZeile(titelText);

    if (sende)
    {
        // Wenn ein Zertifikat angegeben wurde, wird authentifiziert versendet.
        if (zertifikat != nullptr)
        {
            std::cout << "Sende mit Zertifikat: " << zertifikat->getPfad() << std::endl;
        }
        else
        {
            std::cout << "Sende ohne Zertifikat" << std::endl;
        }
    }

    EricPuffer ergebnisPuffer(ericAdapter);
    EricPuffer serverantwortPuffer(ericAdapter);
    eric_druck_parameter_t druckEinstellungen = ::holeDruckeinstellungen();
    transferHandle = argParser.getTransferHandle();
    const eric_verschluesselungs_parameter_t *verschluesselungsParameter =
        zertifikat && sende ? &(zertifikat->getVerschlusselungsParameter()) : nullptr;

    const int rc = ericAdapter.EricBearbeiteVorgang(
        xmlDaten.c_str(), argParser.getDatenartVersion().c_str(),
        bearbeitungsFlags, &druckEinstellungen, verschluesselungsParameter,
        argParser.getHatTransferHandle() ? &transferHandle : nullptr,
        ergebnisPuffer.handle(), serverantwortPuffer.handle() );

    ergebnis.assign(ergebnisPuffer.inhalt(),ergebnisPuffer.laenge());
    if (sende)
    {
        antwort.assign(serverantwortPuffer.inhalt(),serverantwortPuffer.laenge());
    }
    else
    {
        antwort.clear();
    }

    return rc;
}
