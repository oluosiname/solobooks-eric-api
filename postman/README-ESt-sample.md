# ESt sample return — field notes

`sample_est_2025.xml` is a complete 2025 income tax return that ERiC validates
successfully (`rc = 0`, `<Erfolg/>`). Single filer, freelance software developer,
24.500 € profit.

It was derived from the ERiC schema documentation in `downloads/`, not guessed.
Use `scripts/lookup_est_field.py <field>` to resolve any field's type, allowed
values and pattern.

## Constraints that are not obvious

| Field | Requirement |
| --- | --- |
| `E10/@version` | fixed `"2025"` — not `"1"` |
| `S/Person` | `PersonA` / `PersonB` — not `1` or `001` |
| `ESt1A/Allg/A/E0100402` | religion key: `02`,`03`,`05`,`11`,… (`11` = none) |
| `ESt1A/Allg/A/E0101104` | street name; `E0101206` is the house number (digits only, max 4) |
| `ESt1A/Allg/A/E0100081` | Steuer-ID — **not enterable**, ELSTER fills it |
| `Vorsatz` | mandatory block, and the **last** child of `E10` |
| `Vorsatz/StNr` | first 4 digits must equal the Finanzamt number, and the check digit must be valid |
| `Vorsatz/OrdNrArt` | `S` or `O` |
| `Vorsatz/Rueckuebermittlung` | if `Bescheid` is `1`, `ArtRueckuebermittlung` must be `INTERNET` |
| `TransferHeader/Datei/Verschluesselung` | `CMSEncryptedData` — `PKCS#7v1.5` is rejected |
| `S/Gewinn/Freiber_T` | activity description and profit must appear **together**, profit in `E0803202` |

## Test values

Finanzamt `1096` is one of ERiC's test Finanzämter (`EricHoleTestfinanzaemter`).
Steuernummer `1096081508152` passes `EricPruefeSteuernummer` and matches that
Finanzamt. Changing one without the other will fail validation.

`Testmerker 700000004` routes everything to the ELSTER clearing house — nothing
is filed with a real Finanzamt.
