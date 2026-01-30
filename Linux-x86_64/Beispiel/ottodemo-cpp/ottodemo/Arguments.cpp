#include "Arguments.h"

#include <iostream>
#include <stdexcept>

using namespace std::string_literals;

namespace {

#ifdef _WIN32
const char optionPrefix = '/';
#   define GETCWD(x, y) _getcwd(x, y)
#else
const char optionPrefix = '-';
#   define GETCWD(x, y) getcwd(x, y)
#endif

std::string getDemoDir() {
    const size_t bufferSize = 8184u;
    char pathBuffer[bufferSize];

    // Hole das aktuelle Arbeitsverzeichnis
    if (nullptr == GETCWD(pathBuffer,bufferSize))
        std::cerr << "Warnung: Fehler beim Ermitteln des aktuellen Arbeitsverzeichnisses (" << GETLASTERROR() << ")" << std::endl;

    std::string demoDir = pathBuffer;
    if (PATH_SEPARATOR != *demoDir.rbegin())
        demoDir += PATH_SEPARATOR;

    return demoDir;
}

std::string askForHerstellerId() {
    std::cout << "Bitte geben Sie Ihre Hersteller-ID ein: " << std::flush;
    std::string herstellerId;
    std::getline(std::cin, herstellerId);
    std::cout << std::endl;
    return herstellerId;
}

} // anonymous namespace


Arguments::Arguments() : appPath("ottodemo"),
                         demoDirPath(getDemoDir()),
                         ottoDirPath(demoDirPath + ".." + PATH_SEPARATOR + ".." + PATH_SEPARATOR + DYLIB_DIR + PATH_SEPARATOR),
                         logDirPath(demoDirPath),
                         inFilePath(),
                         objectId("7090bc69-be5e-4fd0-b91a-2128021295d6"),
                         outFilePath("beispiel.xml"),
                         certPath(demoDirPath + "test-softidnr-pse.pfx"),
                         certPin("123456"),
                         herstellerId(),
                         showHelp(false),
                         sendData(false),
                         fetchData(false),
                         parseOk(true)
{}

void Arguments::help() const {
    std::cout << std::endl << "Optionen:" << std::endl;
    std::cout << "\t" << optionPrefix << 'h' << "                      Zeigt diese Hilfe an" << std::endl;
    std::cout << "\t" << optionPrefix << '?' << "                      Zeigt diese Hilfe an" << std::endl;
    std::cout << "\t" << optionPrefix << 'd' << " <Otto-Verzeichnis>   Pfad des Verzeichnisses mit den ERiC- bzw. Otto-Bibliotheken" << std::endl;
    std::cout << "\t" << optionPrefix << 'l' << " <Log-Verzeichnis>    Pfad des Verzeichnisses, in dem die Protokolldatei otto.log erstellt werden soll" << std::endl;
    std::cout << "\t" << optionPrefix << 'o' << " <Objekt-ID>          ID der Daten, die abgeholt werden sollen" << std::endl;
    std::cout << "\t" << optionPrefix << 'e' << " <Pfad Zieldatei>     Holt Daten von OTTER ab und schreibt sie in die angegebene Datei" << std::endl;
    std::cout << "\t" << optionPrefix << 's' << " <Pfad Quelldatei>    Sendet den Inhalt der angegebenen Datei an OTTER" << std::endl;
    std::cout << "\t" << optionPrefix << 'c' << " <Pfad Zertifikat>    Pfad des Sicherheitstokens fuer die Authentifizierung" << std::endl;
    std::cout << "\t" << optionPrefix << 'p' << " <Passwort>           Passwort oder PIN des Sicherheitstokens" << std::endl;
    std::cout << "\t" << optionPrefix << 'i' << " <Hersteller-ID>      Individuelle ID des Softwareherstellers" << std::endl;

    Arguments tmpArgs;
    tmpArgs.herstellerId = "kein Standardwert, Angabe ist zwingend erforderlich";
    std::cout << std::endl << "Standardwerte:" << std::endl;
    std::cout << "\t" << optionPrefix << 'd' << " <Otto-Verzeichnis>   " << tmpArgs.ottoDirPath << std::endl;
    std::cout << "\t" << optionPrefix << 'l' << " <Log-Verzeichnis>    " << tmpArgs.logDirPath << std::endl;
    std::cout << "\t" << optionPrefix << 'o' << " <Objekt-ID>          " << tmpArgs.objectId << std::endl;
    std::cout << "\t" << optionPrefix << 'e' << " <Pfad Zieldatei>     " << tmpArgs.outFilePath << std::endl;
    std::cout << "\t" << optionPrefix << 'c' << " <Pfad Zertifikat>    " << tmpArgs.certPath << std::endl;
    std::cout << "\t" << optionPrefix << 'p' << " <Passwort>           " << tmpArgs.certPin << std::endl;
    std::cout << "\t" << optionPrefix << 'i' << " <Hersteller-ID>      " << tmpArgs.herstellerId << std::endl;

    std::cout << std::endl << "Beispiele:" << std::endl;
    std::cout << "\tottodemo " << optionPrefix << "h" << std::endl;
    std::cout << "\tottodemo " << optionPrefix << "e Download.file " << optionPrefix << "o 7090bc69-be5e-4fd0-b91a-2128021295d6 " << optionPrefix << "i <Hersteller-ID>" << std::endl;
    std::cout << "\tottodemo " << optionPrefix << "s Upload.file " << optionPrefix << "i <Hersteller-ID> " << optionPrefix << "c test-softidnr-pse.pfx " << optionPrefix << "p 123456" << std::endl;
    std::cout << "\tottodemo " << optionPrefix << "c \"http://127.0.0.1:24727/eID-Client?testmerker=520000000\" " << std::endl;
    std::cout << std::endl << "\tDer in den Beispielen angegebene Platzhalter \"<Hersteller-ID>\" muss durch die herstellereigene ID ersetzt werden." << std::endl;
}

void Arguments::print() const {
    std::cout << std::endl << "Standardwerte:" << std::endl;
    std::cout << "\tdemoDirPath:           \"" << demoDirPath << "\"" <<  std::endl;
    std::cout << "\tottoDirPath:           \"" << ottoDirPath << "\"" <<  std::endl;
    std::cout << "\tlogDirPath:            \"" << logDirPath << "\"" <<  std::endl;
    std::cout << "\tinFilePath:            \"" << inFilePath << "\"" <<  std::endl;
    std::cout << "\tobjectId:              \"" << objectId << "\"" <<  std::endl;
    std::cout << "\toutFilePath:           \"" << outFilePath << "\"" <<  std::endl;
    std::cout << "\tcertPath:              \"" << certPath << "\"" <<  std::endl;
    std::cout << "\tcertPin:               \"" << certPin << "\"" <<  std::endl;
    std::cout << "\therstellerId:          \"" << herstellerId << "\"" <<  std::endl;
    std::cout << "\tshowHelp:              "   << (showHelp ? "true" : "false") << std::endl;
    std::cout << "\tsendData:              "   << (sendData ? "true" : "false") << std::endl;
    std::cout << "\tfetchData:             "   << (fetchData ? "true" : "false") << std::endl;
    std::cout << "\tparseOk:               "   << (parseOk ? "true" : "false") << std::endl;
}

const Arguments parseArguments(int argc, char *argv[]) {
    Arguments arguments;

    bool optionSetO = false; // Gibt an, ob die Option -o explizit gesetzt wurde
    bool optionSetE = false; // Gibt an, ob die Option -e explizit gesetzt wurde
    bool optionSetS = false; // Gibt an, ob die Option -s explizit gesetzt wurde
    bool optionSetH = false; // Gibt an, ob die Option -h oder -? explizit gesetzt wurde

    char previousOption = 0;
    int argumentIndex = 0;

    if ((0 < argc) && (nullptr != argv[0]))
        arguments.appPath = argv[0];

    try {
        for (argumentIndex = 1; argumentIndex < argc; ++argumentIndex) {
            if (nullptr != argv[argumentIndex]) {
                if (optionPrefix == argv[argumentIndex][0]) {
                    if (0 == previousOption) {
                        previousOption = static_cast<char>(::tolower(argv[argumentIndex][1]));
                        switch(previousOption) {
                            case 'h':
                            case '?':
                                optionSetH = true;
                                arguments.showHelp = true;
                                previousOption = 0; // Hilfeoptionen erwarten keine nachfolgenden Parameter
                                break;
                            case 'l': // Protokollverzeichnis (log_dir)
                            case 'd': // Pfad zum ERiC/Otto-Bibliotheksverzeichnis
                            case 'i': // Hersteller-ID
                            case 'c': // Pfad zum Zertifikat
                            case 'p': // PIN
                                // Optionen, die einen nachfolgenden Parameter erwarten
                                // Fuer solche Optionen ist hier noch nichts zu tun
                                break;
                            case 'o': // Objekt-ID des Datenobjekts, das von OTTER heruntergeladen werden soll
                                optionSetO = true;
                                break;
                            case 'e': // Herunterladen und speichern von Daten in die angegebene Datei
                                optionSetE = true;
                                arguments.fetchData = true;
                                break;
                            case 's':  // Lesen von Daten aus der angegebenen Datei und hochladen
                                optionSetS = true;
                                arguments.sendData = true;
                                break;
                            default:
                                throw std::invalid_argument("Unbekannte Option "s + optionPrefix + argv[argumentIndex]);
                                break;
                        }
                    } else {   // Zwei aufeinanderfolgende Optionsflags bedeuten, dass ein Optionsparameter fehlt
                        throw std::invalid_argument("Fehlender Parameter fuer Option "s + optionPrefix + argv[argumentIndex-1]);
                    }
                } else {
                    switch (previousOption) {
                        case 'd': // Heimverzeichnis
                            arguments.ottoDirPath = argv[argumentIndex];
                            break;
                        case 'l': // Protokollverzeichnis (log_dir)
                            arguments.logDirPath = argv[argumentIndex];
                            break;
                        case 'i': // Hersteller-ID
                            arguments.herstellerId = argv[argumentIndex];
                            break;
                        case 'o': // Objekt-ID
                            arguments.objectId = argv[argumentIndex];
                            break;
                        case 'c': // Pfad zum Zertifikat
                            arguments.certPath = argv[argumentIndex];
                            break;
                        case 'p': // PIN
                            arguments.certPin = argv[argumentIndex];
                            break;
                        case 's': // In Dateiname speichern
                            arguments.inFilePath = argv[argumentIndex];
                            break;
                        case 'e': // Daten abholen
                            arguments.outFilePath = argv[argumentIndex];
                            break;
                        case 0:
                            throw std::invalid_argument("Wert \""s + argv[argumentIndex] + "\" angegeben, aber Optionsparameter fehlt ");
                            break;
                        default:
                            throw std::invalid_argument("Parameter fuer unbekannte Option "s + optionPrefix + argv[argumentIndex-1]);
                            break;
                    }
                    previousOption = 0;
                }
            }
        }

        if (0 != previousOption)
            throw std::invalid_argument("Fehlender Parameter fuer Option "s + optionPrefix + previousOption);
        if (!arguments.sendData) {
            arguments.fetchData = true; // Wenn nicht explizit versendet werden soll, wird standardmäßig abgeholt
            optionSetE = true;
        }
    } catch (const std::invalid_argument &invalidArgument) {
        std::cerr << "Parameterfehler: " << invalidArgument.what() << std::endl;
        arguments.parseOk = false;
    }

    if (arguments.parseOk) {
        // Bestimmte Parameterkombinationen sind nicht möglich
        arguments.parseOk = false;
        if (optionSetH && optionSetE)
            std::cerr << "Die Optionen " << optionPrefix << "e und " << optionPrefix << "h oder " << optionPrefix << "? schliessen sich gegenseitig aus " << std::endl;
        else if(optionSetH && optionSetS)
            std::cerr << "Die Optionen " << optionPrefix << "s und " << optionPrefix << "h oder " << optionPrefix << "?  schliessen sich gegenseitig aus " << std::endl;
        else if(optionSetS && optionSetE)
            std::cerr << "Die Optionen " << optionPrefix << "e und " << optionPrefix << "s schliessen sich gegenseitig aus " << std::endl;
        else if(!optionSetE && optionSetO)
            std::cerr << "Die Angabe einer Objekt-ID ist nur in Verbindung mit einer Datenabholung (Option " << optionPrefix << "e) moeglich" << std::endl;
        else // Gültige Parameterkombination
            arguments.parseOk = true;
    }

    if (arguments.parseOk && !arguments.showHelp) {
        // Für die Hersteller-ID ist kein Default möglich, da sie individuell ist. Folglich muss sie zwingend vom Anwender angegeben werden.
        // Falls über die Kommandozeile keine Hersteller-ID übergeben wurde, wird sie hier interaktiv abgefragt.
        // So ist es möglich, die Demo direkt aus einem Dateimanager wie dem Windows Explorer per Doppelklick auszuführen.
        if (arguments.herstellerId.empty()) {
            arguments.herstellerId = askForHerstellerId();
            if (arguments.herstellerId.empty()) {
                std::cerr << "Es wurde keine Hersteller-ID angegeben. Diese Angabe ist zwingend erforderlich." << std::endl;
                arguments.parseOk = false;
            }
        }
    }

    return arguments;
}

const char* Arguments::getPin() const {
    return certPin == "_NULL" ? nullptr : certPin.c_str();
}

const char* Arguments::getCertPath() const {
    return certPath == "_NULL" ? nullptr : certPath.c_str();
}
