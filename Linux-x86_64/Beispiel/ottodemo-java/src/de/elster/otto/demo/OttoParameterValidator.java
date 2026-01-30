package de.elster.otto.demo;

import java.util.Map;

import com.beust.jcommander.IParametersValidator;
import com.beust.jcommander.ParameterException;

/**
 * Klasse zur Parametervalidierung der Demo
 */
public class OttoParameterValidator implements IParametersValidator {

    /**
     * Standardkonstruktor für OttoParameterValidator
     */
    public OttoParameterValidator() {
        // Wird von JCommander benötigt
    }

    @Override
    public void validate(final Map<String, Object> parameters) throws ParameterException {
        if (parameters.get("--empfange") != null && parameters.get("--sende") != null) {
            throw new ParameterException("Parameter für Senden und Empfangen dürfen nicht gleichzeitig angegeben werden.");
        } else if (parameters.get("--sende") != null && parameters.get("--objektId") != null) {
            throw new ParameterException("Beim Senden darf keine Objekt-ID angegeben werden.");
        }
    }
}
