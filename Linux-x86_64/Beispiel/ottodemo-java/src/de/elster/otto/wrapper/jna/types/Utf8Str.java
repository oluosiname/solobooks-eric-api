package de.elster.otto.wrapper.jna.types;

import java.util.Objects;

/**
 * UTF-8-String Container, der eingeführt wurde, um das Abbilden auf UTF-8 kodierte Strings
 * mit JNA durchzuführen. String-Objekte werden bei dieser Wrapper-Implementierung im nativen System-Encoding
 * an die Otto-Bibliothek übergeben.
 */

public class Utf8Str {
    private final String value;

    /**
     * Erzeugt ein UTF-8-String-Container
     *
     * @param value String
     * @return UTF-8-String-Container
     */
    public static Utf8Str create(final String value) {
        if (Objects.isNull(value)) return null;
        return new Utf8Str(value);
    }

    private Utf8Str(final String value) {
        Objects.requireNonNull(value, "Das Argument `value` darf nicht null sein!");
        this.value = value;
    }

    @Override
    public boolean equals(final Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        final Utf8Str utf8Str = (Utf8Str) o;
        return Objects.equals(value, utf8Str.value);
    }

    @Override
    public int hashCode() {
        return Objects.hashCode(value);
    }

    @Override
    public String toString() {
        return value;
    }
}
