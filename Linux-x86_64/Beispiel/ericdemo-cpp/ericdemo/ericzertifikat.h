#ifndef _ERICZERTIFIKAT_H_
#define _ERICZERTIFIKAT_H_

#include <string>
#include <ericapi.h>
#include "system.h"

// Vorwaertsdeklarationen
class Eric;


/** @brief Verwaltet ein ERiC-Zertifikat */
class EricZertifikat
{
public:
    /** @brief Erzeugt eine Instanz der Klasse 'EricZertifikat'
      *
      * @param eric
      *        Schnittstellenobjekt, das den ERiC kapselt.
      *        Das uebergebene Objekt muss mindestens so lange leben, wie
      *        die erzeugte Instanz der Klasse EricZertifikat, da diese eine Referenz darauf haelt!
      * @param pfad Pfad zur Zertifikatsdatei für ein Software-Zertifikat
      * @param pin Pin der Zertifikatsdatei
      */
    EricZertifikat(const Eric &eric, const std::string &pfad, const std::string &pin);

    ~EricZertifikat();

    const char          *getPin()  const { return verschlusselungsParameter.pin; }
    const std::string   &getPfad() const { return pfad;  }
    const std::string   &getEigenschaften() const { return eigenschaften;  }
    EricZertifikatHandle getHandle() const { return verschlusselungsParameter.zertifikatHandle;  }
    const eric_verschluesselungs_parameter_t &getVerschlusselungsParameter() const { return verschlusselungsParameter; }

    EricZertifikat(const EricZertifikat &) = delete;

private:

    const Eric          &eric;
    const std::string    pfad;
    const std::string    pin;
    std::string          eigenschaften;
    eric_verschluesselungs_parameter_t verschlusselungsParameter;
};

#endif
