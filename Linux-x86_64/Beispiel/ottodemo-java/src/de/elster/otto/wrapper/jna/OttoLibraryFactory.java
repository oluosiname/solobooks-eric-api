package de.elster.otto.wrapper.jna;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Platform;
import de.elster.otto.wrapper.exception.WrapperException;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.Map;

/**
 * Factory-Klasse zum Erzeugen der Otto-Bibliothek
 */
public class OttoLibraryFactory {


    /**
     * Die Otto-Bibliothek darf unter AIX und MacOs nicht entladen werden,
     * daher wird eine starke Referenz auf das Objekt behalten.
     */
    @SuppressWarnings("unused")
    private static OttoLibrary library;

    private OttoLibraryFactory() {
    }

    /**
     * Gibt betriebssystemabhängig den Namen einer Shared-Library zurück
     *
     * @param name Basis-Name der Shared-Library
     * @return betriebssystemabhängiger Name
     */
    public static String mapLibraryName(final String name) {
        final String libName = System.mapLibraryName(name);
        if (Platform.isAIX()) {
            return libName.replaceAll("\\.a$", ".so");
        }
        return libName;
    }

    /**
     * Den Otto laden und initialisieren.
     *
     * @param ottoPath In diesem Verzeichnis werden die Otto Bibliotheken erwartet.
     * @return OttoLibrary-JNA-Proxy
     * @throws WrapperException Falls ein Fehler im Java-Wrapper auftritt
     */
    public static OttoLibrary load(final Path ottoPath) throws WrapperException {


        final String libraryName = mapLibraryName("otto");
        final Path ottoApiPath = ottoPath.resolve(libraryName).normalize();

        if (!Files.exists(ottoApiPath)) {
            throw new WrapperException(String.format("`%s` wurde in Pfad `%s` nicht gefunden", libraryName, ottoApiPath));
        }

        final Map<String, Object> options = new HashMap<>();
        options.put(Library.OPTION_TYPE_MAPPER, OttoLibrary.TYPE_MAPPER);

        // So verhindern wir, dass der JNA sich die jnidispatch
        // Bibliothek nicht von anderswo her als dem mitgelieferten
        // Repository holt:
        System.setProperty("jna.nosys", "true");

        OttoLibrary library = null;
        try {
            library = Native.load(ottoApiPath.toAbsolutePath().toString(), OttoLibrary.class, options);
        } catch (final UnsatisfiedLinkError e) {
            throw new WrapperException("Die otto-Bibliothek konnte nicht geladen werden! "
                    + "Bitte stellen Sie sicher, dass ihre Java-Installation in "
                    + "der gleichen CPU-Architektur vorliegt wie otto (x86/AMD64).", e);
        }

        if (library == null) {
            throw new WrapperException("Fehler beim Laden des JNA-Wrapperobjekts");
        }

        /*
         * Verhindere das Entladen der ERiC-Bibliothek unter AIX und Macos, indem eine
         * starke Referenz auf das Objekt erhalten bleibt.
         */
        if (Platform.isAIX() || Platform.isMac()) {
            OttoLibraryFactory.library = library;
        }

        return library;
    }


}
