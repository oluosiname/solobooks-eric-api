/*
 * Bayerisches Landesamt fuer Steuern, Muenchen - Elster ERiC
 * (c) 2013, LfSt Bayern, Muenchen, IuK 111
 *
 * Name             : erictoolkit.h
 * Komponente       : erictoolkit
 *
 * Author           : 
 * Datum            :
 * Letzte Aenderung : 13. November 2018
 * Aenderung durch  : David Moch
 * SVN Revision     : 96716
 * SVN URL          :
 * SVN ID           :
 */
 
#ifndef ERICTOOLKIT_C_H_
#define ERICTOOLKIT_C_H_

/**
 * @file
 * @brief Bereitstellung von Prüffunktionen ohne Abhängigkeit zu anderen ERiC Bibliotheken.
 * 
 */

#if defined(__APPLE__)
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
		#define __IOS_SIMULATOR__
		#define __IOS__

		#define FILENAMES_CASE_SENSITIVE 0
		#define strcasecmp strcmp

    #elif TARGET_OS_IPHONE
		#define __IOS_DEVICE__
		#define __IOS__

		#define FILENAMES_CASE_SENSITIVE 0
		#define strcasecmp strcmp

    #elif TARGET_OS_MAC
		#define OS_X
		#define MAC_OS_X
		#define MAC_OS
		#ifndef PLATFORM_NAME
		#   define PLATFORM_NAME "Mac OS X"
		#endif

		#define FILENAMES_CASE_SENSITIVE 0
		#define strcasecmp strcmp

		#if defined(__amd64__) || defined(__x86_64__)
		#   define ARCH_AMD64
		#endif
    #endif
#endif


/* Windows Plattformen */

#if defined(WINDOWS) || defined(_WIN32) || defined(_MSC_VER)
#   define WINDOWS_OS
#   ifndef PLATFORM_NAME
#       define PLATFORM_NAME "Windows"
#   endif
#   if defined(_M_AMD64) || defined(_M_X64)
#      define ARCH_AMD64
#   endif
#endif

/* Linux Plattformen */

#if defined(linux) || defined(__linux__)

#define LINUX_OS

#   if defined(__amd64__) || defined(__x86_64__)
#      define ARCH_AMD64
#      ifndef PLATFORM_NAME
#          define PLATFORM_NAME "Linux/x86-64"
#      endif
#   else
#      ifndef PLATFORM_NAME
#          define PLATFORM_NAME "Linux" /* Generic Linux */
#      endif
#   endif

#if !defined(LINUX) && defined(LINUX_OS)
#   define LINUX 1
#endif

#endif

/*****************************************************/

#ifdef WINDOWS_OS
#  ifndef MSVC
#    define WINDOWS_GNUC true
#  endif
#endif


#ifdef WINDOWS_OS
#  define STDCALL               __stdcall
#  ifndef CDECL
#    define CDECL               __cdecl
#  endif

#else
#  if defined(__GNUC__) || defined(__GNUG__)
#    ifdef STDCALL
#      undef STDCALL
#    endif

#    ifdef CDECL
#      undef CDECL
#    endif

#    if defined(__i386__) && !defined(__IOS__) && !defined(ANDROID_OS)
#      define STDCALL           __attribute__((__stdcall__))
#    else
#      define STDCALL
#    endif
#    define CDECL               __attribute__((__cdecl__))
#  endif
#endif

#  if defined(_AIX)
#    ifdef STDCALL
#     undef STDCALL
#    endif
#    define STDCALL
#  endif

#if defined(WINDOWS_OS) && !defined(ERICTOOLKIT_DLL)
#define ETKAPI_DECL __declspec(dllimport)
#else
#define ETKAPI_DECL
#endif

#ifdef __cplusplus
extern "C"
{
#endif

     /**
     * @brief Die Bundesfinanzamtsnummer wird überprüft.
     *
     * Wird eine 13-stellige Steuernummer im
     * ELSTER-Steuernummernformat angegeben, so wird nur die
     * Bundesfinanzamtsnummer (= die ersten 4 Stellen der
     * 13-stelligen Steuernummer) geprüft.
     *
     * Eine Prüfung der Steuernummer selbst findet nicht statt
     * (hierfür EtkPruefeSteuernummer() verwenden).
     *
     * @param[in] steuernummer
     *            13-stellige Steuernummer im ELSTER Steuernummernformat bzw.
     *            4-stellige Bundesfinanzamtsnummer.
     * @return
     *         - ::ERIC_OK
     *         - ::ERIC_GLOBAL_BUFANR_UNBEKANNT: 
     *           Die Bundesfinanzamtsnummer ist unbekannt oder ungültig.
     *         - ::ERIC_GLOBAL_NULL_PARAMETER: 
     *           Es wurde keine Bundesfinanzamtsnummer übergeben (Parameter ist NULL).
     *
     * @see
     *      - EtkPruefeSteuernummer()
     *      - Pruefung_der_Steuer_und_Steueridentifikatsnummer.pdf, siehe \linkExt{https://www.elster.de/elsterweb/entwickler/infoseite/schnittstellenbeschreibungen,Entwicklerbereich} bei \linkExt{https://www.elster.de/eportal/start,ELSTER}.
     */
    ETKAPI_DECL int STDCALL EtkPruefeBuFaNummer(const char* steuernummer);

    /**
     * @brief Die @c bic wird auf Gültigkeit überprüft.
     *
     * Die Prüfung erfolgt in zwei Schritten:
     *
     * -# Formale Prüfung auf gültige Zeichen und richtige Länge
     * -# Prüfung, ob das Länderkennzeichen für BIC gültig ist.
     *
     * @param[in] bic Zeiger auf eine NULL-terminierte Zeichenkette.
     *
     * @return
     *         - ::ERIC_OK
     *         - ::ERIC_GLOBAL_BIC_FORMALER_FEHLER: Ungültige Zeichen, falsche Länge.
     *         - ::ERIC_GLOBAL_BIC_LAENDERCODE_FEHLER
     *         - ::ERIC_GLOBAL_NULL_PARAMETER: Parameter @c bic ist NULL.
     *
     * @see
     *      - @typeDokumentation{ERiC-Entwicklerhandbuch.pdf}, Kap. "BIC ISO-Ländercodes"
     *      - @typeDokumentation{ERiC-Entwicklerhandbuch.pdf}, Kap. "BIC-Prüfung"
     */
    ETKAPI_DECL int STDCALL EtkPruefeBIC(const char* bic);

    /**
     * @brief Überprüft ein @c Einheitswert-Aktenzeichen im ELSTER-Format auf Gültigkeit.
     *
     * @param[in] einheitswertAz
     *            Zeiger auf ein Einheitswert-Aktenzeichen im ELSTER-Format
     *
     * @return
     *  - ::ERIC_OK
     *  - ::ERIC_GLOBAL_EWAZ_UNGUELTIG
     *  - ::ERIC_GLOBAL_NULL_PARAMETER
     *
     */
    ETKAPI_DECL int STDCALL EtkPruefeEWAz(const char* einheitswertAz);

    /**
     * @brief Die @c iban wird auf Gültigkeit überprüft.
     *
     * Die Prüfung erfolgt in vier Schritten:
     *
     * -# Formale Prüfung auf gültige Zeichen und richtige Länge.
     * -# Prüfung, ob das Länderkennzeichen für IBAN gültig ist.
     * -# Prüfung, ob das länderspezifische Format gültig ist.
     * -# Prüfung, ob die Prüfziffer der IBAN gültig ist.
     *
     * @param[in] iban Zeiger auf eine NULL-terminierte Zeichenkette.
     *
     * @return
     *         - ::ERIC_OK
     *         - ::ERIC_GLOBAL_IBAN_FORMALER_FEHLER: Ungültige Zeichen, falsche Länge.
     *         - ::ERIC_GLOBAL_IBAN_LAENDERCODE_FEHLER
     *         - ::ERIC_GLOBAL_IBAN_LANDESFORMAT_FEHLER
     *         - ::ERIC_GLOBAL_IBAN_PRUEFZIFFER_FEHLER
     *         - ::ERIC_GLOBAL_NULL_PARAMETER: Parameter @c iban ist NULL.
     *
     * @see
     *      - @typeDokumentation{ERiC-Entwicklerhandbuch.pdf}, Kap. "IBAN - länderspezifische Formate"
     *      - @typeDokumentation{ERiC-Entwicklerhandbuch.pdf}, Kap. "IBAN-Prüfung"
     *
     */
    ETKAPI_DECL int STDCALL EtkPruefeIBAN(const char* iban);

    /**
     * @brief Die @c steuerId wird auf Gültigkeit überprüft.
     *        Formal korrekte Test Identifikationsnummern (beginnen mit der Ziffer 0) sind zulässig.
     *
     * @param[in] steuerId
     *            Steuer-Identifikationsnummer (IdNr)
     *
     * @return
     *         - ::ERIC_OK
     *         - ::ERIC_GLOBAL_IDNUMMER_UNGUELTIG
     *         - ::ERIC_GLOBAL_NULL_PARAMETER
     *
     * @see
     *         - @typeDokumentation{ERiC-Entwicklerhandbuch.pdf}, Kap. "Prüfung der Steueridentifikationsnummer (IdNr)"
     *         - @typeDokumentation{ERiC-Entwicklerhandbuch.pdf}, Kap. "Test-Steueridentifikationsnummer"
     *         - EtkPruefeSteuernummer()
     */
    ETKAPI_DECL int STDCALL EtkPruefeIdentifikationsMerkmal(const char* steuerId);

    /**
     * @brief Die @c steuernummer wird einschließlich Bundesfinanzamtsnummer
     *        auf formale Richtigkeit geprüft.
     *
     * Zur Prüfung der Bundesfinanzamtsnummer wird
     * EtkPruefeBuFaNummer() verwendet.
     *
     * @param[in] steuernummer
     *            NULL-terminierte 13-stellige Steuernummer im
     *            ELSTER-Steuernummernformat.
     *
     * @return
     *         - ::ERIC_OK
     *         - ::ERIC_GLOBAL_STEUERNUMMER_UNGUELTIG
     *         - ::ERIC_GLOBAL_NULL_PARAMETER
     *
     * @see
     *         - EtkPruefeBuFaNummer()
     *         - Pruefung_der_Steuer_und_Steueridentifikatsnummer.pdf, siehe \linkExt{https://www.elster.de/elsterweb/entwickler/infoseite/schnittstellenbeschreibungen,Entwicklerbereich} bei \linkExt{https://www.elster.de/eportal/start,ELSTER}.
     */
    ETKAPI_DECL int STDCALL EtkPruefeSteuernummer(const char* steuernummer);

     /**
     * @brief Die @c Wirtschafts-Identifikationsnummer (@c W-IdNr.) wird auf formale Gültigkeit geprüft.
     *
     * @param[in] wIdNr
     *            NULL-terminierte Wirtschafts-Identifikationsnummer
     *            mit oder ohne Unterscheidungsmerkmal.
     *
     * @return
     *         - ::ERIC_OK
     *         - ::ERIC_GLOBAL_IDNUMMER_UNGUELTIG
     *         - ::ERIC_GLOBAL_NULL_PARAMETER
     * @see
     *         - @typeDokumentation{ERiC-Entwicklerhandbuch.pdf}, Kap. "Prüfung der Wirtschafts-Identifikationsnummer (W-IdNr.)"
     */
    ETKAPI_DECL int STDCALL EtkPruefeWIdNr(const char* wIdNr);

    /**
    * @brief Abfragen der Produktversion des ERiCToolKit
    *
    * Die Produktversion wird in den bereitgestellten Speicher als NULL-terminierte C Zeichenkette zurückgegeben.
    * Der Speicher muss/darf von der Anwendung nicht freigegeben werden.
    *
    * @return NULL-terminierte C Zeichenkette.
	* 
    */
    ETKAPI_DECL const char* STDCALL EtkHoleProduktVersion();

    /**
    * @brief Abfragen der Dateiversion des ERiCToolKit
    *
    * Die Dateiversion wird in den bereitgestellten Speicher als NULL-terminierte C Zeichenkette zurückgegeben.
    * Der Speicher muss/darf von der Anwendung nicht freigegeben werden.
    *
    * @return NULL-terminierte C Zeichenkette.
    * 
    */
    ETKAPI_DECL const char* STDCALL EtkHoleDateiVersion();

#ifdef __cplusplus
}
#endif

#endif /*ERICTOOLKIT_C_H_*/
