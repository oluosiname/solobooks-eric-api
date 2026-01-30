package de.elster.otto.wrapper.jna;

import com.sun.jna.*;
import de.elster.otto.wrapper.jna.types.ErrorCode;
import de.elster.otto.wrapper.jna.types.LogEbene;
import de.elster.otto.wrapper.jna.types.Utf8Str;

import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.text.Normalizer;
import java.util.Objects;

/**
 * Die Klasse übernimmt das Abbilden von JNA—fremden Typen, auf Typen, die JNA auf native Objekte abbilden kann.
 * Hier sind die Konverter auf JNA-fremde Typen zentral definiert und die Umwandlung wird
 * von JNA übernommen, wenn die Klasse beim Laden der Bibliothek als TypeMapper registriert wird.
 */

class OttoTypeMapper extends DefaultTypeMapper {

    private static final String TYPE_CONVERSION_ERROR_MESSAGE = "Kann Typen %s nicht in %s konvertieren!";

    /**
     * Initialisiert den Otto-Type-Mapper mit Konvertern für JNA-fremde Typen, die für
     * die Otto-Api benötigt werden.
     */
    public OttoTypeMapper() {
        super();
        addFromNativeConverter(LogEbene.class, new OttoLogEbeneConverter());
        addFromNativeConverter(ErrorCode.class, new ErrorCodeConverter());
        addToNativeConverter(Path.class, new PathConverter());
        addTypeConverter(Utf8Str.class, new Utf8CStrConverter());
        addToNativeConverter(URI.class, new UriTypeConverter());
    }

    private static class Utf8CStrConverter implements TypeConverter {

        @Override
        public Object fromNative(final Object nativeValue, final FromNativeContext context) {
            if (Objects.isNull(nativeValue)) {
                return null;
            }
            if (!(nativeValue instanceof Pointer)) {
                throw new IllegalArgumentException(String.format(TYPE_CONVERSION_ERROR_MESSAGE, nativeValue.getClass(), Utf8Str.class));
            }
            final Pointer ptr = (Pointer) nativeValue;
            final String str = ptr.getString(0, StandardCharsets.UTF_8.name());
            return Utf8Str.create(str);
        }

        @Override
        public Object toNative(final Object value, final ToNativeContext context) {
            if (Objects.isNull(value)) {
                return null;
            }
            if (!(value instanceof Utf8Str)) {
                throw new IllegalArgumentException(String.format(TYPE_CONVERSION_ERROR_MESSAGE, value.getClass(), Utf8Str.class));
            }
            final Utf8Str str = (Utf8Str) value;
            final byte[] data = str.toString().getBytes(StandardCharsets.UTF_8);
            final Memory mem = new Memory(data.length + 1L);
            mem.write(0, data, 0, data.length);
            mem.setByte(data.length, (byte) 0);
            return mem;
        }

        @Override
        public Class<Pointer> nativeType() {
            return Pointer.class;
        }
    }

    private static class PathConverter implements ToNativeConverter {

        @Override
        public Object toNative(final Object value, final ToNativeContext toNativeContext) {
            if (Objects.isNull(value)) {
                return null;
            }
            if (!(value instanceof Path)) {
                throw new IllegalArgumentException(String.format(TYPE_CONVERSION_ERROR_MESSAGE, value.getClass(), Path.class));
            }
            String path = value.toString();

            if (Platform.isMac()) {
                // Pfade auf macOS benötigen immer UTF-8
                // macOS benutzt bei HFS+ die "decomposed form"
                // von UTF-8
                path = Normalizer.normalize(path, Normalizer.Form.NFD);
            }
            return path;
        }

        public Class<String> nativeType() {
            return String.class;
        }
    }

    private static class OttoLogEbeneConverter implements FromNativeConverter {

        @Override
        public Object fromNative(final Object nativeValue, final FromNativeContext context) {
            if (Objects.isNull(nativeValue)) {
                return null;
            }
            if (!(nativeValue instanceof Integer)) {
                throw new IllegalArgumentException(String.format(TYPE_CONVERSION_ERROR_MESSAGE, nativeValue.getClass(), LogEbene.class));
            }
            final Integer lgEbene = (Integer) nativeValue;
            return LogEbene.map(lgEbene);
        }

        @Override
        public Class<Integer> nativeType() {
            return Integer.class;
        }
    }

    private static class ErrorCodeConverter implements FromNativeConverter {

        @Override
        public Object fromNative(final Object nativeValue, final FromNativeContext context) {
            if (Objects.isNull(nativeValue)) {
                return null;
            }
            if (!(nativeValue instanceof Integer)) {
                throw new IllegalArgumentException(String.format(TYPE_CONVERSION_ERROR_MESSAGE, nativeValue.getClass(), LogEbene.class));
            }
            final Integer errorCode = (Integer) nativeValue;
            return ErrorCode.getErrorCode(errorCode);
        }

        @Override
        public Class<Integer> nativeType() {
            return Integer.class;
        }
    }

    private static class UriTypeConverter implements ToNativeConverter {
        @Override
        public Object toNative(final Object value, final ToNativeContext context) {
            if (Objects.isNull(value)) {
                return null;
            }
            if (!(value instanceof URI)) {
                throw new IllegalArgumentException(String.format(TYPE_CONVERSION_ERROR_MESSAGE, value.getClass(), URI.class));
            }
            final URI url = (URI) value;
            return url.toString();
        }

        @Override
        public Class<String> nativeType() {
            return String.class;
        }
    }
}
