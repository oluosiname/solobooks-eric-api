#ifndef _ERICDEKODIERUNG_H_
#define _ERICDEKODIERUNG_H_

#include <string>

// Vorwaertsdeklarationen
class Eric;
class EricZertifikat;
namespace System { class KommandozeilenParser; }

/** @brief Entschlüsselt einen Datensatz  */
class EricDekodierung
{
public:
    /** @brief Erzeugt eine Instanz der Klasse 'EricDekodierung'
      *
      * @param eric
      *        Schnittstellenobjekt, das den ERiC kapselt und über das
      *        die Entschlüsselung durchgefuehrt wird.
      *        Das uebergebene Objekt muss mindestens so lange leben, wie
      *        die erzeugte Instanz der Klasse EricDekodierung, da diese eine Referenz darauf haelt!
      */
    explicit EricDekodierung(const Eric &eric);


    /** @brief Führt die Dekodierung mit dem übergebenen Zertifikat aus und
      *        gibt die entschlüsselten Daten zurück
      *
      * @exception Anwendungsfehler, wenn kein Zertifikat angegeben wurde
      */
    int ausfuehren( const System::KommandozeilenParser &argParser,
                    const EricZertifikat *zertifikat,
                    std::string &ergebnis ) const;

    /** @brief Lese die verschlüsselten Daten aus einer Datei ein
      */
    void leseDatensatz(const std::string& dateiName);

private:
    const Eric &ericAdapter;
    std::string verschluesselteDaten;
};

#endif
