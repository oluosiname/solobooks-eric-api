#ifndef _ERIC_SYSTEM_H_
#define _ERIC_SYSTEM_H_


#ifdef _WIN32

#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <libloaderapi.h>

#   define PFAD_SEPARATOR '\\'
#   define NEW_LINE "\r\n"
#   define VER_BIB "dll"
#   define DYNBIB_PRAEFIX ""
#   define DYNBIB_SUFFIX  ".dll"
#   define ERIC_RESOLVE_LIB_PTR HINSTANCE
#   define ERIC_RESOLVE_GET_SYMBOL_FUNCTION ::GetProcAddress
#   define ERIC_RESOLVE_RESET_ERROR ::SetLastError(0)
#   define ERIC_RESOLVE_CLOSE_LIBRARY(handle) (::FreeLibrary(handle) == TRUE)
#   define GETCWD(x, y) _getcwd(x, y)
#   define GETLASTERROR() GetLastError()
#   ifdef MSVC
#       define ERIC_RESOLVE_LOAD_LIBRARY(handle) ::LoadLibraryExW(handle, nullptr, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32)
#   else
#       define ERIC_RESOLVE_LOAD_LIBRARY(handle) ::LoadLibraryEx(handle, nullptr, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32)
#   endif
    const char OPT_PRAEFIX = '/'; // Praefix fuer Kommandozeilenoptionen

// Definiere, dass Macro WINDOWS_MSVC, wenn der Microsoft Visual C++ Compiler verwendet wird
#   ifdef _MSC_VER
#   define WINDOWS_MSVC
#   endif // _MSC_Ver

#else

#   include <iconv.h>
#   include <dlfcn.h>
#   include <errno.h>
#   include <unistd.h>

#   define PFAD_SEPARATOR '/'
#   define NEW_LINE '\n'
#   define VER_BIB "lib"
#   if defined(MAC_OS_X) || defined(__IOS__)
#       define DYNBIB_SUFFIX ".dylib"
#       define DYNBIB_PRAEFIX "lib"
#   elif defined(__IOS__)
#       define DYNBIB_SUFFIX ".framework"
#       define DYNBIB_PRAEFIX ""
#   else
#       define DYNBIB_SUFFIX ".so"
#       define DYNBIB_PRAEFIX "lib"
#   endif
#   define ERIC_RESOLVE_LIB_PTR void*
#   define ERIC_RESOLVE_GET_SYMBOL_FUNCTION ::dlsym
#   define ERIC_RESOLVE_RESET_ERROR ::dlerror()
#   define ERIC_RESOLVE_LOAD_LIBRARY(handle) ::dlopen(handle, RTLD_NOW|RTLD_GLOBAL)
#   define ERIC_RESOLVE_CLOSE_LIBRARY(handle) (::dlclose(handle) == 0)
#   define GETCWD(x, y) getcwd(x, y)
#   define GETLASTERROR() errno
    const char OPT_PRAEFIX = '-'; // Praefix fuer Kommandozeilenoptionen

#endif

#if defined(__xlC__) && !defined(__clang__)
#   define MOVE_NO_XLC(i) (i)
#   define PREVIOUS(i) ((--i)++)
#else
#   define MOVE_NO_XLC(i) std::move(i)
#   define PREVIOUS(i) std::prev(i)
#endif

#include <cstdio>
#include <ostream>
#include <sstream>
#include <vector>
#include <utility>
#include <iterator>
#include <string>
#include <iostream>
#include <eric_types.h>


// Funktionen zum Verwalten der Kommandozeile.
    namespace System {

        /** @brief Parst eine Kommandozeile und ermittelt die gesetzten Kommandozeilenoptionen */
        class KommandozeilenParser {
        public:
            KommandozeilenParser();
            virtual ~KommandozeilenParser() {};

            // Parsefehler erzeugen eine Ausnahme!
#if defined(__xlC__) && !defined(__clang__)
            void parseKommandozeile(const std::vector<std::string>& argumente);
#else 
            void parseKommandozeile(std::vector<std::string>&& argumente);
#endif

            bool               getParseOk()         const { return parseOk; }
            static void        zeigeHilfe(std::ostream& ostream);

            const std::string& getAufrufPfad()          const { return aufrufPfad; }
            const std::string& getLogDir()              const { return logDir; }
            const std::string& getHomeDir()             const { return homeDir; }
            bool                getDatensatzSenden()     const { return datensatzSenden; }
            bool                getHilfeAnzeigen()       const { return hilfeAnzeigen; }
            bool                getDatenEntschluesseln() const { return datenEntschluesseln; };
            const std::string& getAusgabeDatei()        const { return ausgabeDatei; }
            EricTransferHandle  getTransferHandle()      const { return transferHandle; };
            bool                getHatTransferHandle()   const { return hatTransferHandle; }

            // Parameter mit Default-Werten
            const std::string& getZertifikatPfad()      const { return zertifikatPfad.empty() ? KommandozeilenParser::defaultZertifikatPfad : zertifikatPfad; }
            const std::string& getZertifikatPin()       const { return zertifikatPin.empty() ? KommandozeilenParser::defaultZertifikatPin : zertifikatPin; }
            const std::string& getDatensatzDatei()      const { return datensatzDatei.empty() ? KommandozeilenParser::defaultDatensatzDatei : datensatzDatei; }
            const std::string& getDatenartVersion()     const { return datenartVersion.empty() ? KommandozeilenParser::defaultDatenartVersion : datenartVersion; }

        private:
            bool                parseOk;

            // Kommandozeilenoptionen
            std::string         aufrufPfad;
            std::string         logDir;
            std::string         homeDir;
            std::string         zertifikatPfad;
            std::string         zertifikatPin;
            std::string         datensatzDatei;
            std::string         datenartVersion;
            bool                datensatzSenden;
            bool                hilfeAnzeigen;
            bool                datenEntschluesseln;
            std::string         ausgabeDatei;
            EricTransferHandle  transferHandle;
            bool                hatTransferHandle;

            // Default-Werte
            static const std::string defaultZertifikatPfad;
            static const std::string defaultZertifikatPin;
            static const std::string defaultDatensatzDatei;
            static const std::string defaultDatenartVersion;
        };



        /** @brief Ergaenzt einen Bibliotheksnamen zu einem Dateinamen
         *         mit systemspezifischem Praefix und
         *         systemspezifischer Dateinamenserweiterung.
         */
        std::string getBibliotheksDateiname(const std::string& bibliotheksName);

        /** @brief Prüft, ob der Pfad relativ ist */
        bool istPfadRelativ(const char* pfad);

        /** @brief Erzeugt einen Dateipfad aus einem Verzeichnispfad und einem Dateiamen */
        std::string dateiPfad(const std::string& verzeichnisPfad, const std::string& dateiName);

        /** @brief Holt das Arbeitsverzeichnis */
        bool getArbeitsverzeichnis(std::string& arbeitsVerzeichnis);

        /** @brief Schreibt Daten in eine Datei */
        bool schreibeDatei(const std::string& daten, const std::string& dateiName);

        /** @brief Gib eine Titelzeile aus */
        void titelZeile(const std::string& titel);

        /** @brief Konvertiert einen Datentyp in einen std::string
          *
          * @exception std::ios_base::failure, falls die Konvertierung fehl schlägt
          */
        template<typename T>
        std::string toString(const T& value)
        {
            std::ostringstream stringStream;
            stringStream.exceptions(std::ios::badbit | std::ios::failbit);
            stringStream.imbue(std::locale::classic());
            stringStream << value;
            return stringStream.str();
        }

#ifdef WINDOWS_MSVC

        namespace kod {

            /** @brief Gibt den Inhalt eines UTF-16-Wide-String als UTF-8-kodierten String zurück
             *
             * @exception std::runtime_error, wenn die Konvertierung fehl schlägt
             */
            std::string toUtf8(const std::wstring& wstr);


            /** @brief Gibt den Inhalt eines UTF-8-String als UTF-16-kodierten Wide-String zurück
             *
             * @exception std::runtime_error, wenn die Konvertierung fehlschlägt
             */
            std::wstring toUtf16(const std::string& str);

            /** @brief Gibt den Inhalt eines UTF-8-String als String in der aktuell eingestellten Windows-Zeichenkodierung zurück
             *
             * @exception std::runtime_error, wenn die Konvertierung fehlschlägt
             */
            std::string toWindowsZeichenKodierung(const std::string& str);

            /** @brief Objekt zur Verwaltung der aktuellen Konsolentextkodierung
            *
            * Liest die aktuelle Windows-Konsolenzeichenkodierung aus und setzt als aktuelle Windows-Konsolenzeichenkodierung UTF-8.
            * Bei der Zerstörung des Objekts wird die Konsolenzeichenkodierung auf ihre ursprüngliche Einstellung zurückgesetzt.
            */
            class KonsolenKodierungsManager {
              public:
                KonsolenKodierungsManager();
                ~KonsolenKodierungsManager();
                KonsolenKodierungsManager(const KonsolenKodierungsManager &) = delete;

              private:
                const unsigned int zeichenKodierung;
                const unsigned int ausgabeZeichenKodierung;
            };
        }
#endif
   
// Der IBM AIX-Compiler xlC Legacy unterstützt C++11 nur unvollständig.
// Daher werden für diesen Compiler ein paar Hilfsfunktionen benötigt.
#if defined(__xlC__) && !defined(__clang__)
        /** @brief Konvertiert einen Datentyp in einen std::string
          *
          * @exception std::ios_base::failure, falls die Konvertierung fehl schlägt
          */
        unsigned long toUlong(const std::string &s);

        /** @brief Instanzen dieser Templateklasse verwalten einen Zeiger,
          *        der automatisch freigeben wird, wenn die Instanz zerstört wird.
          */
        template <typename T>
        class FreePtr
        {
            public:
                FreePtr() : p(nullptr) {}
                FreePtr(T *p) : p(p) {}
                ~FreePtr() { delete p; }
                T *get() { return p; }
                const T *get() const { return p; }
                T &operator*() { return *p; }
                const T &operator*() const { return *p; }
                T *operator->() { return p; }
                const T *operator->() const { return p; }
                void reset(T *p) { delete this->p; this->p = p; }

                // Kopien unterbinden
                FreePtr(const FreePtr &) = delete;
                FreePtr& operator=(const FreePtr &) = delete;
                FreePtr(const FreePtr &&) = delete;
                FreePtr& operator=(const FreePtr &&) = delete;

            private:
                T *p;
        };
#endif
  }
#endif
