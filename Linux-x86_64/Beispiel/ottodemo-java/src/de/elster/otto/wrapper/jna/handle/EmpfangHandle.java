package de.elster.otto.wrapper.jna.handle;

import com.sun.jna.PointerType;
import com.sun.jna.ptr.PointerByReference;

import java.util.Objects;


/**
 * JNA-Zeigertyp für Otto-Empfangsobjektzeiger
 */
public final class EmpfangHandle extends PointerType {


    /**
     * Der Standardkonstruktor wird von JNA benötigt. Die
     * Initialisierung erfolgt mit der inneren Klasse {@link EmpfangHandle.ByRef} und der Funktion
     * {@link de.elster.otto.wrapper.jna.OttoLibrary#OttoEmpfangBeginnen(InstanceHandle, String, CertificateHandle, String, ByRef)}.
     */
    @SuppressWarnings("unused")
    public EmpfangHandle() {
        super();
    }

    private EmpfangHandle(final ByRef ottoEmpfangHandleRef) {
        super();
        if (Objects.nonNull(ottoEmpfangHandleRef)) {
            super.setPointer(ottoEmpfangHandleRef.getValue());
        }
    }


    /**
     * JNA-Zeigertyp für die Initialisierung eines Otto-Empfangsobjektzeigers.
     */
    public static class ByRef extends PointerByReference {

        /**
         * Standardkonstruktor
         */
        public ByRef() {
            super();
        }

        /**
         * Initialisiert einen Otto-Empfangsobjektzeiger
         *
         * @return Otto-Empfangsobjektzeiger
         */
        public EmpfangHandle empfangHandle() {
            return new EmpfangHandle(this);
        }

    }
}
