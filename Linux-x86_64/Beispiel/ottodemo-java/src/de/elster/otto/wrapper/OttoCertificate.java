package de.elster.otto.wrapper;

import de.elster.otto.wrapper.exception.OttoException;
import de.elster.otto.wrapper.exception.OttoExceptionFactory;
import de.elster.otto.wrapper.jna.OttoLibrary;
import de.elster.otto.wrapper.jna.handle.CertificateHandle;
import de.elster.otto.wrapper.jna.types.ErrorCode;

import java.net.URI;
import java.nio.file.Path;
import java.util.Objects;

/**
 * Die Otto-Zertifikat-Wrapper-Klasse implementiert Autocloseable, sodass mit try-with-resources,
 * der im Otto allokierte Speicher wieder freigegeben werden kann.
 */

public class OttoCertificate implements AutoCloseable {

    private final CertificateHandle certificateHandle;
    private final OttoLibrary library;


    /**
     * Erzeugt ein Otto-Zertifikat-Wrapper-Objekt, dass mit try-with-resources wieder freigegeben werden kann.
     *
     * @param library         Otto-Bibliothek
     * @param instance        Otto-Instanz-Objekt
     * @param certificatePath Zertifikatspfad
     * @param password        Pin des Zertifikats
     * @return geöffnetes Zertifikatsobjekt
     * @throws OttoException , wenn beim Initialisieren des Zertifikatsobjekts im Otto ein Fehler auftritt.
     */

    public static OttoCertificate open(final OttoLibrary library, final OttoInstance instance, final Path certificatePath, final String password) throws OttoException {
        Objects.requireNonNull(library);
        Objects.requireNonNull(instance);
        final CertificateHandle.ByRef byRef = new CertificateHandle.ByRef();
        final ErrorCode errorCode = library.OttoZertifikatOeffnen(instance.instanceHandle(), certificatePath, password, byRef);
        OttoExceptionFactory.onErrorThrow(library, errorCode);
        final CertificateHandle certificate = byRef.certificateHandle();
        return new OttoCertificate(library, certificate);
    }

    /**
     * Erzeugt ein Otto-Zertifikat-Wrapper-Objekt zu einem eID-Client-Zertifikat, dass mit try-with-resources wieder freigegeben werden kann.
     *
     * @param library      Otto-Bibliothek
     * @param instance     Otto-Instanz-Objekt
     * @param eIDClientUrl eID-Client-Url
     * @return geöffnetes Zertifikatsobjekt
     * @throws OttoException , wenn beim Initialisieren des Zertifikatsobjekts im Otto ein Fehler auftritt.
     */

    public static OttoCertificate open(final OttoLibrary library, final OttoInstance instance, final URI eIDClientUrl) throws OttoException {
        Objects.requireNonNull(library);
        Objects.requireNonNull(instance);
        final CertificateHandle.ByRef byRef = new CertificateHandle.ByRef();
        final ErrorCode errorCode = library.OttoZertifikatOeffnen(instance.instanceHandle(), eIDClientUrl, null, byRef);
        OttoExceptionFactory.onErrorThrow(library, errorCode);
        final CertificateHandle certificate = byRef.certificateHandle();
        return new OttoCertificate(library, certificate);
    }

    private OttoCertificate(final OttoLibrary library, final CertificateHandle certificate) {
        this.certificateHandle = certificate;
        this.library = library;
    }


    CertificateHandle certificateHandle() {
        return certificateHandle;
    }

    /**
     * Schließt und gibt das Otto-Zertifikat-Objekt wieder frei.
     *
     * @throws OttoException , wenn es zu einem Fehler bei Freigeben des Zertifikats kommt.
     */
    @Override
    public void close() throws OttoException {
        final ErrorCode errorCode = library.OttoZertifikatSchliessen(certificateHandle());
        certificateHandle.setPointer(null);
        OttoExceptionFactory.onErrorThrow(library, errorCode);
    }
}
