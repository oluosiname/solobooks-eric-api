#ifndef _ERICVORGANG_H_
#define _ERICVORGANG_H_

#include <string>
#include <ericapi.h>

// Vorwaertsdeklarationen
class Eric;
class EricZertifikat;

namespace System { class KommandozeilenParser; }

/** @brief Sammelt Vorgangsdaten und versendet einen Steuerdatensatz */
class EricVorgang
{
public:
    /** @brief Erzeugt eine Instanz der Klasse 'EricVorgang'
      *
      * @param eric
      *        Schnittstellenobjekt, das den ERiC kapselt und ueber das
      *        Validierung und/oder Versand durchgefuehrt werden.
      *        Das uebergebene Objekt muss mindestens so lange leben, wie
      *        die erzeugte Instanz der Klasse EricVorgang, da diese eine Referenz darauf haelt!
      */
    explicit EricVorgang(const Eric &eric);

    virtual ~EricVorgang();

    /** @brief Führt den Vorgang gemäß der übergebenen Argumente aus
      *        und liefert das Ergebnis sowie gegebenenfalls bei Versand die Serverantwort
      *        und bei einer Datenabholung das Transferhandle zurück
      */
    int ausfuehren( const System::KommandozeilenParser &argParser, const EricZertifikat *zertifikat,
                    std::string &ergebnis, std::string &antwort, EricTransferHandle &transferHandle ) const;

    /** @brief Lese den Steuersatz aus einer Datei ein
      */
    void leseDatensatz(const std::string& dateiName);

private:
    const Eric &                    ericAdapter;
    std::string                     xmlDaten;
};

#endif
