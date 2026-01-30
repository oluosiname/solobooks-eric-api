#include "callbackhandler.h"

#include <string>
#include <stdexcept>

#include "eric.h"
#include "system.h"

namespace {

static void STDCALL globalerFortschrittAdapter(uint32_t id, uint32_t pos, uint32_t max, void *userData) {
    const CallbackHandler *const handler = reinterpret_cast<const CallbackHandler *>(userData);
    handler->globalerFortschritt(id, pos, max);
}

static void STDCALL fortschrittAdapter(uint32_t id, uint32_t pos, uint32_t max, void *userData) {
    CallbackHandler *const handler = reinterpret_cast<CallbackHandler *>(userData);
    handler->fortschritt(id, pos, max);
}

std::string mapId(uint32_t id) {
    switch (id) {
    case ERIC_FORTSCHRITTCALLBACK_ID_EINLESEN:
        return "XML einlesen";
    case ERIC_FORTSCHRITTCALLBACK_ID_VORBEREITEN:
        return "Versand vorbereiten";
    case ERIC_FORTSCHRITTCALLBACK_ID_VALIDIEREN:
        return "Validieren";
    case ERIC_FORTSCHRITTCALLBACK_ID_SENDEN:
        return "Versenden";
    case ERIC_FORTSCHRITTCALLBACK_ID_DRUCKEN:
        return "Drucken";
    default:
        throw std::runtime_error("Unbekannte ID");
    }
}

}

CallbackHandler::CallbackHandler(const Eric& myEric)
    : eric(myEric), letzteId(0), letzteSpalte(0)
{
    // Callbacks fuer Fortschrittsbalken und Nachrichten anmelden
    eric.EricRegistriereGlobalenFortschrittCallback(globalerFortschrittAdapter, this);
    eric.EricRegistriereFortschrittCallback(fortschrittAdapter, this);
}

CallbackHandler::~CallbackHandler()
{
    // Callbacks Abmelden
    eric.EricRegistriereGlobalenFortschrittCallback(nullptr, nullptr);
    eric.EricRegistriereFortschrittCallback(nullptr, nullptr);
}

void CallbackHandler::globalerFortschritt(uint32_t id, uint32_t pos, uint32_t max) const {
    std::cout << pos << "/" << max << ": " << mapId(id) << std::endl;
}

void CallbackHandler::fortschritt(uint32_t id, uint32_t pos, uint32_t max) {
    if (letzteId != 0 && letzteId != id) {
        std::cout << std::string(78 - letzteSpalte, '#') << ']' << std::endl;
        letzteId = 0;
        letzteSpalte = 0;
        fortschritt(id, pos, max);
    } else {
        if (pos == 0) {
            std::cout << '[';
            letzteId = id;
            letzteSpalte = 0;
        } else if (pos == max) {
            std::cout << std::string(78 - letzteSpalte, '#') << ']' << std::endl;
            letzteId = 0;
            letzteSpalte = 0;
        } else {
            unsigned int spalte = (pos * 78) / max;
            std::cout << std::string(spalte - letzteSpalte, '#');
            letzteSpalte = spalte;
        }
    }
}
