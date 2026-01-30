#include <cstdio>
#include <exception>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <utility>
#include <string.h>
#include <vector>

#include "ericsystemsteuerung.h"
#include "ericdekodierung.h"
#include "ericvorgang.h"
#include "ericzertifikat.h"
#include "system.h"
#include <ericapi.h>
#include <eric_fehlercodes.h>
#include "eric.h"
#include "ericpuffer.h"
#include "callbackhandler.h"


namespace
{
// Wartet, bis der Anwender die Eingabetaste drückt
void warteAufEingabe()
{
    std::cout << std::endl << "Bitte drücken Sie die Eingabetaste" << std::endl;
    std::cin.ignore();
}

// Gibt bei binären Daten eine Angabe zum Binärformat aus, sonst einen Leerstring
std::string angabeFormat(const std::string& daten) {
    if (daten.size() > 3) {
        if (daten.substr(0, 4) == "\x25\x50\x44\x46") {
            return "[PDF-Dokument]";
        }
        if (daten.substr(0, 4) == "\x50\x4B\x03\x04") {
            return "[Office-Dokument / ZIP-Archiv]";
        }
        if (daten.substr(0, 2) == "\xFF\xD8") {
            return "[Jpeg-Bild]";
        }
    }
    // Datenanfang auf Null-Bytes durchsuchen
    size_t anzahlZeichenPruefen = daten.size() > 3000 ? 3000 : daten.size();
    for (size_t i = 0; i < anzahlZeichenPruefen; ++i)
    {
        if (daten[i] == 0)
            return "[Binäre Daten]";
    }
    return "";
}
}

/** @brief Gib den Antwort-Text einer Eric-Funktion aus. */
static void protokolliere(const System::KommandozeilenParser &argParser, int fehlerkode, const std::string& ergebnis, std::string& antwort, const EricTransferHandle &transferHandle, const Eric& eric)
{
    std::string fehlerText;
    if (fehlerkode != ERIC_OK)
    {   // Genaue Fehlerbeschreibung herausfinden
        EricPuffer antwortPuffer(eric);
        if (ERIC_OK == eric.EricHoleFehlerText(fehlerkode, antwortPuffer.handle()))
        {
            fehlerText = antwortPuffer.inhalt();
        }
        else
        {
            fehlerText = "<Kein Fehlertext verfügbar>";
        }
    }
    else
    {
        fehlerText = "Verarbeitung fehlerfrei.";
    }

    std::string statusText("Validierungstatus");
    if (argParser.getDatensatzSenden())
    {
        statusText = "Sendestatus";
    }
    else if (argParser.getDatenEntschluesseln())
    {
        statusText = "Entschlüsselungsstatus";
    }
    std::cout << std::endl << statusText << ": " << fehlerText << std::endl;

    if (argParser.getHatTransferHandle() && argParser.getDatensatzSenden())
    {
        std::cout << std::endl << "Transferhandle: " << transferHandle << std::endl;
    }

    std::cout << std::endl << "Rückgabe:" << std::endl;
    if (argParser.getDatenEntschluesseln()) {
        const std::string datenFormat = angabeFormat(ergebnis);
        std::cout << (datenFormat.empty() ? ergebnis : datenFormat) << std::endl;
    }
    else {
        std::cout << ergebnis <<  std::endl;
    }
    if (!antwort.empty())
    {
        std::cout << std::endl << "Serverantwort:" << std::endl;
        std::cout << antwort << std::endl;
    }

    // Auf Wunsch schreiben wir das Ergebnis bzw. die Serverantwort in eine Datei
    if (!argParser.getAusgabeDatei().empty())
    {
        if (System::schreibeDatei(antwort.empty() ? ergebnis : antwort, argParser.getAusgabeDatei()))
        {
            std::cout<< std::endl << (antwort.empty() ? "Das Ergebnis" : "Die Serverantwort") << " wurde auch in die Datei \"" << argParser.getAusgabeDatei() << "\" geschrieben." << std::endl;
        }
        else
        {
            std::cout << std::endl << "FEHLER: Die Datei \"" << argParser.getAusgabeDatei() << "\" konnte nicht geschrieben werden." << std::endl;
        }
    }
}

#ifdef WINDOWS_MSVC
int wmain(int argc, wchar_t* argv[]){
    // Setze das Windows-Console-Encoding auf UTF-8
    const System::kod::KonsolenKodierungsManager kodierungsManager;
#else
int main(int argc, char* argv[]) {
#endif

    std::vector<std::string> argumente(argc);
    for (size_t i = 0; i < static_cast<size_t>(argc) && argv[i] != nullptr; ++i) {

        argumente[i].assign(
#ifdef WINDOWS_MSVC
            System::kod::toUtf8(argv[i])
#else
            argv[i]
#endif
        );
    }    

    System::KommandozeilenParser argParser;

    try
    {
        argParser.parseKommandozeile(MOVE_NO_XLC(argumente));
           
    }
    catch (const std::exception &stdException)
    {
        std::cerr << stdException.what() << std::endl;
    }


    if (argParser.getHilfeAnzeigen())
    {    // Kurze Hilfe ausgeben, falls Hilfe gewuenscht wird
        argParser.zeigeHilfe(std::cout);
        warteAufEingabe();
        return EXIT_SUCCESS;
    }
    if (!argParser.getParseOk())
    {    // Kurze Hilfe ausgeben, falls die Parameter nicht stimmen
        argParser.zeigeHilfe(std::cerr);
        warteAufEingabe();
        return EXIT_FAILURE;
    }

    int fehlerkode = ERIC_GLOBAL_UNKNOWN;

    try
    {
        Eric eric(argParser.getHomeDir(), argParser.getLogDir());

        // Callbacks anmelden, diese werden im Dekonstruktor des Objekts wieder abgemeldet
        CallbackHandler callbackHandler(eric);

        // Zertifikateigenschaften ausgeben

#if defined(__xlC__) && !defined(__clang__) // Der IBM AIX-Compiler xlC Legacy unterstützt unique_ptr nicht
        System::FreePtr<EricZertifikat> zertifikat;
#else
        std::unique_ptr<EricZertifikat> zertifikat;
#endif
        if (argParser.getZertifikatPfad().compare("_NULL") != 0)
        {
            zertifikat.reset(new EricZertifikat(eric,argParser.getZertifikatPfad(),argParser.getZertifikatPin()));
            System::titelZeile("Zertifikatseigenschaften von \"" + argParser.getZertifikatPfad() + "\"");
            std::cout << zertifikat->getEigenschaften() << std::endl;
        }

        std::string ergebnis, antwort;
        EricTransferHandle transferHandle = 0;

        // Entweder Daten dekodieren ...
        if (argParser.getDatenEntschluesseln())
        {
            EricDekodierung dekodierung(eric);
            dekodierung.leseDatensatz(argParser.getDatensatzDatei());
            fehlerkode = dekodierung.ausfuehren(argParser, zertifikat.get(),ergebnis);
        } else

        // ... oder den Datensatz validieren und falls gewünscht versenden
        if (!argParser.getDatenartVersion().empty())
        {
            EricVorgang vorgang(eric);
            vorgang.leseDatensatz(argParser.getDatensatzDatei());
            fehlerkode = vorgang.ausfuehren(argParser,zertifikat.get(),ergebnis,antwort,transferHandle);
        }

        ::protokolliere(argParser,fehlerkode,ergebnis,antwort,transferHandle,eric);
    }
    catch(const std::exception& stdException)
    {
        std::cerr<< "Fehler: " << stdException.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "Unbekannter Fehler" << std::endl;
    }

    warteAufEingabe();
    return fehlerkode == ERIC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
