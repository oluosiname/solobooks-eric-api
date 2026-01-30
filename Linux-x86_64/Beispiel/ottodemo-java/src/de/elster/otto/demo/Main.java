package de.elster.otto.demo;

import com.beust.jcommander.JCommander;
import com.beust.jcommander.Parameter;
import com.beust.jcommander.ParameterException;
import com.beust.jcommander.Parameters;
import com.sun.jna.Platform;
import de.elster.otto.wrapper.*;
import de.elster.otto.wrapper.exception.OttoException;
import de.elster.otto.wrapper.jna.OttoLibrary;
import de.elster.otto.wrapper.jna.OttoLibraryFactory;

import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Objects;
import java.util.Scanner;

import java.nio.file.StandardCopyOption;


/**
 * Hauptklasse zur Ausführung der Otto-Demo
 */
@SuppressWarnings("java:S106")
@Parameters(parametersValidators = OttoParameterValidator.class)
public class Main {


    @Parameter(names = {"-e", "--empfange"}, description = "Empfang eines Datenobjekts, das in die Datei <dateiname> "
            + "geschrieben wird. Die Option darf nur angegeben werden, wenn auch die "
            + "Option -o gesetzt ist, sie darf nicht angegeben werden, "
            + "wenn die Option -s gesetzt ist.")
    private Path empfangDatei = Paths.get("beispiel.xml");

    @Parameter(names = {"-s", "--sende"}, description = "Sendet die Datei <dateiname> an OTTER. Die Option darf "
            + "nicht angegeben werden, wenn die Optionen -e oder -o gesetzt sind. "
            + "Eine Prüfung der Objektgröße durch die Otto-Demo ist nicht "
            + "erforderlich, da diese vom Otto vorgenommen wird.")
    private Path sendeDateiPath;

    @Parameter(names = {"-h", "--help"}, description = "Kommandozeilenhilfe anzeigen", help = true)
    private boolean help;

    @Parameter(names = {"-c", "--certPath"}, description = "Pfad zu einem Benutzerzertifikat")
    private String certPath = "test-softidnr-pse.pfx";

    @Parameter(names = {"-p", "--pin"}, description = "Pin für das Benutzerzertifikat, _NULL (Nullzeiger) "
            + "für keine PIN")
    private String certificatePin = "123456";

    @Parameter(names = {"-o", "--objektId"}, description = "Objekt-UUID des Objekts, das abgeholt werden soll. Die Option darf "
            + "nur angegeben werden, wenn auch die Option -e gesetzt ist, sie darf "
            + "nicht angegeben werden, wenn die Option -s gesetzt ist.")
    private String objektId = "7090bc69-be5e-4fd0-b91a-2128021295d6";

    @Parameter(names = {"-d", "--dir"}, description = "Pfad zum Verzeichnis mit den Otto-Bibliotheken")
    private Path ottoPath = Paths.get("../../").resolve(Platform.isWindows() ? "dll" : "lib").toAbsolutePath().normalize();

    @Parameter(names = {"-l", "--log"}, description = "Pfad zum Verzeichnis, in das die Otto-Protokolldateien "
            + "geschrieben werden")
    private Path logPath = Paths.get(".").toAbsolutePath();

    @Parameter(names = {"-i"}, description = "Ihre von ELSTER zugeteilte Hersteller-ID. Wenn nicht "
            + "angegeben, muss diese in der Console eingegeben werden.")
    private String herstellerId;

    private Main() {
    }

    /**
     * Hauptmethode zur Ausführung der Otto-Demo
     *
     * @param args Die Kommandozeilenargumente
     */
    public static void main(final String[] args) {
        final Main main = new Main();
        final JCommander jCommander = JCommander.newBuilder().addObject(main).acceptUnknownOptions(true).build();
        try {
            jCommander.setAllowParameterOverwriting(true);
            jCommander.parse(args);
        } catch (final ParameterException e) {
            printHelp(jCommander, e.getMessage());
            return;
        }

        main.run(jCommander);
    }

    private static void printHelp(final JCommander comm, final String errorText) {
        if (errorText != null) {
            System.err.printf("Error: %s%n%n", errorText);
        }
        comm.usage();
        final String cmd = "java -jar ottodemo.jar";
        System.out.println();
        System.out.println("Beispiele:");
        System.out.println("      " + cmd);
        System.out.println("      " + cmd + " -h");

        System.out.println();
    }

    private static OttoCertificate newOttoCertificate(final OttoLibrary library, final OttoInstance instance, final String certificatePath, final String certificatePin) throws OttoException, URISyntaxException {
        if (Objects.nonNull(certificatePath) && (certificatePath.startsWith("http://") || certificatePath.startsWith("https://"))) {
            return OttoCertificate.open(library, instance, new URI(certificatePath));
        }
        return OttoCertificate.open(library, instance, Objects.nonNull(certificatePath) ? Paths.get(certificatePath) : null, certificatePin);
    }

    private void run(final JCommander comm) {

        if (help) {
            printHelp(comm, null);
        } else if (!comm.getUnknownOptions().isEmpty()) {
            printHelp(comm, "Unknown option " + comm.getUnknownOptions().get(0));
        } else {
            final String nullOption = "_NULL";
            final String certificatePath = nullOption.equalsIgnoreCase(certPath) ? null : certPath;
            certificatePin = nullOption.equalsIgnoreCase(certificatePin) ? null : certificatePin;

            final Scanner in = new Scanner(System.in);
            try {
                final OttoLibrary library = OttoLibraryFactory.load(ottoPath);
                try (final OttoInstance instance = OttoInstance.create(library, logPath, null);
                     final OttoCertificate certificate = newOttoCertificate(library, instance, certificatePath, certificatePin)) {

                    if (herstellerId == null || herstellerId.isEmpty()) {
                        System.out.print("Bitte geben Sie Ihre Hersteller-ID ein: ");
                        herstellerId = in.nextLine();
                        System.out.println();
                    }

                    // Beispiel für die Verwendung eines Proxy
                    // setzeProxy(instance);

                    if (Objects.isNull(sendeDateiPath)) {
                        empfangeDatei(library, instance, certificate);
                    } else {
                        sendeDatei(library, instance, certificate, sendeDateiPath);
                    }
                }
            } catch (final Exception e) {
                System.err.println("Fehler: " + e);
                e.printStackTrace();
            }

            System.out.println();
            System.out.print("Bitte drücken Sie die Eingabetaste");
            in.nextLine();
            System.out.println();
        }
    }

    private void empfangeDatei(final OttoLibrary ottoLibrary, final OttoInstance instance, final OttoCertificate cert) throws
            IOException, OttoException {
        System.out.println("*** Hole Datei von OTTER ***");
        System.out.println("Objekt-UUID: " + objektId);

        if (Files.exists(empfangDatei) && !Files.isRegularFile(empfangDatei)) {
            System.err.println("Es wurde keine gültige Datei angegeben.");
            return;
        }
        if (Files.exists(empfangDatei) && !Files.isWritable(empfangDatei)) {
            System.err.println("In die angegebene Datei darf nicht geschrieben werden.");
            return;
        }

        System.out.println("Speichere Daten in: " + empfangDatei);
        try (final OttoEmpfangInputStream empfangInput = OttoEmpfangInputStream.open(ottoLibrary, instance, cert, objektId,
                herstellerId)) {
            System.out.print("Empfange Daten ");
            final long bytes = Files.copy(empfangInput, empfangDatei, StandardCopyOption.REPLACE_EXISTING);
            System.out.printf("%d Byte erfolgreich abgeholt.%n", bytes);
        }


    }

    private void sendeDatei(final OttoLibrary library, final OttoInstance instance, final OttoCertificate cert, final Path sendePath) throws
            OttoException, IOException {
        System.out.println("*** Sende Datei zum OTTER ***");


        if (Files.exists(sendePath) && !Files.isRegularFile(sendePath)) {
            System.err.println("Es wurde keine gültige Datei angeben.");
            return;
        }
        if (!Files.isReadable(sendePath)) {
            System.err.println("Es kann aus der Datei nicht gelesen werden.");
            return;
        }

        // Bestimmen der base64-codierten Prüfsumme
        System.out.println("Erstelle signierte Prüfsumme.");

        String signedChecksum = null;
        try (final OttoChecksumOutputStream checksum = OttoChecksumOutputStream.open(library, instance)) {
            Files.copy(sendePath, checksum);
            signedChecksum = checksum.sign(cert);
        }
        System.out.println("Prüfsumme wurde erzeugt und signiert.");

        System.out.println("Versende Daten");
        try (final OttoVersandOutputStream versand = OttoVersandOutputStream.open(library, instance, signedChecksum,
                herstellerId)) {
            Files.copy(sendePath, versand);

            final String objectID = versand.finish();
            System.out.println("Versand erfolgreich: Objekt-UUID = " + objectID);
        }
    }

    @SuppressWarnings({"java:S1144", "unused"})
    private void setzeProxy(final OttoInstance instance) throws OttoException {
        final OttoLibrary.OttoProxyKonfiguration proxy = new OttoLibrary.OttoProxyKonfiguration("test:1234", "domain\\name",
                "password", "NTLM");
        instance.setProxyConfiguration(proxy);
    }


}
