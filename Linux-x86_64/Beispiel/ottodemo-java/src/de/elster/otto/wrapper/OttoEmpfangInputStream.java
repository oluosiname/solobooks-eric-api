package de.elster.otto.wrapper;

import de.elster.otto.wrapper.exception.OttoException;
import de.elster.otto.wrapper.exception.OttoExceptionFactory;
import de.elster.otto.wrapper.jna.OttoLibrary;
import de.elster.otto.wrapper.jna.handle.EmpfangHandle;
import de.elster.otto.wrapper.jna.types.ErrorCode;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.Objects;


/**
 * Otto-Empfang-InputStream-Wrapper-Klasse, die das Empfangen mit Otto in Java bequemer ermöglichen soll.
 */
public class OttoEmpfangInputStream extends InputStream implements AutoCloseable {

    private final EmpfangHandle empfangHandle;
    private final OttoLibrary library;
    private final OttoInstance instance;


    private OttoReturnBuffer returnBuffer;
    private ByteBuffer srcBuffer;

    private OttoEmpfangInputStream(final OttoLibrary library, final OttoInstance instance, final EmpfangHandle empfangHandle) {
        this.empfangHandle = empfangHandle;
        this.instance = instance;
        this.library = library;
    }

    /**
     * Erzeugt ein Empfang-InputStream-Wrapperobjekt, um die Daten bequem empfangen zu können.
     *
     * @param library      Otto-Bibliothek
     * @param instance     Otto-Instanz
     * @param certificate  Otto-Zertifikat
     * @param objectId     Objekt-ID des Objekts, das empfangen werden soll
     * @param herstellerId Elster-Hersteller-ID
     * @return Empfang-InputStream-Wrapper
     * @throws OttoException , wenn ein Fehler beim Erzeugen des Empfang-InputStreams auftritt.
     */

    public static OttoEmpfangInputStream open(final OttoLibrary library, final OttoInstance instance,
                                              final OttoCertificate certificate, final String objectId, final String herstellerId) throws
            OttoException {
        Objects.requireNonNull(library);
        Objects.requireNonNull(instance);
        Objects.requireNonNull(certificate);
        final EmpfangHandle.ByRef byRef = new EmpfangHandle.ByRef();
        final ErrorCode errorCode = library.OttoEmpfangBeginnen(instance.instanceHandle(), objectId,
                certificate.certificateHandle(), herstellerId, byRef);
        OttoExceptionFactory.onErrorThrow(library, errorCode);
        return new OttoEmpfangInputStream(library, instance, byRef.empfangHandle());
    }


    @Override
    public int read() throws IOException {

        // Wenn es der erste Durchlauf ist, dann initialisiere den Puffer.
        initBufferIfNecessary();

        // Wenn keine Daten mehr im Puffer sind, dann aktualisiere den Puffer.
        if (!srcBuffer.hasRemaining()) {
            // Prüfe, ob das Ende des Streams erreicht ist
            if (srcBuffer.capacity() == 0) {
                return -1;
            }
            updateBuffer();

            // Prüfe, ob das Ende des Streams erreicht ist
            if (srcBuffer.capacity() == 0) {
                return -1;
            }
        }

        return srcBuffer.get() & 0xFF;
    }

    private void initBufferIfNecessary() throws IOException {
        if (Objects.isNull(srcBuffer)) {
            updateBuffer();
        }
    }

    @Override
    public int read(final byte[] b, final int off, final int len) throws IOException {
        Objects.requireNonNull(b);
        WrapperUtils.checkFromIndexSize(off, len, b.length);
        final ByteBuffer bb = ByteBuffer.wrap(b, off, len);

        // Wenn es der erste Durchlauf ist, dann initialisiere den Puffer.
        initBufferIfNecessary();

        // Übertrage Daten
        while (bb.hasRemaining()) {
            // Wenn keine Daten mehr im Puffer sind, dann aktualisiere den Puffer.
            if (!srcBuffer.hasRemaining()) {
                if (srcBuffer.capacity() == 0) {
                    return -1;
                }
                updateBuffer();
                // Wenn das Ende des Streams erreicht ist, dann gebe die Anzahl der gelesenen Bytes zurück,
                // beziehungsweise -1, wenn keine Bytes gelesen wurden.
                if (srcBuffer.capacity() == 0) {
                    return bb.position() == 0 ? -1 : bb.position();
                }
            }
            bb.put(srcBuffer.get());
        }

        return bb.position();
    }

    private void updateBuffer() throws IOException {
        if (Objects.isNull(returnBuffer)) {
            try {
                returnBuffer = OttoReturnBuffer.create(library, instance);
            } catch (final OttoException e) {
                throw new IOException(e);
            }
        }

        final ErrorCode errorcode = library.OttoEmpfangFortsetzen(empfangHandle, returnBuffer.bufferHandle());
        WrapperUtils.onErrorThrowIOException(library, errorcode);
        final byte[] content = returnBuffer.content();
        srcBuffer = ByteBuffer.wrap(Objects.isNull(content) ? new byte[0] : content);
    }

    /**
     * Gibt das Empfangsobjekt im Otto frei.
     *
     * @throws IOException , wenn ein Fehler beim Freigeben des Empfangsobjekts auftritt.
     */
    @Override
    public void close() throws IOException {
        final ErrorCode errorcode = library.OttoEmpfangBeenden(empfangHandle);
        empfangHandle.setPointer(null);
        try {
            if (Objects.nonNull(returnBuffer)) {
                returnBuffer.close();
            }
            OttoExceptionFactory.onErrorThrow(library, errorcode);
        } catch (final OttoException e) {
            throw new IOException(e);
        }
    }
}
