#include "Arguments.h"
#include "OttoWrapper.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <string>
#include <utility>


namespace {

// Warten bis der Anwender die Eingabetaste drückt
int waitForEnter(const int returnValue) {
    std::cout << std::endl << "Bitte druecken Sie die Eingabetaste" << std::endl;
    std::cin.ignore();

    return returnValue;
}


// Diese Klasse sorgt für die sichere Freigabe allokierter Otto-Handles
template<typename HandleType>
class OttoHandle {
    public:
        OttoHandle() : OttoHandle(0) {}
        OttoHandle(HandleType handle) : handle(handle), ottoWrapper(OttoWrapper::get()) {}
        ~OttoHandle();

        OttoHandle(const OttoHandle &) = delete;
        OttoHandle &operator=(const OttoHandle &) = delete;

        HandleType *operator&() { return &handle; }
        const HandleType *operator&() const { return &handle; }

        operator HandleType() { return handle; }
        HandleType get() { return handle; }
        const HandleType get() const { return handle; }

    private:
        HandleType handle;
        const OttoWrapper &ottoWrapper;
};

const char* getOttoErrorText(OttoStatusCode rc, const OttoWrapper& otto) {
    const char* errorText = otto.holeFehlertext(rc);
    return errorText ? errorText : "<Kein Fehlertext vorhanden>";
}

template<> OttoHandle<OttoInstanzHandle>::~OttoHandle() { ottoWrapper.instanzFreigeben(handle); }
template<> OttoHandle<OttoRueckgabepufferHandle>::~OttoHandle() { ottoWrapper.rueckgabepufferFreigeben(handle); }
template<> OttoHandle<OttoZertifikatHandle>::~OttoHandle() { ottoWrapper.zertifikatSchliessen(handle); }
template<> OttoHandle<OttoPruefsummeHandle>::~OttoHandle() { ottoWrapper.pruefsummeFreigeben(handle); }
template<> OttoHandle<OttoVersandHandle>::~OttoHandle() { ottoWrapper.versandBeenden(handle); }
template<> OttoHandle<OttoEmpfangHandle>::~OttoHandle() { ottoWrapper.empfangBeenden(handle); }


// Diese Funktion holt Daten zu einer gegebenen Objekt-ID blockweise von OTTER ab und speichert sie in einer Datei
int fetchData(const Arguments &arguments) {

    std::cout << "*** Hole Datei von OTTER ***" << std::endl;
    std::cout << "Objekt-ID: " + arguments.objectId << std::endl;
    std::cout << "Speichere Daten in: " << arguments.outFilePath << std::endl;

    OttoWrapper &otto(OttoWrapper::get());
    if (!otto.loadOtto(arguments.ottoDirPath))
        return EXIT_FAILURE;

    OttoHandle<OttoInstanzHandle> ottoInstance;
    OttoHandle<OttoZertifikatHandle> ottoCertificate;
    OttoHandle<OttoRueckgabepufferHandle> ottoBuffer;
    OttoHandle<OttoEmpfangHandle> ottoFetch;

    // 1. Otto-Instanz erzeugen
    OttoStatusCode ottoStatusCode = otto.instanzErzeugen(arguments.logDirPath.c_str(),nullptr,nullptr,&ottoInstance);

    // 2. Zertifikat für die Authentifizierung öffnen
    if (OTTO_OK == ottoStatusCode)
        ottoStatusCode = otto.zertifikatOeffnen(ottoInstance,arguments.getCertPath(), arguments.getPin(), &ottoCertificate);

    // 3. Puffer für die Rückgabe der abgeholten Datenblöcke erzeugen
    if (OTTO_OK == ottoStatusCode)
        ottoStatusCode = otto.rueckgabepufferErzeugen(ottoInstance, &ottoBuffer);

    // 4. Abholvorgang starten
    if (OTTO_OK == ottoStatusCode)
        ottoStatusCode = otto.empfangBeginnen(ottoInstance,arguments.objectId.c_str(),ottoCertificate,arguments.herstellerId.c_str(),&ottoFetch);

    // 5. Datenblöcke abholen und in die Zieldatei schreiben, bis keine Daten mehr kommen
    uint64_t empfangeneByte = 0;
    if (OTTO_OK == ottoStatusCode) {
        std::cout << "Empfange Daten ";
        std::ofstream targetFile(arguments.outFilePath,std::ofstream::binary | std::ofstream::trunc);
        uint64_t dataSize = 0u;
        do {
            std::cout << '.';
            ottoStatusCode = otto.empfangFortsetzen(ottoFetch, ottoBuffer);
            if (OTTO_OK == ottoStatusCode) {
                dataSize = otto.rueckgabepufferGroesse(ottoBuffer);
                if (0u < dataSize)
                    targetFile.write(otto.rueckgabepufferInhalt(ottoBuffer), static_cast<std::streamsize>(dataSize));
            }
            empfangeneByte += dataSize;
        } while ((OTTO_OK == ottoStatusCode) && (0u < dataSize));
        std::cout << std::endl;
    }

    if (OTTO_OK == ottoStatusCode) {
        std::cout << empfangeneByte << " Byte erfolgreich abgeholt." << std::endl;
    }
    else {
        std::cout << "Datenabholung fehlgeschlagen: Statuscode = " << static_cast<int>(ottoStatusCode)
        << " (" << getOttoErrorText(ottoStatusCode, otto) << ")" << std::endl;
    }


    return static_cast<int>(ottoStatusCode);
}


// Diese Funktion liest Daten blockweise aus einer lokalen Datei ein und sendet sie an OTTER
// OTTER liefert im Erfolgsfall eine Objekt-ID für die hochgeladenen Daten zurück
int sendData(const Arguments &arguments) {

    std::cout << "*** Sende Datei zum OTTER ***" << std::endl;

    OttoWrapper &otto(OttoWrapper::get());
    if (!otto.loadOtto(arguments.ottoDirPath))
        return EXIT_FAILURE;

    OttoHandle<OttoInstanzHandle> ottoInstance;
    OttoHandle<OttoZertifikatHandle> ottoCertificate;
    OttoHandle<OttoRueckgabepufferHandle> ottoBuffer;
    OttoHandle<OttoPruefsummeHandle> ottoHash;
    OttoHandle<OttoVersandHandle> ottoSend;

    // 1. Otto-Instanz erzeugen
    OttoStatusCode ottoStatusCode = otto.instanzErzeugen(arguments.logDirPath.c_str(),nullptr,nullptr,&ottoInstance);

    // 2. Zertifikat für die Signierung der Prüfsumme öffnen
    if(OTTO_OK == ottoStatusCode)
        ottoStatusCode = otto.zertifikatOeffnen(ottoInstance,arguments.getCertPath(),arguments.getPin(),&ottoCertificate);

    // 3. Puffer für die Rückgabe der signierten Prüfsumme erzeugen
    if(OTTO_OK == ottoStatusCode)
        ottoStatusCode = otto.rueckgabepufferErzeugen(ottoInstance,&ottoBuffer);

    // 4. Prüfsummenberechnung starten
    if (OTTO_OK == ottoStatusCode)
        ottoStatusCode = otto.pruefsummeErzeugen(ottoInstance,&ottoHash);

    // 5. Daten blockweise (1 MiB) aus der Quelldatei lesen und eine Prüfsumme darüber bilden
    if (OTTO_OK == ottoStatusCode) {
        std::cout << "Erstelle signierte Pruefsumme ";
        std::ifstream sourceFile(arguments.inFilePath,std::ofstream::binary);
        const uint64_t dataSize =1024u * 1024u; // 1 MiB
        auto readBuffer = std::make_unique<char []>(dataSize);
        do {
            std::cout << '.';
            sourceFile.read(readBuffer.get(), dataSize);
            if (0u < sourceFile.gcount())
                ottoStatusCode = otto.pruefsummeAktualisieren(ottoHash, readBuffer.get(), static_cast<uint64_t>(sourceFile.gcount()));
        } while ((OTTO_OK == ottoStatusCode) && !sourceFile.eof());
        std::cout << std::endl;
    }

    // 5. Prüfsumme signieren
    if (OTTO_OK == ottoStatusCode)
        ottoStatusCode = otto.pruefsummeSignieren(ottoHash,ottoCertificate,ottoBuffer);

    // 6. Versandvorgang mit Zertifikat und signierter Prüfsumme starten
    if (OTTO_OK == ottoStatusCode) {
        std::cout << "Pruefsumme wurde erzeugt und signiert." << std::endl;
        ottoStatusCode = otto.versandBeginnen(ottoInstance, otto.rueckgabepufferInhalt(ottoBuffer), arguments.herstellerId.c_str(), &ottoSend);
    }

    // 7. Daten in 1 MiB großen Blöcken aus der Quelldatei lesen und an OTTER senden
    if (OTTO_OK == ottoStatusCode) {
        std::cout << "Versende Daten ";
        std::ifstream sourceFile(arguments.inFilePath,std::ofstream::binary);
        const uint64_t dataSize = 1024u * 1024u; // 1 MiB
        auto readBuffer = std::make_unique<char[]>(dataSize);
        do {
            std::cout << '.';
            sourceFile.read(readBuffer.get(),dataSize);
            if(0u < sourceFile.gcount())
                ottoStatusCode = otto.versandFortsetzen(ottoSend, readBuffer.get(), static_cast<uint64_t>(sourceFile.gcount()));
        } while((OTTO_OK == ottoStatusCode) && !sourceFile.eof());
        std::cout << std::endl;
    }

    // 8. Versandvorgang abschließen und für den Upload erhaltene Objekt-ID ausgeben
    if (OTTO_OK == ottoStatusCode) {
        ottoStatusCode = otto.versandAbschliessen(ottoSend, ottoBuffer);
        if (OTTO_OK == ottoStatusCode)
            std::cout << "Versand erfolgreich: Objekt-ID = " << otto.rueckgabepufferInhalt(ottoBuffer) << std::endl;
    }

    if (OTTO_OK != ottoStatusCode) {
        std::cout << "Versand fehlgeschlagen: Statuscode = " << static_cast<int>(ottoStatusCode)
                  << " (" << getOttoErrorText(ottoStatusCode, otto) << ")" << std::endl;
    }

    return  static_cast<int>(ottoStatusCode);
}

} // anonymous namespace


int main(int argc, char* argv[]) {
    const Arguments arguments(parseArguments(argc, argv));

    if (!arguments.parseOk || arguments.showHelp) {
        arguments.help();
        return waitForEnter(arguments.parseOk ? EXIT_SUCCESS : EXIT_FAILURE);
    } else if (arguments.fetchData) {
        return waitForEnter(fetchData(arguments));
    } else if (arguments.sendData) {
        return waitForEnter(sendData(arguments));
    } else {
        std::cerr << "Keine Aktion angegeben" << std::endl;
        arguments.help();
        return waitForEnter(EXIT_FAILURE);
    }
}
