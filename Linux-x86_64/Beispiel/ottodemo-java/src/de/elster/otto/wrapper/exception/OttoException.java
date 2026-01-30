package de.elster.otto.wrapper.exception;

import de.elster.otto.wrapper.jna.types.ErrorCode;

/**
 * Eine Otto-Exception wird geworfen, wenn ein Fehler im Otto auftritt.
 */
public class OttoException extends Exception {

    /**
     * Fehlercode des Otto-Fehlers
     */
    private final ErrorCode errorCode;

    private static final long serialVersionUID = -3756339869987293753L;

    @Override
    public String toString() {
        return "OttoException{errorCode=" + errorCode + ", message='" + getMessage() + "'}";
    }

    /**
     * Konstruktor der OttoException
     *
     * @param errorCode Fehlercode, der die OttoException verursacht hat
     * @param message   Otto-Fehlernachricht, die zum Fehlercode im Otto gehört
     */
    public OttoException(final ErrorCode errorCode, final String message) {
        super(message);
        this.errorCode = errorCode;
    }

    /**
     * Gibt den Fehlercode der Otto-Exception zurück.
     *
     * @return Otto-Fehlercode
     */

    public ErrorCode errorCode() {
        return errorCode;
    }
}
