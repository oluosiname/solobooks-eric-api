package de.elster.otto.wrapper.jna.handle;

import com.sun.jna.PointerType;
import com.sun.jna.ptr.PointerByReference;

import java.util.Objects;


/**
 * JNA-Zeigertyp für Otto-Versandobjektzeiger
 */
public final class VersandHandle extends PointerType {


    /**
     * Der Standardkonstruktor wird von JNA benötigt. Die
     * Initialisierung erfolgt mit der inneren Klasse {@link VersandHandle.ByRef} und der Funktion
     * {@link de.elster.otto.wrapper.jna.OttoLibrary#OttoVersandBeginnen(InstanceHandle, String, String, ByRef)}.
     */
    @SuppressWarnings("unused")
    public VersandHandle() {
        super();
    }

    private VersandHandle(final ByRef ottoVersandHandleRef) {
        super();
        if (Objects.nonNull(ottoVersandHandleRef)) {
            super.setPointer(ottoVersandHandleRef.getValue());
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
         * Initialisiert einen Otto-Versandobjektzeiger
         *
         * @return Otto-Versandobjektzeiger
         */
        public VersandHandle versandHandle() {
            return new VersandHandle(this);
        }


    }
}
