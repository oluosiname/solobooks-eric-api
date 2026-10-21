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

// Der IBM AIX-Compiler xlC Legacy unterstützt C++11 nur unvollständig.
// Daher wird std::auto_ptr verwendet
#if defined(__xlC__) && !defined(__clang__)
#define UniquePtr std::auto_ptr
#else
template<typename T>
using UniquePtr = std::unique_ptr<T, std::default_delete<T>>;
#endif

namespace {
        
    UniquePtr<eric_druck_parameter_t> erzeugeDruckParameter(const char * const pdfPfad, const bool vorschau) {
        UniquePtr<eric_druck_parameter_t> druckParameter(new eric_druck_parameter_t());
        druckParameter->version = 4;
        druckParameter->vorschau = vorschau ? 1U : 0;
        druckParameter->pdfName = pdfPfad;
        return druckParameter;
    }

}


EricVorgang::EricVorgang(const Eric& eric) : ericAdapter(eric)
{ }


void EricVorgang::leseDatensatz(const std::string &dateiName)
{
    Datensatzleser::lese(dateiName, xmlDaten);
}

int EricVorgang::ausfuehren( const System::KommandozeilenParser &argParser, const EricZertifikat *zertifikat,
                             std::string &ergebnis, std::string &antwort) const
{
    System::titelZeile("Lese die Datensatzdatei \"" + argParser.getDatensatzDatei() + "\" mit Datenartversion \"" + argParser.getDatenartVersion() + "\" ein");

    uint32_t bearbeitungsFlags = ERIC_VALIDIERE;


    const bool drucke =  argParser.getPdfPfad() != "_NULL";
    const bool sende = argParser.getDatensatzSenden();
    
    UniquePtr<eric_druck_parameter_t> druckParameter;
    UniquePtr<EricPuffer> serverPuffer;
    const eric_verschluesselungs_parameter_t* verschluesselungsParameter = nullptr;
    
    std::string pdfPfad;

    std::string titelText = sende ? "Sende" : "Validiere";

    if (drucke)
    {
        bearbeitungsFlags |= ERIC_DRUCKE;
        titelText += " und drucke den Datensatz in die Datei \""+ argParser.getPdfPfad() +'"';

#ifdef WINDOWS_MSVC
        pdfPfad.assign(System::kod::toWindowsZeichenKodierung(argParser.getPdfPfad()));
#else
        pdfPfad.assign(argParser.getPdfPfad());
#endif
        druckParameter = erzeugeDruckParameter(pdfPfad.c_str(), !sende);
    } else {
        titelText += " den Datensatz";
    }
   
    System::titelZeile(titelText);

    if (sende)
    {
        bearbeitungsFlags |= ERIC_SENDE;
        // Wenn ein Zertifikat angegeben wurde, wird authentifiziert versendet.
        if (zertifikat != nullptr)
        {
            std::cout << "Sende mit Zertifikat: " << zertifikat->getPfad() << std::endl;
        }
        else
        {
            std::cout << "Sende ohne Zertifikat" << std::endl;
        }
        serverPuffer.reset(new EricPuffer(ericAdapter));
        verschluesselungsParameter = zertifikat ? &zertifikat->getVerschlusselungsParameter(): nullptr;
    }

    EricPuffer ergebnisPuffer(ericAdapter);
   

    const int rc = ericAdapter.EricBearbeiteVorgang(
        xmlDaten.c_str(),
        argParser.getDatenartVersion().c_str(),
        bearbeitungsFlags,
        druckParameter.get(),
        verschluesselungsParameter,
        ergebnisPuffer.handle(),
        serverPuffer.get() ? serverPuffer->handle(): nullptr);

    ergebnis.assign(ergebnisPuffer.inhalt(),ergebnisPuffer.laenge());
    if (serverPuffer.get() && serverPuffer->inhalt())
    {
        antwort.assign(serverPuffer->inhalt(),serverPuffer->laenge());
    }
    else
    {
        antwort.clear();
    }

    return rc;
}
