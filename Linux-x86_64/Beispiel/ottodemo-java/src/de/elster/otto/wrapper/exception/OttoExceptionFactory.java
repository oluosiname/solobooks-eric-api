package de.elster.otto.wrapper.exception;

import de.elster.otto.wrapper.jna.OttoLibrary;
import de.elster.otto.wrapper.jna.types.ErrorCode;
import de.elster.otto.wrapper.jna.types.Utf8Str;

import java.util.Objects;

/**
 * Factory-Objekt für das Erzeugen von OttoExceptions. Ruft die {@link OttoLibrary#OttoHoleFehlertext(int)}-Funktion auf,
 * um die Nachricht zum Fehlercode zu ermitteln.
 */
public class OttoExceptionFactory {
    private OttoExceptionFactory() {
    }

    /**
     * Wirft eine Otto-Exception, wenn der Otto-Fehlercode nicht {@link ErrorCode#OTTO_OK} ist. Dabei wird die Nachricht
     * zum Otto-Fehlercode ermittelt.
     *
     * @param library Otto-Bibliothek
     * @param error   Otto-Fehlercode
     * @throws OttoException bei einem Fehlercode, der nicht {@link ErrorCode#OTTO_OK}  ist.
     */
    public static void onErrorThrow(final OttoLibrary library, final ErrorCode error) throws OttoException {
        if (error != ErrorCode.OTTO_OK) {
            throw createOttoException(library, error);
        }
    }

    /**
     * Erzeugt eine Otto-Exception und ermittelt mit Otto den Fehlertext zu einem Otto-Fehlercode.
     *
     * @param library Otto-Bibliothek
     * @param error   Otto-Fehlercode
     * @return OttoException
     */

    public static OttoException createOttoException(final OttoLibrary library, final ErrorCode error) {
        final Utf8Str m = library.OttoHoleFehlertext(error.value());
        final String message = Objects.isNull(m) ? null : m.toString();
        return new OttoException(error, message);
    }
}
