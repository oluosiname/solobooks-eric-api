package de.elster.otto.wrapper.exception;

/**
 * Exceptionklasse für Fehler die im Java-Wrapper auftreten, z.B., wenn die
 * Otto-Bibliothek nicht gefunden werden kann.
 */
public class WrapperException extends Exception {
    private static final long serialVersionUID = 6262168713530251182L;

    /**
     * Konstruktor für eine WrapperException mit Fehlermeldung.
     *
     * @param msg Die Fehlermeldung
     */
    public WrapperException(final String msg) {
        super(msg);
    }

    /**
     * Konstruktor für eine WrapperException mit Fehler Textmeldung und der cause
     *
     * @param msg   Die Fehlermeldung
     * @param cause Fehlerursache
     */
    public WrapperException(final String msg, final Throwable cause) {
        super(msg, cause);
    }

}
