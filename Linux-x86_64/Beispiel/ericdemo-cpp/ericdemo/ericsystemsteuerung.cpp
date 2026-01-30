#include <platform.h>
#include <ericapi.h>

#include "ericsystemsteuerung.h"
#include "anwendungsfehler.h"
#include "eric.h"
#include "system.h"


using std::string;


EricSystemsteuerung::EricSystemsteuerung(void)
{
}

EricSystemsteuerung::~EricSystemsteuerung(void)
{
}


void EricSystemsteuerung::setzeAlleEinstellungenZurueck(const Eric &eric)
{
    if (eric.EricEinstellungAlleZuruecksetzen() != 0)
    {
        throw Anwendungsfehler("Konfigurationsdateien koennen nicht zurueckgesetzt werden.");
    }
}

void EricSystemsteuerung::protokolliereSystemeigenschaften(const Eric &eric)
{
    if (eric.EricSystemCheck() != 0)
    {
        throw Anwendungsfehler("Systemeigenschaften koennen nicht protokolliert werden.");
    }
}

