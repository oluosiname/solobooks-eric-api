package de.elster.otto.wrapper.jna.handle;

import com.sun.jna.PointerType;
import com.sun.jna.ptr.PointerByReference;

import java.nio.file.Path;
import java.util.Objects;

/**
 * JNA-Zeigertyp für Otto-Zertifikatszeiger
 */
public class CertificateHandle extends PointerType {

    /**
     * Der Standardkonstruktor wird von JNA benötigt. Die
     * Initialisierung erfolgt mit der inneren Klasse {@link ByRef} und der Funktion
     * {@link de.elster.otto.wrapper.jna.OttoLibrary#OttoZertifikatOeffnen(InstanceHandle, Path, String, ByRef)}.
     */
    @SuppressWarnings("unused")
    public CertificateHandle() {
        super();
    }

    private CertificateHandle(final ByRef ottoZertifikatHandleRef) {
        super();
        if (Objects.nonNull(ottoZertifikatHandleRef)) {
            super.setPointer(ottoZertifikatHandleRef.getValue());
        }
    }

    /**
     * JNA-Zeigertyp für die Initialisierung eines Otto-Zertifikatszeigers.
     */
    public static class ByRef extends PointerByReference {

        /**
         * Standardkonstruktor
         */
        public ByRef() {
            super();
        }

        /**
         * Initialisiert einen Otto-Zertifikatszeiger
         *
         * @return Otto-Zertifikatszeiger
         */

        public CertificateHandle certificateHandle() {
            return new CertificateHandle(this);
        }
    }
}
