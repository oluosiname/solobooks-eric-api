#ifndef _ERICPUFFER_H_
#define _ERICPUFFER_H_

#pragma once

#include <string>
#include <vector>
#include "eric.h"

/** @brief Verwalten von ERiC-Rueckgabepuffern */
class EricPuffer
{

public:
    /** @brief Dieser Konstruktor erzeugt eine Instanz eines
     *  EricRueckgabepuffers und speichert das dabei erhaltene Handle.
     *  Der Destruktor von 'EricPuffer' gibt den EricRueckgabepuffer automatisch wieder frei.
     *
     *  @param eric
     *         Schnittstellenobjekt, das den ERiC kapselt und ueber das
     *         im Rahmen dieser Klasse die Verwaltung von Rueckgabepuffern
     *         durchgefuehrt wird.
     *         Das uebergebene Objekt muss mindestens so lange leben, wie
     *         die erzeugte Instanz der Klasse EricPuffer, da diese eine Referenz
     *         darauf haelt.
     */
    EricPuffer(const Eric& eric);

    /** Der Destruktor gibt den von diesem Objekt verwalteten
      * Rueckgabepuffer wieder frei. */
    ~EricPuffer();

    /** @brief Hole das Handle auf den von diesem Objekt verwalteten Rueckgabepuffer */
    EricRueckgabepufferHandle handle() const;

    /** @brief Hole Zeiger auf Inhalt des von diesem Objekt verwalteten Rueckgabepuffers */
    const char* inhalt() const;

    /** @brief Hole Anzahl der in den von diesem Objekt verwalteten Rueckgabepuffer geschriebenen Bytes */
    uint32_t laenge() const;


private:

    EricRueckgabepufferHandle mPufferHandle;
    const Eric& mEricAdapter;

    EricPuffer &operator=(const EricPuffer &); // Zuweisungen verboten

};

#endif
