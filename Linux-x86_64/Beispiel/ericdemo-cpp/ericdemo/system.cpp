#include <cstdio>
#include <fstream>
#include <iostream>
#include <platform.h>
#include <stdlib.h>
#include <string>
#include <locale>
#include <iterator>
#include "anwendungsfehler.h"
#include "system.h"

#ifdef _WIN32
#   include <direct.h>
#   include <shlwapi.h>


namespace {
       bool istPfadseparator(const char c) { return (PFAD_SEPARATOR==c) || ('/'==c); }
}
#else
extern char **environ;
namespace {
    bool istPfadseparator(const char c) { return (PFAD_SEPARATOR==c); }
}
#endif

namespace System {

const std::string KommandozeilenParser::defaultZertifikatPfad("test-softidnr-pse.pfx");
const std::string KommandozeilenParser::defaultZertifikatPin("123456");
const std::string KommandozeilenParser::defaultDatensatzDatei("ESt_2020.xml");
const std::string KommandozeilenParser::defaultDatenartVersion("ESt_2020");

KommandozeilenParser::KommandozeilenParser() :
    parseOk(true),
    aufrufPfad(),
    logDir(),
    homeDir(),
    zertifikatPfad(),
    zertifikatPin(),
    datensatzDatei(),
    datenartVersion(),
    datensatzSenden(true),
    hilfeAnzeigen(false),
    datenEntschluesseln(false),
    ausgabeDatei(),
    transferHandle(0),
    hatTransferHandle(false)
{ }

#if defined(__xlC__) && !defined(__clang__)
void KommandozeilenParser::parseKommandozeile( const std::vector<std::string> &argumente)
#else
void KommandozeilenParser::parseKommandozeile( std::vector<std::string> &&argumente)
#endif
{
    char letzteOption = 0;

    for (auto iter(argumente.begin()); iter != argumente.end(); ++iter) {
        if (aufrufPfad.empty()) {
            aufrufPfad.assign(MOVE_NO_XLC(*iter));
            continue;
        }

        if (OPT_PRAEFIX == iter->at(0)) {
            if (0 == letzteOption) {
                letzteOption = std::tolower(iter->at(1), std::locale(""));
                switch (letzteOption) {
                case 'h':
                case '?':
                    hilfeAnzeigen = true;
                    letzteOption = 0; // Hilfeoptionen erwarten keine
                                      // nachfolgenden Parameter
                    break;
                case 'n':
                    datensatzSenden = false;
                    letzteOption = 0;
                    break;
                case 'l': // Protokollverzeichnis (log_dir)
                case 'd': // Heimverzeichnis (home_dir)
                case 'c': // Pfad zum Zertifikat
                case 'p': // PIN
                case 'v': // Datenartversion
                case 'x': // Datensatz.xml
                case 's': // Rueckgabe speichern
                case 't': // Transferhandle
                    // Optionen, die einen nachfolgenden Parameter erwarten
                    // Fuer solche Optionen ist hier noch nichts zu tun
                    break;
                case 'e':
                    datenEntschluesseln = true;
                    letzteOption = 0;
                    break;

                default:
                    parseOk = false;
                    throw Anwendungsfehler(std::string("Unbekannte Option ") + *iter);
                    break;
                }
            } else {
                // Zwei aufeinanderfolgende Optionsflags bedeuten, dass ein Optionsparameter fehlt
                parseOk = false;
                throw Anwendungsfehler(std::string("Fehlender Parameter fuer Option ") + *PREVIOUS(iter));
            }
        } else {
            switch (letzteOption) {
            case 'l': // Protokollverzeichnis (log_dir)
                logDir.assign(MOVE_NO_XLC(*iter));
                break;
            case 'd': // Heimverzeichnis (home_dir)
                homeDir.assign(MOVE_NO_XLC(*iter));
                break;
            case 'c': // Pfad zum Zertifikat
                zertifikatPfad.assign(MOVE_NO_XLC(*iter));
                break;
            case 'p': // PIN
                zertifikatPin.assign(MOVE_NO_XLC(*iter));
                break;
            case 's': // In Dateiname speichern
                ausgabeDatei.assign(MOVE_NO_XLC(*iter));
                break;
            case 't': // Transferhandle
                try {
#if defined(__xlC__) && !defined(__clang__)
                    transferHandle = System::toUlong(*iter);
#else
                    transferHandle = static_cast<EricTransferHandle>(std::stoul(*iter));
#endif
                    hatTransferHandle = true;
                } catch (const std::invalid_argument &) {
                    parseOk = false;
                    throw Anwendungsfehler(std::string("Ungueltiger Parameter fuer Option ") + *PREVIOUS(iter));
                }
                break;
            case 'v': // Datenartversion
                datenartVersion.assign(MOVE_NO_XLC(*iter));
                break;
            case 'x': // Datensatzdatei
                datensatzDatei.assign(MOVE_NO_XLC(*iter));
                break;
            case 0:
                parseOk = false;
                throw Anwendungsfehler(std::string("Wert \"") + *iter + std::string("\" angegeben, aber Optionsparameter fehlt "));
                break;

            default:
                parseOk = false;
                throw Anwendungsfehler(std::string("Parameter fuer unbekannte Option ") + *PREVIOUS(iter));
                break;
            }
            letzteOption = 0;
        }
    }

    if (datenEntschluesseln && !datenartVersion.empty()) {
        parseOk = false;
        throw Anwendungsfehler(std::string("Die Optionen ") + OPT_PRAEFIX + 'v' + " und " + OPT_PRAEFIX + "e schliessen sich gegenseitig aus.");
    }
}

void KommandozeilenParser::zeigeHilfe(std::ostream& ostream) {
    std::string aufrufPfad = "ericdemo";
    ostream << NEW_LINE
        << "Optionen:" << NEW_LINE
        << "    " << OPT_PRAEFIX << 'h'
        << "                   Diese Hilfe ausgeben" << NEW_LINE
        << "    " << OPT_PRAEFIX << 'v' << " <datenartversion>"
        << " Datenartversion" << NEW_LINE
        << "    " << OPT_PRAEFIX << 'x' << " <xml>"
        << "             Pfad zur Datensatzdatei" << NEW_LINE
        << "    " << OPT_PRAEFIX << 'c' << " <certificate>"
        << "     Pfad zu einem Benutzerzertifikat, _NULL (Nullzeiger) fuer kein Benutzerzertifikat" << NEW_LINE
        << "    " << OPT_PRAEFIX << 'p' << " <pin>"
        << "             PIN fuer das Benutzerzertifikat, _NULL (Nullzeiger) fuer keine PIN" << NEW_LINE
        << "    " << OPT_PRAEFIX << 'd' << " <dir>"
        << "             Pfad zum Verzeichnis mit den ERiC-Bibliotheken" << NEW_LINE
        << "    " << OPT_PRAEFIX << 'l' << " <log>"
        << "             Pfad zum Verzeichnis, in dem die ERiC-Protokolldateien geschrieben werden" << NEW_LINE
        << "    " << OPT_PRAEFIX << 's' << " <dateipfad>"
        << "       Schreibt die Serverantwort oder - wenn nicht vorhanden - das Ergebnis in die angegebene Datei" << NEW_LINE
        << "    " << OPT_PRAEFIX << 't' << " <transferhandle>"
        << "  Transferhandle, das an die Server uebermittelt wird (nur bei Datenabholungen anzugeben)" << NEW_LINE
        << "    " << OPT_PRAEFIX << 'n'
        << "                   Der Datensatz soll nicht versendet, sondern nur validiert werden" << NEW_LINE
        << "    " << OPT_PRAEFIX << 'e'
        << "                   Der Datensatz soll nicht validiert oder versendet, sondern entschluesselt werden" << NEW_LINE
        << NEW_LINE
        << "Standardwerte:" << NEW_LINE
        << "    <datenartversion>: ESt_2020" << NEW_LINE
        << "    <xml>:             ESt_2020.xml" << NEW_LINE
        << "    <certificate>:     test-softidnr-pse.pfx" << NEW_LINE
        << "    <pin>:             123456" << NEW_LINE
#ifdef _WIN32
        << "    <dir>:             <Arbeitsverzeichnis>" << PFAD_SEPARATOR << ".." << PFAD_SEPARATOR << ".." << PFAD_SEPARATOR << "dll" << NEW_LINE
#else
        << "    <dir>:             <Arbeitsverzeichnis>" << PFAD_SEPARATOR << ".." << PFAD_SEPARATOR << ".." << PFAD_SEPARATOR << "lib" << NEW_LINE
#endif
        << "    <log>:             <Arbeitsverzeichnis>" << NEW_LINE
        << NEW_LINE
        << "Beispiele:" << NEW_LINE
        << "    " << aufrufPfad << NEW_LINE
        << "    " << aufrufPfad << " " << OPT_PRAEFIX << "?" << NEW_LINE
        << "    " << aufrufPfad << " " << OPT_PRAEFIX << "v ESt_2020 " << OPT_PRAEFIX << "x ESt_2020.xml " << OPT_PRAEFIX << "c test-softidnr-pse.pfx " << OPT_PRAEFIX << "p 123456" << NEW_LINE
        << "    " << aufrufPfad << " " << OPT_PRAEFIX << "v ESt_2016 " << OPT_PRAEFIX << "x ESt_2016.xml " << OPT_PRAEFIX << "c _NULL " << OPT_PRAEFIX << "p _NULL" << NEW_LINE
        << "    " << aufrufPfad << " " << OPT_PRAEFIX << "v ESt_2020 " << OPT_PRAEFIX << "x ESt_2020.xml "
                                       << OPT_PRAEFIX << "n" << NEW_LINE
        << "    " << aufrufPfad << " " << OPT_PRAEFIX << "v ESt_2020 " << OPT_PRAEFIX << "x ESt_2020.xml " << NEW_LINE
        << "    " << aufrufPfad << " " << OPT_PRAEFIX << "v Kontoinformation " << OPT_PRAEFIX << "x kontoinformation.xml "
        << OPT_PRAEFIX << "c \"http://127.0.0.1:24727/eID-Client?testmerker=520000000\" " << OPT_PRAEFIX << "p _NULL" << NEW_LINE
        << "    " << aufrufPfad << " " << OPT_PRAEFIX << "v MitteilungAbholung " << OPT_PRAEFIX << "x MitteilungAbholungAnfrage.xml "
        << OPT_PRAEFIX << "c test-softidnr-pse.pfx " << OPT_PRAEFIX << "p 123456 " << OPT_PRAEFIX << "t 0" << NEW_LINE
        << "    " << aufrufPfad << " " << OPT_PRAEFIX << "e " << OPT_PRAEFIX << "x Abholdaten.b64 " << OPT_PRAEFIX << "s Abholdaten.xml" << std::endl;
}



std::string getBibliotheksDateiname(const std::string &bibliotheksName)
{
    return std::string(DYNBIB_PRAEFIX) + bibliotheksName + DYNBIB_SUFFIX;
}

#ifdef _WIN32
bool istPfadRelativ(const char *pfad)
{
    return nullptr != pfad ? TRUE == ::PathIsRelativeA(pfad) : false;
}
#else

bool istPfadRelativ(const char *pfad)
{
    return nullptr != pfad ? PFAD_SEPARATOR != pfad[0] : false;
}
#endif

bool getArbeitsverzeichnis(std::string &arbeitsVerzeichnis)
{
    const size_t pufferLaenge = 8184;
    char pfadPuffer[pufferLaenge];

    // Hole das aktuelle Arbeitsverzeichnis
    if (GETCWD(pfadPuffer, pufferLaenge) == nullptr)
    {
        std::cerr << "Fehler beim Ermitteln des aktuellen Arbeitsverzeichnisses (" << GETLASTERROR() << ")" << std::endl;
        return false;
    }

    arbeitsVerzeichnis = pfadPuffer;
    arbeitsVerzeichnis += PFAD_SEPARATOR;

    return true;
}


std::string dateiPfad(const std::string &verzeichnisPfad, const std::string &dateiName)
{
    std::string pfad = verzeichnisPfad;

    // Falls der Pfad nicht mit einem Pfadseparator abschliesst, wird dieser angefuegt.
    if (!pfad.empty() && !istPfadseparator(*pfad.rbegin()))
        pfad += PFAD_SEPARATOR;

    pfad.append(dateiName);

    return pfad;
}

bool schreibeDatei(const std::string& daten, const std::string& dateiName)
{
    bool ausgabeOkay = false;
    std::ofstream ausgabeDatei(
#ifdef WINDOWS_MSVC
        System::kod::toUtf16(dateiName)
#else 
        dateiName.c_str()
#endif
        ,std::ios_base::binary);
    if (ausgabeDatei.is_open())
    {
        ausgabeDatei.write(daten.data(),daten.size());
        ausgabeOkay = !ausgabeDatei.fail();
    }
    if (!ausgabeOkay)
    {
        std::cerr << "Fehler beim Schreiben in die Datei \"" << dateiName << "\"" << std::endl;
    }
    return ausgabeOkay;
}


#ifdef WINDOWS_MSVC

namespace kod {
    std::string toUtf8(const std::wstring& wStr) {
        if (wStr.empty()) {
            return std::string{};
        }
        const int groesse = WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (groesse == 0) {
            throw std::runtime_error("Fehler beim Bestimmen der Puffergröße! " +
                std::to_string(GetLastError()));
        }

        std::string utf8Str(groesse - 1, '\0');
        const int rc = WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), -1, &utf8Str[0], groesse, nullptr, nullptr);
        if (rc == 0) {
            throw std::runtime_error("Kann UTF-16-Wide-String nicht in UTF-8-String konvertieren! " + std::to_string(GetLastError()));
        }
        return utf8Str;
    }

    std::wstring toUtf16(const std::string& str) {
        if (str.empty()) {
            return std::wstring{};
        }

        const int groesse = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
        if (groesse == 0) {
            throw std::runtime_error("Fehler beim Bestimmen der Puffergröße! " +
                std::to_string(GetLastError()));
        }
        std::wstring utf16Str(groesse - 1, '\0');
        const int rc = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &utf16Str[0], groesse);
        if (rc == 0) {
            throw std::runtime_error("Kann String `" + str + "` nicht in UTF-16-Wide-String konvertieren! " + std::to_string(GetLastError()));
        }
        return utf16Str;
    }

    std::string toWindowsZeichenKodierung(const std::string& str) {
        if (str.empty()) {
            return std::string{};
        }

        static const unsigned int windowsZeichenKodierung = GetACP();
        if (CP_UTF8 == windowsZeichenKodierung) {
            return str;
        }

        const std::wstring wStr{ toUtf16(str) };
        const int groesse = WideCharToMultiByte(windowsZeichenKodierung, 0, wStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (groesse == 0) {
            throw std::runtime_error("Fehler beim Bestimmen der Puffergröße! " +
                std::to_string(GetLastError()));
        }

        std::string windowsZeichenKodierungString(groesse - 1, '\0');
        const int rc = WideCharToMultiByte(windowsZeichenKodierung, 0, wStr.c_str(), -1, &windowsZeichenKodierungString[0], groesse, nullptr, nullptr);
        if (rc == 0) {
            throw std::runtime_error("Kann String `" + str + "` nicht in ACP-String konvertieren! " + std::to_string(GetLastError()));
        }
        return windowsZeichenKodierungString;
    }

   KonsolenKodierungsManager::KonsolenKodierungsManager() : zeichenKodierung(GetConsoleCP()), ausgabeZeichenKodierung(GetConsoleOutputCP()) {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }

    KonsolenKodierungsManager::~KonsolenKodierungsManager() {
        SetConsoleOutputCP(ausgabeZeichenKodierung);
        SetConsoleCP(zeichenKodierung);
    }
}
#endif

#if defined(__xlC__) && !defined(__clang__)
unsigned long toUlong(const std::string &s)
{
    std::istringstream stringStream;
    stringStream.exceptions(std::ios::badbit | std::ios::failbit);
    stringStream.imbue(std::locale::classic());

    unsigned long value = 0;
    stringStream >> value;
    return value;
}
#endif

void titelZeile(const std::string& titel)
{
    std::cout << NEW_LINE << "*** " << titel << " ***" << NEW_LINE << std::endl;
}

}
