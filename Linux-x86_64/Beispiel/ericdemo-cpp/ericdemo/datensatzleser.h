#ifndef _DATENSATZLESER_H_
#define _DATENSATZLESER_H_

#include <string>

/** @brief Liest XML-Datensaetze aus einer Datei. */
class Datensatzleser
{
public:
    Datensatzleser() {};
    ~Datensatzleser() {};

    /** @brief Lese XML-Datensatz aus einer Datei in den uebergebenen String. */
    void lese(const std::string& dateiName, std::string& xmlDatensatz);
};

#endif //_DATENSATZLESER_H_
