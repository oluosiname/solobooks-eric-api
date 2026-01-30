#include <iostream>
#include <sstream>

#include "eric.h"
#include "anwendungsfehler.h"
#include "resolve.h"
#include "ericsystemsteuerung.h"
#include "system.h"

const std::string pfadSeparator(1, PFAD_SEPARATOR);
const std::string dirLib(VER_BIB);

namespace {

    /** @brief Lade eine Funktion aus einer Bibliothek */
    template<class Funktionstyp>
    Funktionstyp ladeFunktion(const char* funktionsName, Resolve::Library lib)
    {
        Funktionstyp f = Resolve::function<Funktionstyp>(lib, funktionsName);
        if (f == nullptr)
        {
            throw Anwendungsfehler(std::string(funktionsName) + ": konnte nicht geladen werden.");
        }
        return f;
    }

    // Ein typedef und ein Funktionszeiger je ERiC API-Funktion
    typedef int (STDCALL *EricInitialisiereFun)(const char *pluginPfad, const char *logPfad);
    EricInitialisiereFun EricInitialisierePtr;

    typedef int (STDCALL *EricBeendeFun)();
    EricBeendeFun EricBeendePtr;

    typedef int (STDCALL *EricBearbeiteVorgangFun)(
        const char* datenpuffer,
        const char* datenartVersion,
        uint32_t bearbeitungsFlags,
        const eric_druck_parameter_t *druckParameter,
        const eric_verschluesselungs_parameter_t *cryptoParameter,
        EricTransferHandle *transferHandle,
        EricRueckgabepufferHandle rueckgabeXmlPuffer,
        EricRueckgabepufferHandle serverantwortXmlPuffer);
    EricBearbeiteVorgangFun EricBearbeiteVorgangPtr;

    typedef int (STDCALL *EricGetHandleToCertificateFun)(EricZertifikatHandle *hToken,
        uint32_t *iInfoPinSupport,
        const char *pathToKeystore);
    EricGetHandleToCertificateFun EricGetHandleToCertificatePtr;

    typedef int (STDCALL *EricCloseHandleToCertificateFun)(EricZertifikatHandle hToken);
    EricCloseHandleToCertificateFun EricCloseHandleToCertificatePtr;

    typedef int (STDCALL * EricDekodiereDatenFun) (
        EricZertifikatHandle zertifikatHandle,
        const char * pin,
        const char * base64Eingabe,
        EricRueckgabepufferHandle rueckgabeXmlPuffer);
    EricDekodiereDatenFun EricDekodiereDatenPtr;

    typedef int (STDCALL *EricEinstellungAlleZuruecksetzenFun)();
    EricEinstellungAlleZuruecksetzenFun EricEinstellungAlleZuruecksetzenPtr;

    typedef int (STDCALL *EricEinstellungSetzenFun)(const char* name, const char* wert);
    EricEinstellungSetzenFun EricEinstellungSetzenPtr;

    typedef int (STDCALL *EricHoleFehlerTextFun)(
        int fehlerkode, EricRueckgabepufferHandle rueckgabePuffer);
    EricHoleFehlerTextFun EricHoleFehlerTextPtr;

    typedef int (STDCALL *EricPruefeSteuernummerFun)(
        const char *Steuernummer);
    EricPruefeSteuernummerFun EricPruefeSteuernummerPtr;

    typedef int (STDCALL *EricSystemCheckFun)();
    EricSystemCheckFun EricSystemCheckPtr;

    typedef int (STDCALL *EricRegistriereGlobalenFortschrittCallbackFun)(
        EricFortschrittCallback func,
        void *userData);
    EricRegistriereGlobalenFortschrittCallbackFun EricRegistriereGlobalenFortschrittCallbackPtr;

    typedef int (STDCALL *EricRegistriereFortschrittCallbackFun)(
        EricFortschrittCallback func,
        void *userData);
    EricRegistriereFortschrittCallbackFun EricRegistriereFortschrittCallbackPtr;

    typedef EricRueckgabepufferHandle (STDCALL *EricRueckgabepufferErzeugenFun) (void);
    EricRueckgabepufferErzeugenFun EricRueckgabepufferErzeugenPtr;

    typedef const char* (STDCALL *EricRueckgabepufferInhaltFun) (EricRueckgabepufferHandle handle);
    EricRueckgabepufferInhaltFun EricRueckgabepufferInhaltPtr;

    typedef uint32_t (STDCALL *EricRueckgabepufferLaengeFun) (EricRueckgabepufferHandle handle);
    EricRueckgabepufferLaengeFun EricRueckgabepufferLaengePtr;

    typedef int (STDCALL *EricRueckgabepufferFreigebenFun) (EricRueckgabepufferHandle handle);
    EricRueckgabepufferFreigebenFun EricRueckgabepufferFreigebenPtr;

    typedef int (STDCALL *EricEntladePluginsFun)();
    EricEntladePluginsFun EricEntladePluginsPtr;

    typedef int(STDCALL *EricCreateTHFun) (
        const char* xml,
        const char* verfahren,
        const char* datenart,
        const char* vorgang,
        const char* testmerker,
        const char* herstellerId,
        const char* datenLieferant,
        const char* versionClient,
        const char* publicKey,
        EricRueckgabepufferHandle rueckgabeXmlPuffer);
    EricCreateTHFun EricCreateTHPtr;

    typedef int(STDCALL *EricHoleZertifikatEigenschaftenFun) (
        EricZertifikatHandle hToken,
        const char* pin,
        EricRueckgabepufferHandle rueckgabeXmlPuffer);
    EricHoleZertifikatEigenschaftenFun EricHoleZertifikatEigenschaftenPtr;
}

bool Eric::ladeEricApi(const std::string &apiVerzeichnis)
{
    static const std::string ericapiDateiname = System::getBibliotheksDateiname("ericapi");

    if (!istGeladen()) {
        libEricApi = Resolve::library<>(
#ifdef WINDOWS_MSVC
            System::kod::toUtf16(System::dateiPfad(apiVerzeichnis, ericapiDateiname))
#else 
            System::dateiPfad(apiVerzeichnis, ericapiDateiname)
#endif
            .c_str());
        if (nullptr != libEricApi) {

            try
            {
                EricInitialisierePtr                          = ladeFunktion<EricInitialisiereFun>("EricInitialisiere", libEricApi);
                EricBeendePtr                                 = ladeFunktion<EricBeendeFun>("EricBeende", libEricApi);
                EricBearbeiteVorgangPtr                       = ladeFunktion<EricBearbeiteVorgangFun>("EricBearbeiteVorgang", libEricApi);
                EricGetHandleToCertificatePtr                 = ladeFunktion<EricGetHandleToCertificateFun>("EricGetHandleToCertificate", libEricApi);
                EricCloseHandleToCertificatePtr               = ladeFunktion<EricCloseHandleToCertificateFun>("EricCloseHandleToCertificate", libEricApi);
                EricHoleFehlerTextPtr                         = ladeFunktion<EricHoleFehlerTextFun>("EricHoleFehlerText", libEricApi);
                EricPruefeSteuernummerPtr                     = ladeFunktion<EricPruefeSteuernummerFun>("EricPruefeSteuernummer", libEricApi);
                EricSystemCheckPtr                            = ladeFunktion<EricSystemCheckFun>("EricSystemCheck", libEricApi);
                EricEinstellungSetzenPtr                      = ladeFunktion<EricEinstellungSetzenFun>("EricEinstellungSetzen", libEricApi);
                EricEinstellungAlleZuruecksetzenPtr           = ladeFunktion<EricEinstellungAlleZuruecksetzenFun>("EricEinstellungAlleZuruecksetzen", libEricApi);
                EricRegistriereGlobalenFortschrittCallbackPtr = ladeFunktion<EricRegistriereGlobalenFortschrittCallbackFun>("EricRegistriereGlobalenFortschrittCallback", libEricApi);
                EricRegistriereFortschrittCallbackPtr         = ladeFunktion<EricRegistriereFortschrittCallbackFun>("EricRegistriereFortschrittCallback", libEricApi);
                EricRueckgabepufferErzeugenPtr                = ladeFunktion<EricRueckgabepufferErzeugenFun>("EricRueckgabepufferErzeugen", libEricApi);
                EricRueckgabepufferInhaltPtr                  = ladeFunktion<EricRueckgabepufferInhaltFun>("EricRueckgabepufferInhalt", libEricApi);
                EricRueckgabepufferLaengePtr                  = ladeFunktion<EricRueckgabepufferLaengeFun>("EricRueckgabepufferLaenge", libEricApi);
                EricRueckgabepufferFreigebenPtr               = ladeFunktion<EricRueckgabepufferFreigebenFun>("EricRueckgabepufferFreigeben", libEricApi);
                EricEntladePluginsPtr                         = ladeFunktion<EricEntladePluginsFun>("EricEntladePlugins", libEricApi);
                EricCreateTHPtr                               = ladeFunktion<EricCreateTHFun>("EricCreateTH", libEricApi);
                EricHoleZertifikatEigenschaftenPtr            = ladeFunktion<EricHoleZertifikatEigenschaftenFun>("EricHoleZertifikatEigenschaften", libEricApi);
                EricDekodiereDatenPtr                         = ladeFunktion<EricDekodiereDatenFun>("EricDekodiereDaten", libEricApi);
            }
            catch (const Anwendungsfehler &)
            {
                entladeEricApi();
            }
        }
    }

    return istGeladen();
}

void Eric::entladeEricApi()
{
    if ( istGeladen() )
    {
        EricBeende();
        Resolve::free_library(libEricApi);
        libEricApi = nullptr;
    }
}

// ERiC Initialisieren
Eric::Eric(const std::string &argHomeDir, const std::string &argLogDir) : libEricApi(nullptr)
{
    static const int STATUS_OK = 0;
    int statusCode = STATUS_OK;
    bool initialisierungsFehler = false;
    std::stringstream statusMeldung;
    std::string homeDir;
    const std::string DIR_UP("..");

    // Heimverzeichnis aus Programmparameter verwenden. Falls nicht möglich,
    // verwende das Arbeitsverzeichnis und ergänze es.
    if (argHomeDir.empty() || System::istPfadRelativ(argHomeDir.c_str()))
    {
        if (System::getArbeitsverzeichnis(homeDir))
        {
            homeDir += (argHomeDir.empty() ? DIR_UP + pfadSeparator + DIR_UP + pfadSeparator + dirLib : argHomeDir);            
        }
        else
        {
            throw Anwendungsfehler("Das Arbeitsverzeichnis konnte nicht ermittelt werden.");
        }
    }
    else
    {
        homeDir = argHomeDir;
    }

    // 1.  Lade ericapi
    if (!ladeEricApi(homeDir))
    {
        initialisierungsFehler = true;
        statusMeldung << "Die Programmbibliothek ericapi konnte nicht geladen werden.";
    }

    // 2.  Pfad fuer Protokolldateien und Druckdateien setzen
    if (istGeladen())
    {
        std::string logDir;

        if (argLogDir.empty() || System::istPfadRelativ(argLogDir.c_str()))
        {
            if (!System::getArbeitsverzeichnis(logDir))
            {
                throw Anwendungsfehler("Das Arbeitsverzeichnis konnte nicht ermittelt werden.");
            }
        }
        else
        {
            logDir = argLogDir;
        }

        statusCode = EricInitialisiere(
#ifdef WINDOWS_MSVC
            System::kod::toWindowsZeichenKodierung(homeDir).c_str(), System::kod::toWindowsZeichenKodierung(logDir).c_str()
#else
            homeDir.c_str(), logDir.c_str()
#endif
        );
        if (STATUS_OK != statusCode)
        {
            initialisierungsFehler = true;
            statusMeldung << "ERiC-Initialisierung fehlgeschlagen (Fehler " << statusCode << ")";
        }
    }

    // Fehlerbehandlung
    if (initialisierungsFehler)
    {
        entladeEricApi();
        throw Anwendungsfehler(statusMeldung.str());
    }
}

Eric::~Eric()
{
    if (istGeladen()) {
        entladeEricApi();
    }
}

bool Eric::istGeladen() const {
    return nullptr != libEricApi;
}


// Implementierungen der Proxy-Methoden fuer Funktionen der ericapi

int Eric::EricInitialisiere(const char *pluginPfad, const char *logPfad) {
    return EricInitialisierePtr(pluginPfad, logPfad);
}

int Eric::EricBeende() {
    return EricBeendePtr();
}

int Eric::EricBearbeiteVorgang(const char* datenpuffer,
                               const char* datenartVersion,
                               uint32_t bearbeitungsFlags,
                               const eric_druck_parameter_t *druckParameter,
                               const eric_verschluesselungs_parameter_t *cryptoParameter,
                               EricTransferHandle *transferHandle,
                               EricRueckgabepufferHandle rueckgabeXmlPuffer,
                               EricRueckgabepufferHandle serverantwortXmlPuffer) const
{
    return EricBearbeiteVorgangPtr(datenpuffer, datenartVersion,
        bearbeitungsFlags, druckParameter, cryptoParameter, transferHandle, rueckgabeXmlPuffer, serverantwortXmlPuffer);
}

int Eric::EricGetHandleToCertificate(EricZertifikatHandle * hToken,
                                     uint32_t *iInfoPinSupport,
                                     const char *pathToKeystore) const
{
    return EricGetHandleToCertificatePtr(hToken, iInfoPinSupport, pathToKeystore);
}

int Eric::EricCloseHandleToCertificate(EricZertifikatHandle hToken) const
{
    return EricCloseHandleToCertificatePtr(hToken);
}

int Eric::EricDekodiereDaten(EricZertifikatHandle zertifikatHandle,
                             const char * pin,
                             const char * base64Eingabe,
                             EricRueckgabepufferHandle rueckgabeXmlPuffer) const
{
    return EricDekodiereDatenPtr(zertifikatHandle, pin, base64Eingabe, rueckgabeXmlPuffer);
}

int Eric::EricHoleFehlerText(int fehlerkode, EricRueckgabepufferHandle rueckgabePuffer) const
{
    return EricHoleFehlerTextPtr(fehlerkode, rueckgabePuffer);
}

int Eric::EricPruefeSteuernummer(const char *steuernummer) const
{
    return EricPruefeSteuernummerPtr(steuernummer);
}

int Eric::EricSystemCheck() const
{
    return EricSystemCheckPtr();
}

int Eric::EricRegistriereGlobalenFortschrittCallback(
    EricFortschrittCallback func,
    void *userData) const {
    return EricRegistriereGlobalenFortschrittCallbackPtr(func, userData);
}

int Eric::EricRegistriereFortschrittCallback(
    EricFortschrittCallback func,
    void *userData) const {
    return EricRegistriereFortschrittCallbackPtr(func, userData);
}

int Eric::EricEinstellungAlleZuruecksetzen(void) const {
    return EricEinstellungAlleZuruecksetzenPtr();
}

int Eric::EricEinstellungSetzen(const char* name, const char* wert) const {
    return EricEinstellungSetzenPtr(name, wert);
}

EricRueckgabepufferHandle Eric::EricRueckgabepufferErzeugen() const {
    return EricRueckgabepufferErzeugenPtr();
}

const char* Eric::EricRueckgabepufferInhalt(EricRueckgabepufferHandle handle) const {
    return EricRueckgabepufferInhaltPtr(handle);
}

uint32_t Eric::EricRueckgabepufferLaenge(EricRueckgabepufferHandle handle) const {
    return EricRueckgabepufferLaengePtr(handle);
}

int Eric::EricRueckgabepufferFreigeben(EricRueckgabepufferHandle handle) const {
    return EricRueckgabepufferFreigebenPtr(handle);
}

int Eric::EricCreateTH(const char * xml,
                       const char * verfahren,
                       const char * datenart,
                       const char * vorgang,
                       const char * testmerker,
                       const char * herstellerId,
                       const char * datenLieferant,
                       const char * versionClient,
                       const char * publicKey,
                       EricRueckgabepufferHandle handle) const
{
    return EricCreateTHPtr(xml, verfahren, datenart, vorgang, testmerker, herstellerId,
        datenLieferant, versionClient, publicKey, handle);
}

int Eric::EricHoleZertifikatEigenschaften(EricZertifikatHandle hToken,
                                          const char* pin,
                                          EricRueckgabepufferHandle rueckgabeXmlPuffer) const
{
    return EricHoleZertifikatEigenschaftenPtr(hToken, pin, rueckgabeXmlPuffer);
}
