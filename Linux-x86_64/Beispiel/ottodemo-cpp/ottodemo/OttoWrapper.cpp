#include "OttoWrapper.h"
#include "System.h"

#include <iostream>


namespace {
    auto getFunctionAddr(const char * const functionName, void *dylibHandle, const std::string &libPath) {
        const auto functionAddr = GET_FUNCTION_ADDR(static_cast<DYLIB_HANDLE>(dylibHandle),functionName);
        if (nullptr == functionAddr)
            std::cerr << "Die Funktion \"" << functionName << "\" wurde in der Bibliothek \"" << libPath << "\" nicht gefunden";
        return functionAddr;
    }
} // anonymous namespace

OttoWrapper &OttoWrapper::get() {
    static OttoWrapper ottoWrapper;
    return ottoWrapper;
}

OttoWrapper::OttoWrapper()
    : dylibHandle(nullptr),
      ottoInstanzErzeugen(nullptr),
      ottoInstanzFreigeben(nullptr),
      ottoZertifikatOeffnen(nullptr),
      ottoZertifikatSchliessen(nullptr),
      ottoRueckgabepufferErzeugen(nullptr),
      ottoRueckgabepufferGroesse(nullptr),
      ottoRueckgabepufferInhalt(nullptr),
      ottoRueckgabepufferFreigeben(nullptr),
      ottoPruefsummeErzeugen(nullptr),
      ottoPruefsummeAktualisieren(nullptr),
      ottoPruefsummeSignieren(nullptr),
      ottoPruefsummeFreigeben(nullptr),
      ottoVersandBeginnen(nullptr),
      ottoVersandFortsetzen(nullptr),
      ottoVersandAbschliessen(nullptr),
      ottoVersandBeenden(nullptr),
      ottoEmpfangBeginnen(nullptr),
      ottoEmpfangBeginnenAbholzertifikat(nullptr),
      ottoEmpfangFortsetzen(nullptr),
      ottoEmpfangBeenden(nullptr),
      ottoProxyKonfigurationSetzen(nullptr),
      ottoVersion(nullptr)
{
}

OttoWrapper::~OttoWrapper() {
    if (nullptr != dylibHandle)
        CLOSE_LIBRARY(static_cast<DYLIB_HANDLE>(dylibHandle));
}

bool OttoWrapper::loadOtto(const std::string &libDirPath) {
    std::string libPath = libDirPath;
    if ((!libPath.empty()) && (PATH_SEPARATOR != *libPath.rbegin()))
        libPath += PATH_SEPARATOR;
    libPath += DYLIB_PREFIX "otto" DYLIB_SUFFIX;

    if (nullptr == dylibHandle) {
        RESET_ERROR();
        dylibHandle = OPEN_LIBRARY(libPath.c_str());
    }

    // Laden der otto-Bibliothek
    if (0 == dylibHandle) {
        std::cerr << libPath << " konnte nicht geladen werden: Fehlercode " << GETLASTERROR() << std::endl;
        return false;
    }

    // Adressen der Schnittstellenfunktionen der otto-Bibliothek ermitteln
   return    (nullptr != (ottoInstanzErzeugen = reinterpret_cast<OttoInstanzErzeugen>(getFunctionAddr("OttoInstanzErzeugen",dylibHandle,libPath))))
          && (nullptr != (ottoInstanzFreigeben = reinterpret_cast<OttoInstanzFreigeben>(getFunctionAddr("OttoInstanzFreigeben",dylibHandle,libPath))))
          && (nullptr != (ottoZertifikatOeffnen = reinterpret_cast<OttoZertifikatOeffnen>(getFunctionAddr("OttoZertifikatOeffnen",dylibHandle,libPath))))
          && (nullptr != (ottoZertifikatSchliessen = reinterpret_cast<OttoZertifikatSchliessen>(getFunctionAddr("OttoZertifikatSchliessen",dylibHandle,libPath))))
          && (nullptr != (ottoRueckgabepufferErzeugen = reinterpret_cast<OttoRueckgabepufferErzeugen>(getFunctionAddr("OttoRueckgabepufferErzeugen",dylibHandle,libPath))))
          && (nullptr != (ottoRueckgabepufferGroesse = reinterpret_cast<OttoRueckgabepufferGroesse>(getFunctionAddr("OttoRueckgabepufferGroesse",dylibHandle,libPath))))
          && (nullptr != (ottoRueckgabepufferInhalt = reinterpret_cast<OttoRueckgabepufferInhalt>(getFunctionAddr("OttoRueckgabepufferInhalt",dylibHandle,libPath))))
          && (nullptr != (ottoRueckgabepufferFreigeben = reinterpret_cast<OttoRueckgabepufferFreigeben>(getFunctionAddr("OttoRueckgabepufferFreigeben",dylibHandle,libPath))))
          && (nullptr != (ottoPruefsummeErzeugen = reinterpret_cast<OttoPruefsummeErzeugen>(getFunctionAddr("OttoPruefsummeErzeugen",dylibHandle,libPath))))
          && (nullptr != (ottoPruefsummeAktualisieren = reinterpret_cast<OttoPruefsummeAktualisieren>(getFunctionAddr("OttoPruefsummeAktualisieren",dylibHandle,libPath))))
          && (nullptr != (ottoPruefsummeSignieren = reinterpret_cast<OttoPruefsummeSignieren>(getFunctionAddr("OttoPruefsummeSignieren",dylibHandle,libPath))))
          && (nullptr != (ottoPruefsummeFreigeben = reinterpret_cast<OttoPruefsummeFreigeben>(getFunctionAddr("OttoPruefsummeFreigeben",dylibHandle,libPath))))
          && (nullptr != (ottoVersandBeginnen = reinterpret_cast<OttoVersandBeginnen>(getFunctionAddr("OttoVersandBeginnen",dylibHandle,libPath))))
          && (nullptr != (ottoVersandFortsetzen = reinterpret_cast<OttoVersandFortsetzen>(getFunctionAddr("OttoVersandFortsetzen",dylibHandle,libPath))))
          && (nullptr != (ottoVersandAbschliessen = reinterpret_cast<OttoVersandAbschliessen>(getFunctionAddr("OttoVersandAbschliessen",dylibHandle,libPath))))
          && (nullptr != (ottoVersandBeenden = reinterpret_cast<OttoVersandBeenden>(getFunctionAddr("OttoVersandBeenden",dylibHandle,libPath))))
          && (nullptr != (ottoEmpfangBeginnen = reinterpret_cast<OttoEmpfangBeginnen>(getFunctionAddr("OttoEmpfangBeginnen",dylibHandle,libPath))))
          && (nullptr != (ottoEmpfangBeginnenAbholzertifikat = reinterpret_cast<OttoEmpfangBeginnenAbholzertifikat>(getFunctionAddr("OttoEmpfangBeginnenAbholzertifikat",dylibHandle,libPath))))
          && (nullptr != (ottoEmpfangFortsetzen = reinterpret_cast<OttoEmpfangFortsetzen>(getFunctionAddr("OttoEmpfangFortsetzen",dylibHandle,libPath))))
          && (nullptr != (ottoEmpfangBeenden = reinterpret_cast<OttoEmpfangBeenden>(getFunctionAddr("OttoEmpfangBeenden",dylibHandle,libPath))))
          && (nullptr != (ottoHoleFehlertext = reinterpret_cast<OttoHoleFehlertext>(getFunctionAddr("OttoHoleFehlertext",dylibHandle,libPath))))
          && (nullptr != (ottoProxyKonfigurationSetzen = reinterpret_cast<OttoProxyKonfigurationSetzen>(getFunctionAddr("OttoProxyKonfigurationSetzen",dylibHandle,libPath))))
          && (nullptr != (ottoVersion = reinterpret_cast<OttoVersion>(getFunctionAddr("OttoVersion",dylibHandle,libPath))));
}


// Kapselung der Otto API-Funktionen

OttoStatusCode OttoWrapper::instanzErzeugen(const byteChar * const logPfad,OttoLogCallback logCallback,void *logCallbackBenutzerdaten,OttoInstanzHandle *instanz) const {
    return ottoInstanzErzeugen(logPfad,logCallback,logCallbackBenutzerdaten,instanz);
}

OttoStatusCode OttoWrapper::instanzFreigeben(OttoInstanzHandle instanz) const {
    return ottoInstanzFreigeben(instanz);
}

OttoStatusCode OttoWrapper::zertifikatOeffnen(OttoInstanzHandle instanz,const byteChar *zertifikatsPfad,const byteChar *zertifikatsPasswort,OttoZertifikatHandle *zertifikat) const {
    return ottoZertifikatOeffnen(instanz,zertifikatsPfad,zertifikatsPasswort,zertifikat);
}

OttoStatusCode OttoWrapper::zertifikatSchliessen(OttoZertifikatHandle zertifikat) const {
    return ottoZertifikatSchliessen(zertifikat);
}

OttoStatusCode OttoWrapper::rueckgabepufferErzeugen(OttoInstanzHandle instanz,OttoRueckgabepufferHandle *rueckgabepuffer) const {
    return ottoRueckgabepufferErzeugen(instanz,rueckgabepuffer);
}

uint64_t OttoWrapper::rueckgabepufferGroesse(OttoRueckgabepufferHandle rueckgabepuffer) const {
    return ottoRueckgabepufferGroesse(rueckgabepuffer);
}

const byteChar *OttoWrapper::rueckgabepufferInhalt(OttoRueckgabepufferHandle rueckgabepuffer) const {
    return ottoRueckgabepufferInhalt(rueckgabepuffer);
}

OttoStatusCode OttoWrapper::rueckgabepufferFreigeben(OttoRueckgabepufferHandle rueckgabepuffer) const {
    return ottoRueckgabepufferFreigeben(rueckgabepuffer);
}

OttoStatusCode OttoWrapper::pruefsummeErzeugen(OttoInstanzHandle instanz,OttoPruefsummeHandle *pruefsumme) const {
    return ottoPruefsummeErzeugen(instanz,pruefsumme);
}

OttoStatusCode OttoWrapper::pruefsummeAktualisieren(OttoPruefsummeHandle pruefsumme,const byteChar *datenBlock,uint64_t datenBlockGroesse) const {
    return ottoPruefsummeAktualisieren(pruefsumme,datenBlock,datenBlockGroesse);
}

OttoStatusCode OttoWrapper::pruefsummeSignieren(OttoPruefsummeHandle pruefsumme,OttoZertifikatHandle zertifikat,OttoRueckgabepufferHandle rueckgabepuffer) const {
    return ottoPruefsummeSignieren(pruefsumme,zertifikat,rueckgabepuffer);
}

OttoStatusCode OttoWrapper::pruefsummeFreigeben(OttoPruefsummeHandle pruefsumme) const {
    return ottoPruefsummeFreigeben(pruefsumme);
}

OttoStatusCode OttoWrapper::versandBeginnen(OttoInstanzHandle instanz,const byteChar *signiertePruefsumme,const byteChar *herstellerId,OttoVersandHandle *versand) const {
    return ottoVersandBeginnen(instanz,signiertePruefsumme,herstellerId,versand);
}

OttoStatusCode OttoWrapper::versandFortsetzen(OttoVersandHandle versand,const byteChar *datenBlock,uint64_t datenBlockGroesse) const {
    return ottoVersandFortsetzen(versand,datenBlock,datenBlockGroesse);
}

OttoStatusCode OttoWrapper::versandAbschliessen(OttoVersandHandle versand,OttoRueckgabepufferHandle objektId) const {
    return ottoVersandAbschliessen(versand,objektId);
}

OttoStatusCode OttoWrapper::versandBeenden(OttoVersandHandle versand) const {
    return ottoVersandBeenden(versand);
}

OttoStatusCode OttoWrapper::empfangBeginnen(OttoInstanzHandle instanz,const byteChar *objektId,OttoZertifikatHandle zertifikat,const byteChar *herstellerId,OttoEmpfangHandle *empfang) const {
    return ottoEmpfangBeginnen(instanz,objektId,zertifikat,herstellerId,empfang);
}

OttoStatusCode OttoWrapper::empfangBeginnenAbholzertifikat(OttoInstanzHandle instanz,const byteChar *objektId,OttoZertifikatHandle zertifikat,const byteChar *herstellerId,const byteChar *abholzertifikat,OttoEmpfangHandle *empfang) const {
    return ottoEmpfangBeginnenAbholzertifikat(instanz,objektId,zertifikat,herstellerId,abholzertifikat,empfang);
}

OttoStatusCode OttoWrapper::empfangFortsetzen(OttoEmpfangHandle empfang,OttoRueckgabepufferHandle datenBlock) const {
    return ottoEmpfangFortsetzen(empfang,datenBlock);
}

OttoStatusCode OttoWrapper::empfangBeenden(OttoEmpfangHandle empfang) const {
    return ottoEmpfangBeenden(empfang);
}

const char* OttoWrapper::holeFehlertext(OttoStatusCode statuscode) const {
    return ottoHoleFehlertext(statuscode);
}

OttoStatusCode OttoWrapper::proxyKonfigurationSetzen(OttoInstanzHandle instanz,const OttoProxyKonfiguration *proxyKonfiguration) const {
    return ottoProxyKonfigurationSetzen(instanz,proxyKonfiguration);
}

OttoStatusCode OttoWrapper::version(OttoRueckgabepufferHandle rueckgabepuffer) const {
    return ottoVersion(rueckgabepuffer);
}
