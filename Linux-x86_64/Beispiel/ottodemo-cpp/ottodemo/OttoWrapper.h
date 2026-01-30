#pragma once

#include <otto.h>

#include <string>


/* Signleton-Klasse zum Laden der Otto-Bibliothek und zur Kapselung der Otto-API */

class OttoWrapper {
    public:
        static OttoWrapper &get();

        ~OttoWrapper();

        OttoWrapper(const OttoWrapper &) = delete;
        OttoWrapper &operator=(const OttoWrapper &) = delete;

        bool loadOtto(const std::string &libDirPath);

        // Gekapselte Funktionen der Otto-API.  Für eine Beschreibung der Funktionen siehe otto.h
        OttoStatusCode  instanzErzeugen(const byteChar * const logPfad,OttoLogCallback logCallback,void *logCallbackBenutzerdaten,OttoInstanzHandle *instanz) const;
        OttoStatusCode  instanzFreigeben(OttoInstanzHandle instanz) const;
        OttoStatusCode  zertifikatOeffnen(OttoInstanzHandle instanz,const byteChar *zertifikatsPfad,const byteChar *zertifikatsPasswort,OttoZertifikatHandle *zertifikat) const;
        OttoStatusCode  zertifikatSchliessen(OttoZertifikatHandle zertifikat) const;
        OttoStatusCode  rueckgabepufferErzeugen(OttoInstanzHandle instanz,OttoRueckgabepufferHandle *rueckgabepuffer) const;
        uint64_t        rueckgabepufferGroesse(OttoRueckgabepufferHandle rueckgabepuffer) const;
        const byteChar *rueckgabepufferInhalt(OttoRueckgabepufferHandle rueckgabepuffer) const;
        OttoStatusCode  rueckgabepufferFreigeben(OttoRueckgabepufferHandle rueckgabepuffer) const;
        OttoStatusCode  pruefsummeErzeugen(OttoInstanzHandle instanz,OttoPruefsummeHandle *pruefsumme) const;
        OttoStatusCode  pruefsummeAktualisieren(OttoPruefsummeHandle pruefsumme,const byteChar *datenBlock,uint64_t datenBlockGroesse) const;
        OttoStatusCode  pruefsummeSignieren(OttoPruefsummeHandle pruefsumme,OttoZertifikatHandle zertifikat,OttoRueckgabepufferHandle rueckgabepuffer) const;
        OttoStatusCode  pruefsummeFreigeben(OttoPruefsummeHandle pruefsumme) const;
        OttoStatusCode  versandBeginnen(OttoInstanzHandle instanz,const byteChar *signiertePruefsumme,const byteChar *herstellerId,OttoVersandHandle *versand) const;
        OttoStatusCode  versandFortsetzen(OttoVersandHandle versand,const byteChar *datenBlock,uint64_t datenBlockGroesse) const;
        OttoStatusCode  versandAbschliessen(OttoVersandHandle versand,OttoRueckgabepufferHandle objektId) const;
        OttoStatusCode  versandBeenden(OttoVersandHandle versand) const;
        OttoStatusCode  empfangBeginnen(OttoInstanzHandle instanz,const byteChar *objektId,OttoZertifikatHandle zertifikat,const byteChar *herstellerId,OttoEmpfangHandle *empfang) const;
        OttoStatusCode  empfangBeginnenAbholzertifikat(OttoInstanzHandle instanz,const byteChar *objektId,OttoZertifikatHandle zertifikat,const byteChar *herstellerId,const byteChar *abholzertifikat,OttoEmpfangHandle *empfang) const;
        OttoStatusCode  empfangFortsetzen(OttoEmpfangHandle empfang,OttoRueckgabepufferHandle datenBlock) const;
        OttoStatusCode  empfangBeenden(OttoEmpfangHandle empfang) const;
        const char*     holeFehlertext(OttoStatusCode statuscode) const;
        OttoStatusCode  proxyKonfigurationSetzen(OttoInstanzHandle instanz,const OttoProxyKonfiguration *proxyKonfiguration) const;
        OttoStatusCode  version(OttoRueckgabepufferHandle rueckgabepuffer) const;

    private:
        OttoWrapper();

        void *dylibHandle;

        // Funktionstypen und -zeiger für Otto-Funktionen
        using OttoInstanzErzeugen = decltype(&::OttoInstanzErzeugen);
        OttoInstanzErzeugen ottoInstanzErzeugen;

        using OttoInstanzFreigeben = decltype(&::OttoInstanzFreigeben);
        OttoInstanzFreigeben ottoInstanzFreigeben;

        using OttoZertifikatOeffnen = decltype(&::OttoZertifikatOeffnen);
        OttoZertifikatOeffnen ottoZertifikatOeffnen;

        using OttoZertifikatSchliessen = decltype(&::OttoZertifikatSchliessen);
        OttoZertifikatSchliessen ottoZertifikatSchliessen;

        using OttoRueckgabepufferErzeugen = decltype(&::OttoRueckgabepufferErzeugen);
        OttoRueckgabepufferErzeugen ottoRueckgabepufferErzeugen;

        using OttoRueckgabepufferGroesse = decltype(&::OttoRueckgabepufferGroesse);
        OttoRueckgabepufferGroesse ottoRueckgabepufferGroesse;

        using OttoRueckgabepufferInhalt = decltype(&::OttoRueckgabepufferInhalt);
        OttoRueckgabepufferInhalt ottoRueckgabepufferInhalt;

        using OttoRueckgabepufferFreigeben = decltype(&::OttoRueckgabepufferFreigeben);
        OttoRueckgabepufferFreigeben ottoRueckgabepufferFreigeben;

        using OttoPruefsummeErzeugen = decltype(&::OttoPruefsummeErzeugen);
        OttoPruefsummeErzeugen ottoPruefsummeErzeugen;

        using OttoPruefsummeAktualisieren = decltype(&::OttoPruefsummeAktualisieren);
        OttoPruefsummeAktualisieren ottoPruefsummeAktualisieren;

        using OttoPruefsummeSignieren = decltype(&::OttoPruefsummeSignieren);
        OttoPruefsummeSignieren ottoPruefsummeSignieren;

        using OttoPruefsummeFreigeben = decltype(&::OttoPruefsummeFreigeben);
        OttoPruefsummeFreigeben ottoPruefsummeFreigeben;

        using OttoVersandBeginnen = decltype(&::OttoVersandBeginnen);
        OttoVersandBeginnen ottoVersandBeginnen;

        using OttoVersandFortsetzen = decltype(&::OttoVersandFortsetzen);
        OttoVersandFortsetzen ottoVersandFortsetzen;

        using OttoVersandAbschliessen = decltype(&::OttoVersandAbschliessen);
        OttoVersandAbschliessen ottoVersandAbschliessen;

        using OttoVersandBeenden = decltype(&::OttoVersandBeenden);
        OttoVersandBeenden ottoVersandBeenden;

        using OttoEmpfangBeginnen = decltype(&::OttoEmpfangBeginnen);
        OttoEmpfangBeginnen ottoEmpfangBeginnen;

        using OttoEmpfangBeginnenAbholzertifikat = decltype(&::OttoEmpfangBeginnenAbholzertifikat);
        OttoEmpfangBeginnenAbholzertifikat ottoEmpfangBeginnenAbholzertifikat;

        using OttoEmpfangFortsetzen = decltype(&::OttoEmpfangFortsetzen);
        OttoEmpfangFortsetzen ottoEmpfangFortsetzen;

        using OttoEmpfangBeenden = decltype(&::OttoEmpfangBeenden);
        OttoEmpfangBeenden ottoEmpfangBeenden;

        using OttoHoleFehlertext = decltype(&::OttoHoleFehlertext);
        OttoHoleFehlertext ottoHoleFehlertext;

        using OttoProxyKonfigurationSetzen = decltype(&::OttoProxyKonfigurationSetzen);
        OttoProxyKonfigurationSetzen ottoProxyKonfigurationSetzen;

        using OttoVersion = decltype(&::OttoVersion);
        OttoVersion ottoVersion;
};
