#ifndef _ANWENDUNGSFEHLER_H_
#define _ANWENDUNGSFEHLER_H_

#include <stdexcept>

/** @brief Fehler bei der Ausfuehrung der Beispielanwendung. 
 *
 * Falls ein Fehler in den Anwendungsklassen 
 * auftritt wird eine Ausnahme dieser Klasse geworfen.
 */
class Anwendungsfehler : public std::runtime_error
{
public:
    /** @brief Konstruktor mit Fehlertext. */
    Anwendungsfehler(const std::string& what) 
        : std::runtime_error(what) {}
};

#endif
