package de.elster.otto.wrapper.jna.handle;

import com.sun.jna.PointerType;
import com.sun.jna.ptr.PointerByReference;

import java.util.Objects;

/**
 * JNA-Zeigertyp für Otto-Prüfsummenobjektzeiger
 */
public final class ChecksumHandle extends PointerType {

    /**
     * Der Standardkonstruktor wird von JNA benötigt. Die
     * Initialisierung erfolgt mit der inneren Klasse {@link ChecksumHandle.ByRef} und der Funktion
     * {@link de.elster.otto.wrapper.jna.OttoLibrary#OttoPruefsummeErzeugen(InstanceHandle, ByRef)}.
     */
    @SuppressWarnings("unused")
    public ChecksumHandle() {
        super();
    }

    private ChecksumHandle(final ByRef ottoPruefsummeHandle) {
        super();
        if (Objects.nonNull(ottoPruefsummeHandle)) {
            super.setPointer(ottoPruefsummeHandle.getValue());
        }
    }

    /**
     * JNA-Zeigertyp für die Initialisierung eines Otto-Prüfsummenobjektzeigers.
     */
    public static class ByRef extends PointerByReference {

        /**
         * Standardkonstruktor
         */
        public ByRef() {
            super();
        }

        /**
         * Initialisiert einen Otto-Prüfsummenobjektzeiger
         *
         * @return Otto-Prüfsummenobjekzeiger
         */
        public ChecksumHandle checksumHandle() {
            return new ChecksumHandle(this);
        }
    }

}
