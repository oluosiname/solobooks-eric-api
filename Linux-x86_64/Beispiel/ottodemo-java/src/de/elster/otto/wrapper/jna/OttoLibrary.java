package de.elster.otto.wrapper.jna;

import com.sun.jna.*;
import com.sun.jna.Structure.FieldOrder;
import de.elster.otto.wrapper.jna.handle.*;
import de.elster.otto.wrapper.jna.handle.InstanceHandle;
import de.elster.otto.wrapper.jna.types.ErrorCode;
import de.elster.otto.wrapper.jna.types.LogEbene;
import de.elster.otto.wrapper.jna.types.Utf8Str;

import java.net.URI;
import java.nio.file.Path;

/**
 * JNA-Wrapper für Otto basierend auf der Header-Datei ottoapi.h
 */
@SuppressWarnings("java:S100")
public interface OttoLibrary extends Library {

    /**
     * Proxy-Konfigurationsstruktur zum Setzen der Otto-Proxy-Konfiguration.
     */
    @FieldOrder({"version", "url", "benutzerName", "benutzerPasswort", "authentifizierungsMethode"})
    @SuppressWarnings({"java:S2160", "java:S1104"})
    class OttoProxyKonfiguration extends Structure {

        /**
         * Standardkonstruktor für OttoProxyKonfiguration wird von JNA benötigt.
         */
        @SuppressWarnings("unused")
        public OttoProxyKonfiguration() {
        }

        /**
         * Konstruktor zur Initialisierung der Proxy-Konfigurationsstruktur.
         *
         * @param url                       URL des Proxies
         * @param benutzerName              Benutzername für den Proxy-Server
         * @param benutzerPasswort          Passwort für den Proxy-Server
         * @param authentifizierungsMethode Authentifizierungsprotokoll
         */
        public OttoProxyKonfiguration(final String url, final String benutzerName, final String benutzerPasswort, final String authentifizierungsMethode) {
            this.url = url;
            this.benutzerName = benutzerName;
            this.benutzerPasswort = benutzerPasswort;
            this.authentifizierungsMethode = Utf8Str.create(authentifizierungsMethode);
        }

        /**
         * Version der Proxykonfiguration
         */
        public int version = 1;

        /**
         * Proxy-Url
         */
        public String url;

        /**
         * Benutzername
         */
        public String benutzerName;

        /**
         * Passwort
         */
        public String benutzerPasswort;

        /**
         * String der Authentifizierungsmethode
         */
        public Utf8Str authentifizierungsMethode;
    }

    /**
     * Interface für den Callback des Otto-Logs
     */
    @FunctionalInterface
    interface LogCallback extends Callback {
        /**
         * Ruft die andere {@link LogCallback#logCallback}-Funktion auf.
         *
         * @param instanzId     Instanz-ID
         * @param logZeitpunkt  Zeitstempel
         * @param logEbene      Log-Ebene
         * @param logNachricht  Log-Nachricht
         * @param benutzerdaten Pointer zu Benutzerdaten
         */
        @SuppressWarnings("unused")
        default void callback(final Utf8Str instanzId, final Utf8Str logZeitpunkt, final LogEbene logEbene, final Utf8Str logNachricht, final Pointer benutzerdaten) {
            logCallback(instanzId.toString(), logZeitpunkt.toString(), logEbene, logNachricht.toString());
        }

        /**
         * LogCallback-Interface-Methode, die implementiert werden muss
         *
         * @param instanzId    Instanz-ID
         * @param logZeitpunkt Zeitstempel
         * @param logEbene     Log-Ebene
         * @param logNachricht Log-Nachricht
         */

        void logCallback(String instanzId, String logZeitpunkt, LogEbene logEbene, String logNachricht);
    }

    /**
     * Erzeugt eine Otto-Instanz.
     *
     * @param logPfad                  Pfad zur Log-Datei
     * @param logCallback              Callback zum Log
     * @param logCallbackBenutzerdaten Pointer zu den Benutzerdaten des Callbacks
     * @param instanz                  Otto-Instanz-Handle-Referenz
     * @return Fehlercode
     */
    ErrorCode OttoInstanzErzeugen(Path logPfad, LogCallback logCallback, Pointer logCallbackBenutzerdaten, InstanceHandle.ByRef instanz);

    /**
     * Gibt die Otto Instanz frei.
     *
     * @param instanz Otto-Instanz-Handle
     * @return Fehlercode
     */
    ErrorCode OttoInstanzFreigeben(InstanceHandle instanz);

    /**
     * Öffnet ein Otto-Zertifikat.
     *
     * @param instanz             Otto-Instanz-Handle
     * @param zertifikatsPfad     Pfad zum Zertifikat
     * @param zertifikatsPasswort Passwort des Zertifikats
     * @param zertifikat          Zertifikat-Handle-Referenz
     * @return Fehlercode
     */
    ErrorCode OttoZertifikatOeffnen(InstanceHandle instanz, Path zertifikatsPfad, String zertifikatsPasswort, CertificateHandle.ByRef zertifikat);

    /**
     * Öffnet ein Otto-eID-Client-Zertifikat.
     *
     * @param instanz             Otto-Instanz-Handle
     * @param eIDClientUrl        eID-Client-Url
     * @param zertifikatsPasswort Passwort des Zertifikat
     * @param zertifikat          Zertifikat-Handle-Referenz
     * @return Fehlercode
     */
    ErrorCode OttoZertifikatOeffnen(InstanceHandle instanz, URI eIDClientUrl, String zertifikatsPasswort, CertificateHandle.ByRef zertifikat);

    /**
     * Schließt ein Otto-Zertifikat.
     *
     * @param zertifikat Zertifikat-Handle
     * @return Fehlercode
     */
    ErrorCode OttoZertifikatSchliessen(CertificateHandle zertifikat);

    /**
     * Beginnt den Empfang von Otto.
     *
     * @param instanz      Otto-Instanz-Handle
     * @param objektId     ObjektID des zu übertragenen Objekts
     * @param zertifikat   Zertifikat-Handle
     * @param herstellerId Die zu verwendende Hersteller-Identifikationsnummer
     * @param empfang      Empfangsobjekt-Handle-Referenz
     * @return Fehlercode
     */
    ErrorCode OttoEmpfangBeginnen(InstanceHandle instanz, String objektId, CertificateHandle zertifikat, String herstellerId, EmpfangHandle.ByRef empfang);

    /**
     * Erzeugt einen Rückgabepuffer
     *
     * @param instanz Otto-Instanz-Handle
     * @param puffer  Rückgabepuffer-Handle-Referenz
     * @return Fehlercode
     */
    ErrorCode OttoRueckgabepufferErzeugen(InstanceHandle instanz, ReturnBufferHandle.ByRef puffer);

    /**
     * Gibt den Rückgabepuffer frei.
     *
     * @param puffer Rückgabepuffer-Handle
     * @return Fehlercode
     */
    ErrorCode OttoRueckgabepufferFreigeben(ReturnBufferHandle puffer);

    /**
     * Setzt den Empfang fort.
     *
     * @param empfang    Empfangsobjekt-Handle
     * @param datenBlock Rückgabepuffer-Handle
     * @return Fehlercode
     */
    ErrorCode OttoEmpfangFortsetzen(EmpfangHandle empfang, ReturnBufferHandle datenBlock);

    /**
     * Gibt die Größe des Rückgabepuffers zurück.
     *
     * @param puffer Rückgabepuffer-Handle
     * @return long
     * Die Größe des Rückgabepuffers
     */
    long OttoRueckgabepufferGroesse(ReturnBufferHandle puffer);

    /**
     * Gibt den Pointer zum Rückgabepuffer Inhalt zurück.
     *
     * @param puffer Rückgabepuffer-Handle
     * @return Pointer zum Rückgabepufferinhalt
     */
    Pointer OttoRueckgabepufferInhalt(ReturnBufferHandle puffer);

    /**
     * Beendet den Otto Empfang.
     *
     * @param empfang Empfangsobjekt-Handle
     * @return Fehlercode
     */
    ErrorCode OttoEmpfangBeenden(EmpfangHandle empfang);

    /**
     * Erzeugt eine Prüfsumme für die Otto-Instanz.
     *
     * @param instanz    Otto-Instanz-Handle
     * @param pruefsumme Prüfsummenobjekt-Handle-Referenz
     * @return Fehlercode
     */
    ErrorCode OttoPruefsummeErzeugen(InstanceHandle instanz, ChecksumHandle.ByRef pruefsumme);

    /**
     * Aktualisiert die Prüfsumme für den übergebenen Datenblock.
     *
     * @param pruefsumme        Prüfsummenobjekt-Handle
     * @param datenBlock        Datenblock
     * @param datenBlockGroesse Größe des Datenblocks
     * @return Fehlercode
     */
    ErrorCode OttoPruefsummeAktualisieren(ChecksumHandle pruefsumme, Memory datenBlock, long datenBlockGroesse);

    /**
     * Signiert die Prüfsumme.
     *
     * @param pruefsumme Prüfsummenobjekt-Handle
     * @param zertifikat Zertifikat-Handle
     * @param puffer     Rückgabepuffer-Handle
     * @return Fehlercode
     */
    ErrorCode OttoPruefsummeSignieren(ChecksumHandle pruefsumme, CertificateHandle zertifikat, ReturnBufferHandle puffer);

    /**
     * Gibt die aktuelle Prüfsumme frei.
     *
     * @param pruefsumme Prüfsummenobjekt-Handle
     * @return Fehlercode
     */
    ErrorCode OttoPruefsummeFreigeben(ChecksumHandle pruefsumme);

    /**
     * Beginnt den Versand an Otto mit signierter Prüfsumme.
     *
     * @param instanz             Otto-Instanz-Handle
     * @param signiertePruefsumme String der signierten Prüfsumme
     * @param herstellerId        Die zu verwendende Hersteller Identifikationsnummer
     * @param versand             Versandobjekt-Handle-Referenz
     * @return Fehlercode
     */
    ErrorCode OttoVersandBeginnen(InstanceHandle instanz, String signiertePruefsumme, String herstellerId, VersandHandle.ByRef versand);

    /**
     * Setzet den Versand an Otto fort.
     *
     * @param versand           Versandobjekt-Handle
     * @param datenBlock        Datenblock
     * @param datenBlockGroesse Größe des Datenblocks
     * @return Fehlercode
     */
    ErrorCode OttoVersandFortsetzen(VersandHandle versand, Memory datenBlock, long datenBlockGroesse);

    /**
     * Schließt den Versand an Otto ab.
     *
     * @param versand  Versandobjekt-Handle
     * @param objektId Objekt-ID
     * @return Fehlercode
     */
    ErrorCode OttoVersandAbschliessen(VersandHandle versand, ReturnBufferHandle objektId);

    /**
     * Beendet den Versand an Otto.
     *
     * @param versand Versandobjekt-Handle
     * @return Fehlercode
     */
    ErrorCode OttoVersandBeenden(VersandHandle versand);


    /**
     * Holt den Fehlertext zu einem Otto-Statuscode
     *
     * @param statuscode Statuscode
     * @return Fehlertext oder null
     */
    Utf8Str OttoHoleFehlertext(int statuscode);


    /**
     * Setzt die Proxykonfiguration für die Kommunikation mit Otto.
     *
     * @param instanz            Otto-Instanz-Handle
     * @param proxyKonfiguration Otto-ProxyKonfiguration zur Konfiguration des Proxys
     * @return Fehlercode
     */
    ErrorCode OttoProxyKonfigurationSetzen(InstanceHandle instanz, OttoProxyKonfiguration proxyKonfiguration);


    /**
     * Wird für Structures benötigt, wenn keine Library über JNA geladen worden ist.
     */
    @SuppressWarnings("unused")
    TypeMapper TYPE_MAPPER = new OttoTypeMapper();
}
