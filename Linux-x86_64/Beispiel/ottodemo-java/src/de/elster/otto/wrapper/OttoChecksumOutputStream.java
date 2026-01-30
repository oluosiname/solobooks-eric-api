package de.elster.otto.wrapper;

import com.sun.jna.Memory;
import de.elster.otto.wrapper.exception.OttoException;
import de.elster.otto.wrapper.exception.OttoExceptionFactory;
import de.elster.otto.wrapper.jna.OttoLibrary;
import de.elster.otto.wrapper.jna.handle.ChecksumHandle;
import de.elster.otto.wrapper.jna.types.ErrorCode;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.Objects;


/**
 * Otto-Prüfsummenobjekt-Wrapper-Klasse, die AutoCloseable implementiert, damit der Speicher mit try-with-resources
 * wieder freigegeben werden kann.
 */
public class OttoChecksumOutputStream extends OutputStream implements AutoCloseable {

    private final ChecksumHandle checksumHandle;
    private final OttoInstance instance;
    private final OttoLibrary library;
    private Memory memory;

    /**
     * Erzeugt ein Otto-Prüfsummenobjekt-Wrapper
     *
     * @param library  Otto-Bibliothek
     * @param instance Otto-Instanz
     * @return Otto-Prüfsummenobjekt-Wrapper
     * @throws OttoException , wenn ein Fehler im Otto beim Initialisieren des Prüfsummenobjekts auftritt.
     */

    public static OttoChecksumOutputStream open(final OttoLibrary library, final OttoInstance instance) throws OttoException {
        Objects.requireNonNull(library);
        Objects.requireNonNull(instance);
        final ChecksumHandle.ByRef byRef = new ChecksumHandle.ByRef();
        final ErrorCode errorCode = library.OttoPruefsummeErzeugen(instance.instanceHandle(), byRef);
        OttoExceptionFactory.onErrorThrow(library, errorCode);
        return new OttoChecksumOutputStream(library, instance, byRef.checksumHandle());
    }

    private OttoChecksumOutputStream(final OttoLibrary library, final OttoInstance instance, final ChecksumHandle checksumHandle) {
        this.library = library;
        this.instance = instance;
        this.checksumHandle = checksumHandle;
    }


    @Override
    public void write(final byte[] b, final int off, final int len) throws IOException {
        Objects.requireNonNull(b);
        WrapperUtils.checkFromIndexSize(off, len, b.length);
        memory = WrapperUtils.ensureMemoryCapacity(memory, len);
        memory.write(0, b, off, len);
        final ErrorCode errorCode = library.OttoPruefsummeAktualisieren(checksumHandle, memory, len);
        WrapperUtils.onErrorThrowIOException(library, errorCode);
    }

    @Override
    public void write(final int b) throws IOException {
        memory = WrapperUtils.ensureMemoryCapacity(memory, 1);
        memory.setByte(0, (byte) (0xff & b));
        final ErrorCode errorCode = library.OttoPruefsummeAktualisieren(checksumHandle, memory, 1);
        WrapperUtils.onErrorThrowIOException(library, errorCode);
    }

    /**
     * Berechnet eine signiert Prüfsumme, die zurückgegeben wird. Die Methode darf erst aufgerufen werden,
     * wenn alle Daten an den OutputStream übertragen wurden.
     *
     * @param certificate Otto-Zertifikat-Wrapper
     * @return Signatur-String der Prüfsumme
     * @throws OttoException , wenn ein Fehler beim Bilden der Signatur der Prüfsumme auftritt
     */


    public String sign(final OttoCertificate certificate) throws OttoException {
        Objects.requireNonNull(certificate);
        try (final OttoReturnBuffer returnBuffer = OttoReturnBuffer.create(library, instance)) {
            final ErrorCode errorCode = library.OttoPruefsummeSignieren(checksumHandle, certificate.certificateHandle(),
                    returnBuffer.bufferHandle());
            OttoExceptionFactory.onErrorThrow(library, errorCode);
            return new String(returnBuffer.content(), StandardCharsets.UTF_8);
        }
    }

    /**
     * Gibt das Prüfsummenobjekt im Otto frei.
     *
     * @throws IOException , wenn ein Fehler beim Freigeben der Prüfsumme auftritt
     */

    @Override
    public void close() throws IOException {
        try {
            if (Objects.nonNull(memory)) {
                memory.close();
            }
        } finally {
            final ErrorCode errorCode = library.OttoPruefsummeFreigeben(checksumHandle);
            checksumHandle.setPointer(null);
            WrapperUtils.onErrorThrowIOException(library, errorCode);
        }
    }
}
