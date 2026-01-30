package de.elster.otto.wrapper.jna.types;

/**
 * Das LogEbene-Enum beinhaltet alle Log-Ebenen im Otto-Logging.
 */
public enum LogEbene {
    /**
     * Debug-Log-Ebene
     */
    DEBUG,
    /**
     * Info-Log-Ebene
     */
    INFO,
    /**
     * Warnung-Log-Ebene
     */
    WARN,
    /**
     * Fehler-Log-Ebene
     */
    FEHLER;

    /**
     * Bildet die int-Repräsentation der Log-Ebene auf den Enum-Wert ab.
     *
     * @param logEbene int-Repräsentation
     * @return LogEbene-Enum-Wert
     */

    public static LogEbene map(final int logEbene) {
        switch (logEbene) {
            case 1:
                return DEBUG;
            case 2:
                return INFO;
            case 3:
                return WARN;
            case 4:
                return FEHLER;
            default:
                throw new IllegalArgumentException(String.format("int %d konnte nicht auf OttoLogEbene abgebildet werden!", logEbene));
        }
    }
}
