package de.elster.otto.wrapper;

import com.sun.jna.Memory;
import de.elster.otto.wrapper.exception.OttoException;
import de.elster.otto.wrapper.exception.OttoExceptionFactory;
import de.elster.otto.wrapper.jna.OttoLibrary;
import de.elster.otto.wrapper.jna.handle.VersandHandle;
import de.elster.otto.wrapper.jna.types.ErrorCode;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.Objects;


/**
 * Otto-Versand-Wrapper-Klasse, die AutoCloseable implementiert, damit der Speicher mit try-with-resources
 * wieder freigegeben werden kann.
 */
public class OttoVersandOutputStream extends OutputStream implements AutoCloseable {

    private final VersandHandle versandHandle;
    private final OttoInstance instance;
    private final OttoLibrary library;
    private Memory memory;

    /**
     * Erzeugt ein Otto-Versandobjekt. Das Objekt erbt von OutputStream zum bequemeren Upload von Dateien.
     *
     * @param library        Otto-Bibliothek
     * @param instance       Instanz
     * @param signedChecksum signierte Prüfsumme
     * @param herstellerId   Elster-Hersteller-ID
     * @return Otto-Versand-Wrapper-Klasse
     * @throws OttoException , wenn ein Fehler beim Erzeugen des Versandobjekts auftritt
     */

    public static OttoVersandOutputStream open(final OttoLibrary library, final OttoInstance instance, final String signedChecksum, final String herstellerId) throws
            OttoException {
        Objects.requireNonNull(library);
        Objects.requireNonNull(instance);
        final VersandHandle.ByRef byRef = new VersandHandle.ByRef();
        final ErrorCode errorCode = library.OttoVersandBeginnen(instance.instanceHandle(), signedChecksum, herstellerId,
                byRef);
        OttoExceptionFactory.onErrorThrow(library, errorCode);
        return new OttoVersandOutputStream(library, instance, byRef.versandHandle());
    }

    private OttoVersandOutputStream(final OttoLibrary library, final OttoInstance instance, final VersandHandle versandHandle) {
        this.library = library;
        this.instance = instance;
        this.versandHandle = versandHandle;
    }


    @Override
    public void write(final byte[] b, final int off, final int len) throws IOException {
        Objects.requireNonNull(b);
        WrapperUtils.checkFromIndexSize(off, len, b.length);
        memory = WrapperUtils.ensureMemoryCapacity(memory, len);
        memory.write(0, b, off, len);
        final ErrorCode errorCode = library.OttoVersandFortsetzen(versandHandle, memory, len);
        WrapperUtils.onErrorThrowIOException(library, errorCode);
    }

    @Override
    public void write(final int b) throws IOException {
        memory = WrapperUtils.ensureMemoryCapacity(memory, 1);
        memory.setByte(0, (byte) (0xff & b));
        final ErrorCode errorCode = library.OttoVersandFortsetzen(versandHandle, memory, 1);
        WrapperUtils.onErrorThrowIOException(library, errorCode);
    }


    /**
     * Schließt den Versandprozess ab und gibt die Objekt-UUID zurück, die zur hochgeladenen Datei gehört.
     *
     * @return Objekt-UUID
     * @throws OttoException , wenn ein Fehler beim Beenden des Uploads auftritt
     */

    public String finish() throws OttoException {
        try (final OttoReturnBuffer returnBuffer = OttoReturnBuffer.create(library, instance)) {
            final ErrorCode errorCode = library.OttoVersandAbschliessen(versandHandle, returnBuffer.bufferHandle());
            OttoExceptionFactory.onErrorThrow(library, errorCode);
            return new String(returnBuffer.content(), StandardCharsets.UTF_8);
        }
    }

    /**
     * Gibt das Otto-Versandobjekt frei.
     *
     * @throws IOException , wenn ein Fehler beim Freigeben des Versandobjekts aufritt.
     */

    @Override
    public void close() throws IOException {
        try {
            if (Objects.nonNull(memory)) {
                memory.close();
            }
        } finally {
            final ErrorCode errorCode = library.OttoVersandBeenden(versandHandle);
            versandHandle.setPointer(null);
            WrapperUtils.onErrorThrowIOException(library, errorCode);
        }
    }
}
