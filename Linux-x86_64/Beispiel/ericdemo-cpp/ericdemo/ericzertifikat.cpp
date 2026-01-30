#include "ericzertifikat.h"
#include "anwendungsfehler.h"
#include "eric.h"
#include "ericpuffer.h"

EricZertifikat::EricZertifikat(const Eric& eric_, const std::string& pfad_, const std::string& pin_)
: eric(eric_),
pfad(pfad_),
#ifdef WINDOWS_MSVC
pin(System::kod::toWindowsZeichenKodierung(pin_))
#else
pin(pin_)
#endif // WINDOWS_MSVC
{
    verschlusselungsParameter.version = 3;
    verschlusselungsParameter.zertifikatHandle = 0;
    verschlusselungsParameter.pin = pin == "_NULL" ? nullptr : pin.c_str();
    
    
    uint32_t pinInfo = 0;
    int rc = eric.EricGetHandleToCertificate(&verschlusselungsParameter.zertifikatHandle, &pinInfo,
#ifdef WINDOWS_MSVC
                                             System::kod::toWindowsZeichenKodierung(pfad)
#else
                                             pfad
#endif
                                             .c_str());

    if (0 != rc)
    {
        throw Anwendungsfehler(std::string("Das Zertifikat \"") + pfad + "\" konnte nicht geladen werden.");
    }

    EricPuffer ericPuffer(eric);
    rc = eric.EricHoleZertifikatEigenschaften(verschlusselungsParameter.zertifikatHandle, verschlusselungsParameter.pin, ericPuffer.handle());
    if (0 == rc)
    {
        eigenschaften.assign(ericPuffer.inhalt(),ericPuffer.laenge());
    }
    else
    {
        eigenschaften = std::string("Die Zertifikatseigenschaften konnten nicht ermittelt werden. Fehlercode ") + System::toString(rc);
    }
}

EricZertifikat::~EricZertifikat()
{
    eric.EricCloseHandleToCertificate(verschlusselungsParameter.zertifikatHandle);
}
