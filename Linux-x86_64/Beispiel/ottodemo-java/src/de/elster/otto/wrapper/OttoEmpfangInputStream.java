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
    private final OttoReturnBuffer returnBuffer;
    private ByteBuffer srcBuffer;

    private OttoEmpfangInputStream(final OttoLibrary library, final EmpfangHandle empfangHandle, final OttoReturnBuffer returnBuffer) {
        this.empfangHandle = empfangHandle;
        this.library = library;
        this.returnBuffer = Objects.requireNonNull(returnBuffer);
        this.srcBuffer = ByteBuffer.wrap(Objects.nonNull(returnBuffer.content()) ? returnBuffer.content() : new byte[0]);
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


        //Initialisiere Empfang-Objekt
        final EmpfangHandle.ByRef byRef = new EmpfangHandle.ByRef();
        final ErrorCode errorCode = library.OttoEmpfangBeginnen(instance.instanceHandle(), objectId,
                certificate.certificateHandle(), herstellerId, byRef);
        final EmpfangHandle empfangHandle = byRef.empfangHandle();
        OttoExceptionFactory.onErrorThrow(library, errorCode);

        OttoReturnBuffer returnBuffer = null;
        try {
            //Initialisiere Puffer-Objekt
            returnBuffer = OttoReturnBuffer.create(library, instance);

            // Starte Empfang
            final ErrorCode errorcode = library.OttoEmpfangFortsetzen(empfangHandle, returnBuffer.bufferHandle());
            OttoExceptionFactory.onErrorThrow(library, errorcode);

        } catch (final Exception e) {
            // Gebe Empfang- und Puffer-Objekt im Fehlerfall frei
            try (final OttoReturnBuffer ignoredBuffer = returnBuffer) {
                library.OttoEmpfangBeenden(empfangHandle);
                throw e;
            }

        }

        return new OttoEmpfangInputStream(library, empfangHandle, returnBuffer);
    }


    @Override
    public int read() throws IOException {
        if (srcBuffer.capacity() == 0) {
            return -1;
        }
        // Wenn keine Daten mehr im Puffer sind, dann aktualisiere den Puffer.
        if (!srcBuffer.hasRemaining()) {
            // Prüfe, ob das Ende des Streams erreicht ist
            updateBuffer();
            // Prüfe, ob das Ende des Streams erreicht ist
            if (srcBuffer.capacity() == 0) {
                return -1;
            }
        }
        return srcBuffer.get() & 0xFF;
    }


    @Override
    public int read(final byte[] b, final int off, final int len) throws IOException {
        Objects.requireNonNull(b);
        WrapperUtils.checkFromIndexSize(off, len, b.length);
        final ByteBuffer bb = ByteBuffer.wrap(b, off, len);
        // Übertrage Daten
        while (bb.hasRemaining() && srcBuffer.capacity() != 0) {
            // Übertrage Daten in den Puffer
            final int rem = Math.min(srcBuffer.remaining(), bb.remaining());
            if (rem > 0) {
                bb.put(srcBuffer.array(), srcBuffer.position(), rem);
                srcBuffer.position(srcBuffer.position() + rem);
            } else {
                updateBuffer();
            }
        }
        if (srcBuffer.capacity() == 0 && bb.position() == off) {
            return -1;
        }
        return bb.position() - off;
    }

    private void updateBuffer() throws IOException {
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
        try (final OttoReturnBuffer ignoredBuffer = returnBuffer) {
            final ErrorCode errorcode = library.OttoEmpfangBeenden(empfangHandle);
            empfangHandle.setPointer(null);
            OttoExceptionFactory.onErrorThrow(library, errorcode);
        } catch (final OttoException e) {
            throw new IOException(e);
        }
    }
}