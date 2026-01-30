package de.elster.otto.wrapper.jna.handle;

import com.sun.jna.Pointer;
import com.sun.jna.PointerType;
import com.sun.jna.ptr.PointerByReference;
import de.elster.otto.wrapper.jna.OttoLibrary;

import java.nio.file.Path;
import java.util.Objects;


/**
 * JNA-Zeigertyp für Otto-Instanzzeiger
 */

public class InstanceHandle extends PointerType {


    /**
     * Der Standardkonstruktor wird von JNA benötigt. Die
     * Initialisierung erfolgt mit der inneren Klasse {@link EmpfangHandle.ByRef} und der Funktion
     * {@link de.elster.otto.wrapper.jna.OttoLibrary#OttoInstanzErzeugen(Path, OttoLibrary.LogCallback, Pointer, ByRef)}.
     */
    @SuppressWarnings("unused")
    public InstanceHandle() {
        super();
    }

    private InstanceHandle(final ByRef ottoInstanzHandleRef) {
        super();
        if (Objects.nonNull(ottoInstanzHandleRef)) {
            super.setPointer(ottoInstanzHandleRef.getValue());
        }
    }


    /**
     * JNA-Zeigertyp für die Initialisierung eines Otto-Instanzzeigers.
     */
    public static class ByRef extends PointerByReference {

        /**
         * Standardkonstruktor
         */
        public ByRef() {
            super();
        }


        /**
         * Initialisiert einen Otto-Instanzzeiger
         *
         * @return Otto-Instanzzeiger
         */
        public InstanceHandle instanzHandle() {
            return new InstanceHandle(this);
        }
    }
}
