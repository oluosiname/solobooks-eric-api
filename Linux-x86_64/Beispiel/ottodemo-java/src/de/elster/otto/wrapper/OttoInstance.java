package de.elster.otto.wrapper;

import de.elster.otto.wrapper.exception.OttoException;
import de.elster.otto.wrapper.exception.OttoExceptionFactory;
import de.elster.otto.wrapper.jna.OttoLibrary;
import de.elster.otto.wrapper.jna.handle.InstanceHandle;
import de.elster.otto.wrapper.jna.types.ErrorCode;

import java.nio.file.Path;
import java.util.Objects;

/**
 * Das Otto-Instanz-Wrapper-Objekt kapselt die wenig intuitive Handle-Klasse aus JNA und ermöglicht das Freigeben des Instanz-Objekts
 * mit try-with-resources.
 */

public class OttoInstance implements AutoCloseable {
    private final InstanceHandle instanceHandle;
    private final OttoLibrary library;

    /**
     * Solange eine Otto-Instanz lebt, muss auch der registrierte Callback leben. Deswegen wird
     * hier eine starke Referenz auf den Callback gespeichert.
     */
    @SuppressWarnings({"unused", "FieldCanBeLocal"})
    private final OttoLibrary.LogCallback logCallback;

    /**
     * Erzeuge ein Instanz-Handle-Wrapper-Objekt.
     *
     * @param library     Otto-Bibliothek
     * @param logPath     Pfad zu einem Ordner, in den Otto die Log-Dateien ablegen soll.
     * @param logCallback Log-Callback-Funktion oder null, wenn kein Callback genutzt werden soll.
     * @return Otto-Instanz-Wrapper-Objekt
     * @throws OttoException , wenn ein Fehler beim Erzeugen des Instanz-Wrapper-Objekts auftritt.
     */

    public static OttoInstance create(final OttoLibrary library, final Path logPath, final OttoLibrary.LogCallback logCallback) throws OttoException {
        Objects.requireNonNull(library);
        final InstanceHandle.ByRef byRef = new InstanceHandle.ByRef();
        final ErrorCode errorCode = library.OttoInstanzErzeugen(logPath, logCallback, null, byRef);
        OttoExceptionFactory.onErrorThrow(library, errorCode);
        return new OttoInstance(library, byRef.instanzHandle(), logCallback);
    }

    private OttoInstance(final OttoLibrary library, final InstanceHandle instanz, final OttoLibrary.LogCallback logCallback) {
        this.instanceHandle = instanz;
        this.library = library;
        this.logCallback = logCallback;
    }


    InstanceHandle instanceHandle() {
        return instanceHandle;
    }

    /**
     * Setzte die Otto-Proxy-Konfiguration
     *
     * @param proxyConfiguration Proxy-Konfiguration
     * @throws OttoException , wenn ein Fehler beim Setzen des Proxys auftritt.
     */
    public void setProxyConfiguration(final OttoLibrary.OttoProxyKonfiguration proxyConfiguration) throws OttoException {
        final ErrorCode errorCode = library.OttoProxyKonfigurationSetzen(instanceHandle(), proxyConfiguration);
        OttoExceptionFactory.onErrorThrow(library, errorCode);
    }

    /**
     * Gibt das Instanz-Objekt im Otto frei.
     *
     * @throws OttoException , wenn ein Fehler beim Freigeben der Instanz auftritt.
     */
    @Override
    public void close() throws OttoException {
        final ErrorCode errorCode = library.OttoInstanzFreigeben(instanceHandle);
        instanceHandle.setPointer(null);
        OttoExceptionFactory.onErrorThrow(library, errorCode);
    }
}
