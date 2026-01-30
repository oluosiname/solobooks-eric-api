#ifndef _ERIC_CALLBACKHANDLER_H_
#define _ERIC_CALLBACKHANDLER_H_

#include <eric_types.h>

class Eric;

/** @brief Stellt Callback-Funktionen mit Konsolenausgabe fuer den Einsatz mit ERiC bereit. */
class CallbackHandler
{
public:
    CallbackHandler(const Eric& myEric);

    virtual ~CallbackHandler();

    void globalerFortschritt(uint32_t id, uint32_t pos, uint32_t max) const;

    void fortschritt(uint32_t id, uint32_t pos, uint32_t max);

private:
    const Eric& eric;

    unsigned int letzteId;
    unsigned int letzteSpalte;
};

#endif //_ERIC_CALLBACKHANDLER_H_
