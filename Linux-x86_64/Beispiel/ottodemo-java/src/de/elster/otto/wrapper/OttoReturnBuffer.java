package de.elster.otto.wrapper;

import com.sun.jna.Pointer;
import de.elster.otto.wrapper.exception.OttoException;
import de.elster.otto.wrapper.exception.OttoExceptionFactory;
import de.elster.otto.wrapper.jna.OttoLibrary;
import de.elster.otto.wrapper.jna.handle.ReturnBufferHandle;
import de.elster.otto.wrapper.jna.types.ErrorCode;

import java.util.Objects;

/**
 * Otto-Rückgabepuffer-Objekt, dass das zuverlässige Freigeben des Objekts mit try-with-resources vereinfachen soll.
 */

public class OttoReturnBuffer implements AutoCloseable {

    private final OttoLibrary library;
    private final ReturnBufferHandle bufferHandle;

    /**
     * Erzeuge ein Otto-Rückgabepuffer-Objekt.
     *
     * @param library  Otto-Bibliothek
     * @param instance Instanz
     * @return Otto-Rückgabepuffer-Objekt
     * @throws OttoException , wenn ein Fehler beim Erzeugen des Rückgabepuffer-Objekts auftritt.
     */

    public static OttoReturnBuffer create(final OttoLibrary library, final OttoInstance instance) throws OttoException {
        Objects.requireNonNull(library);
        Objects.requireNonNull(instance);
        final ReturnBufferHandle.ByRef byRef = new ReturnBufferHandle.ByRef();
        final ErrorCode errorCode = library.OttoRueckgabepufferErzeugen(instance.instanceHandle(), byRef);
        OttoExceptionFactory.onErrorThrow(library, errorCode);
        final ReturnBufferHandle returnBufferHandle = byRef.returnBufferHandle();
        return new OttoReturnBuffer(library, returnBufferHandle);
    }

    private OttoReturnBuffer(final OttoLibrary library, final ReturnBufferHandle bufferHandle) {
        this.bufferHandle = bufferHandle;
        this.library = library;
    }

    /**
     * Gibt den Inhalt des Puffers zurück.
     *
     * @return Gibt den Inhalt des Puffers zurück.
     */

    public byte[] content() {
        final Pointer pointer = library.OttoRueckgabepufferInhalt(bufferHandle());
        if (Objects.isNull(pointer) || pointer == Pointer.NULL) {
            return new byte[0];
        }
        final long size = library.OttoRueckgabepufferGroesse(bufferHandle());
        if (size > Integer.MAX_VALUE) {
            throw new IllegalStateException("ReturnBuffer-Größe ist zu groß für ein Integer!");
        }
        return pointer.getByteArray(0, (int) size);
    }


    ReturnBufferHandle bufferHandle() {
        return bufferHandle;
    }

    /**
     * Gibt das Rückgabepuffer-Objekt im Otto frei.
     *
     * @throws OttoException , wenn ein Fehler beim Freigeben des Rückgabepuffer-Objekts auftritt.
     */

    @Override
    public void close() throws OttoException {
        final ErrorCode errorCode = library.OttoRueckgabepufferFreigeben(bufferHandle());
        bufferHandle.setPointer(null);
        OttoExceptionFactory.onErrorThrow(library, errorCode);
    }
}
