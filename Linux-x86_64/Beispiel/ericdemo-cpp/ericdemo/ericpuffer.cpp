#include "ericpuffer.h"
#include "anwendungsfehler.h"
#include "system.h"

#include <algorithm>
#include <string.h>

using std::string;

EricPuffer::EricPuffer(const Eric& eric) : mEricAdapter(eric)
{
    mPufferHandle = mEricAdapter.EricRueckgabepufferErzeugen();
    if(mPufferHandle == nullptr) {
        throw Anwendungsfehler("Erzeugung des Rueckgabepuffers fehlgeschlagen.");
    }
}

EricPuffer::~EricPuffer()
{
    int rc = mEricAdapter.EricRueckgabepufferFreigeben(mPufferHandle);
    if(rc != 0) {
        std::cerr << "Freigeben des Rueckgabepuffers fehlgeschlagen." << std::endl;
    }
}

EricRueckgabepufferHandle EricPuffer::handle() const
{
    return mPufferHandle;
}

const char* EricPuffer::inhalt() const
{
    const char* antwort = mEricAdapter.EricRueckgabepufferInhalt(mPufferHandle);
    if(antwort == nullptr) {
        throw Anwendungsfehler("Zugriff auf Inhalt des Rueckgabepuffers fehlgeschlagen.");
    }
    return antwort;
}

uint32_t EricPuffer::laenge() const
{
    return mEricAdapter.EricRueckgabepufferLaenge(mPufferHandle);
}

