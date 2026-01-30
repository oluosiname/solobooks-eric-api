#ifndef _ERICSYSTEMSTEUERUNG_H_
#define _ERICSYSTEMSTEUERUNG_H_

#include <string>


// Vorwaertsdeklaration
class Eric;


/** @brief Stellt Schnittstellen zur Konfiguration von ERiC-Systemeinstellungen bereit. */
class EricSystemsteuerung
{
public:
    EricSystemsteuerung();
    ~EricSystemsteuerung();

    /** @brief Schreibe die Systemeigenschaften in die Protokolldatei. */
    static void protokolliereSystemeigenschaften(const Eric &eric);

    /** @brief Setzt alle ERiC Einstellungen zurueck. */
    static void setzeAlleEinstellungenZurueck(const Eric &eric);
};

#endif
