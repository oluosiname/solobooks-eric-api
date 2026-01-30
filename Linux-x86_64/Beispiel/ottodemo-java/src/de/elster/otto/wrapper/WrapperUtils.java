package de.elster.otto.wrapper;

import com.sun.jna.Memory;
import de.elster.otto.wrapper.exception.OttoExceptionFactory;
import de.elster.otto.wrapper.jna.OttoLibrary;
import de.elster.otto.wrapper.jna.types.ErrorCode;

import java.io.IOException;
import java.util.Objects;

class WrapperUtils {
    private WrapperUtils() {
    }

    static Memory ensureMemoryCapacity(final Memory memory, final int capacity) {
        if (Objects.isNull(memory) || !memory.valid()) {
            return new Memory(Math.max(capacity, 1 << 13 /* 8 KiB*/));
        } else if (memory.size() < capacity) {
            memory.close();
            return new Memory(capacity);
        }
        return memory;
    }

    static void onErrorThrowIOException(final OttoLibrary library, final ErrorCode errorCode) throws IOException {
        if (errorCode != ErrorCode.OTTO_OK) {
            throw new IOException(OttoExceptionFactory.createOttoException(library, errorCode));
        }
    }

    /*
     *  Siehe Objects.checkFromIndexSize
     */
    static void checkFromIndexSize(final int fromIndex, final int size, final int length) {
        if (length < fromIndex + size || fromIndex < 0 || size < 0) {
            throw new IndexOutOfBoundsException();
        }
    }
}
