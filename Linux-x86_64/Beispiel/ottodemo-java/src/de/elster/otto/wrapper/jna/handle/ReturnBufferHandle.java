package de.elster.otto.wrapper.jna.handle;

import com.sun.jna.PointerType;
import com.sun.jna.ptr.PointerByReference;

import java.util.Objects;


/**
 * JNA-Zeigertyp für Otto-Rückgabepufferzeiger
 */
public class ReturnBufferHandle extends PointerType {


    /**
     * Der Standardkonstruktor wird von JNA benötigt. Die
     * Initialisierung erfolgt mit der inneren Klasse {@link ReturnBufferHandle.ByRef} und der Funktion
     * {@link de.elster.otto.wrapper.jna.OttoLibrary#OttoRueckgabepufferErzeugen(InstanceHandle, ByRef)}.
     */
    @SuppressWarnings("unused")
    public ReturnBufferHandle() {
        super();
    }

    private ReturnBufferHandle(final ByRef ottoInstanzHandleRef) {
        super();
        if (Objects.nonNull(ottoInstanzHandleRef)) {
            super.setPointer(ottoInstanzHandleRef.getValue());
        }
    }

    /**
     * JNA-Zeigertyp für die Initialisierung eines Otto-Rückgabepufferzeigers.
     */

    public static class ByRef extends PointerByReference {
        /**
         * Standardkonstruktor
         */
        public ByRef() {
            super();
        }


        /**
         * Initialisiert einen Otto-Rückgabepufferzeiger
         *
         * @return Otto-Rückgabepufferzeiger
         */
        public ReturnBufferHandle returnBufferHandle() {
            return new ReturnBufferHandle(this);
        }


    }
}