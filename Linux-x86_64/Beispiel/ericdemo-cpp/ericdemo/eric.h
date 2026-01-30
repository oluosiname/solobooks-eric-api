#ifndef _ERIC_ERICLIB_H_
#define _ERIC_ERICLIB_H_

#include <string>
#include <ericdef.h>
#include <eric_types.h>
#include <ericapi.h>

#include "resolve.h"


/** @brief Die Klasse 'Eric' kapselt die ERiC-Schnittstelle.
 *         Sie laedt und entlaedt die dynamische Bibliothek 'ericapi',
 *         ermittelt die Adressen der ERiC-Schnittstellenfunktionen und
 *         stellt Wrapper-Methoden zu deren Aufruf zur Verfuegung.
 */
class Eric
{
public:
    /**
     * @brief Erzeugt ein Schnittstellenobjekt fuer die ericapi und initialisiert es.
     *
     * @throw Anwendungsfehler
     *        Die Initialisierung ist fehlgeschlagen.
     *        Die Ursache kann ueber die Methode 'what()' an der Ausnahme abgefragt werden.
     */
    explicit Eric(const std::string &argHomeDir, const std::string &argLogDir);
    virtual ~Eric();

    /** @brief Entlaedt die ericapi. Alle weiteren Programmbibliotheken
     *         werden von der ericapi vorher automatisch mit Systemmitteln entladen.
     */
    void entladeEricApi();

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    int EricInitialisiere(const char *pluginPfad, const char *logPfad);

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    int EricBeende();

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    int EricRegistriereGlobalenFortschrittCallback(
        EricFortschrittCallback func,
        void *userData) const;

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    int EricRegistriereFortschrittCallback(
        EricFortschrittCallback func,
        void *userData) const;

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    int EricBearbeiteVorgang(
        const char* datenpuffer,
        const char* datenartVersion,
        uint32_t bearbeitungsFlags,
        const eric_druck_parameter_t *druckParameter,
        const eric_verschluesselungs_parameter_t *cryptoParameter,
        EricTransferHandle *transferHandle,
        EricRueckgabepufferHandle rueckgabeXmlPuffer,
        EricRueckgabepufferHandle serverantwortXmlPuffer) const;

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    int EricGetHandleToCertificate(
        EricZertifikatHandle * hToken,
        uint32_t *iInfoPinSupport,
        const char *pathToKeystore) const;

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    int EricCloseHandleToCertificate(
        EricZertifikatHandle hToken) const;

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    int EricDekodiereDaten(
        EricZertifikatHandle zertifikatHandle,
        const char * pin,
        const char * base64Eingabe,
        EricRueckgabepufferHandle rueckgabeXmlPuffer) const;

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    int EricHoleFehlerText(
        int fehlerkode,
        EricRueckgabepufferHandle rueckgabePuffer) const;

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    int EricPruefeSteuernummer(
        const char *steuernummer) const;

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    int EricSystemCheck() const;

    int EricEinstellungAlleZuruecksetzen(void) const;

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    int EricEinstellungSetzen(const char* name, const char* wert) const;

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    EricRueckgabepufferHandle EricRueckgabepufferErzeugen() const;

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    const char* EricRueckgabepufferInhalt(EricRueckgabepufferHandle handle) const;

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    uint32_t EricRueckgabepufferLaenge(EricRueckgabepufferHandle handle) const;

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    int EricRueckgabepufferFreigeben(EricRueckgabepufferHandle handle) const;

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    int EricCreateTH(
        const char * xml,
        const char * verfahren,
        const char * datenart,
        const char * vorgang,
        const char * testmerker,
        const char * herstellerId,
        const char * datenLieferant,
        const char * versionClient,
        const char * publicKey,
        EricRueckgabepufferHandle handle) const;

    /** @brief Wrapper fuer die gleichnamige ERiC API-Funktion. Siehe API-Referenz in der ERiC Dokumentation. */
    int EricHoleZertifikatEigenschaften(
        EricZertifikatHandle hToken,
        const char* pin,
        EricRueckgabepufferHandle rueckgabeXmlPuffer) const;

protected:
    /** @brief Laedt die ericapi dynamisch. Alle weiteren Programmbibliotheken
     *         werden von der ericapi automatisch mit Systemmitteln geladen.
     *
     *  @param apiVerzeichnis - Pfad des Verzeichnisses,
     *                          aus dem die ericapi geladen werden soll
     */
    bool ladeEricApi(const std::string &apiVerzeichnis);


private:
    Eric(const Eric &);
    Eric & operator= (const Eric &);

    bool istGeladen() const;

    Resolve::Library libEricApi;
};

#endif
