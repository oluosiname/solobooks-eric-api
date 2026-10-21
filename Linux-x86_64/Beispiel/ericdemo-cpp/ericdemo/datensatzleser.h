#ifndef _DATENSATZLESER_H_
#define _DATENSATZLESER_H_

#include <string>

/** @brief Liest XML-Datensaetze aus einer Datei. */
class Datensatzleser
{
public:
    /** @brief Lese XML-Datensatz aus einer Datei in den uebergebenen String. */
    static void lese(const std::string& dateiName, std::string& xmlDatensatz);

    Datensatzleser() = delete;
};

#endif //_DATENSATZLESER_H_
