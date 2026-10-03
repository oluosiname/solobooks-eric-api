"""Pydantic models for request/response validation"""
from datetime import date

from pydantic import BaseModel, Field
from typing import Optional, List, Dict, Any


class ValidationRequest(BaseModel):
    """Request model for XML validation"""
    xml: str = Field(..., description="The XML content to validate")
    datenartversion: Optional[str] = Field(None, description="ERiC data version (e.g., UStVA_2025, EUER_2025, ESt_2025, ZMDO). Auto-detected if not provided. Note ZMDO carries no year.")


class ValidationResult(BaseModel):
    """Response model for XML validation"""
    valid: bool
    message: Optional[str] = None
    validation_result: Optional[str] = None
    error_code: Optional[int] = None
    error_message: Optional[str] = None


class SubmissionRequest(BaseModel):
    """Request model for tax return submission (UStVA, ZM, etc.)"""
    xml: str = Field(..., description="The XML content to submit")
    cert_base64: str = Field(..., description="Base64-encoded certificate (.pfx file)")
    password: str = Field(..., description="Certificate password")
    datenartversion: Optional[str] = Field(None, description="ERiC data version (e.g., UStVA_2025, EUER_2025, ESt_2025, ZMDO). Auto-detected if not provided. Note ZMDO carries no year.")
    return_pdf: bool = Field(True, description="Whether to return PDF as file download (true) or base64 in JSON (false)")


class SubmissionResult(BaseModel):
    """Response model for tax return submission (JSON format)"""
    status: str
    # transfer_handle is ERiC's Datenabholung bundling parameter, not a receipt.
    # transferticket is the reference the filer quotes to the Finanzamt.
    transfer_handle: Optional[int] = None
    transferticket: Optional[str] = None
    pdf_base64: Optional[str] = None
    server_response: Optional[str] = None
    message: str


class HealthStatus(BaseModel):
    """Response model for health check"""
    status: str



class DatenabholungRequest(BaseModel):
    """Request model for a VaSt Belegabruf (ElsterDatenabholung)"""
    idnr: str = Field(..., description="Steuer-IdNr of the Dateninhaber (11 digits)")
    year: int = Field(..., description="Veranlagungsjahr")
    cert_base64: str = Field(..., description="Base64-encoded certificate (.pfx file)")
    password: str = Field(..., description="Certificate password")
    hersteller_id: str = Field(..., description="ELSTER Hersteller-ID")
    datenlieferant: str = Field('Solobooks', description="DatenLieferant")
    product_name: str = Field('Solobooks', description="Hersteller/ProduktName")
    product_version: str = Field('spike', description="Hersteller/ProduktVersion")
    belegart: Optional[str] = Field('VaSt_LStB', description="Narrow to one Belegart, or null for all")
    testmerker: Optional[str] = Field(None, description="Testmerker, omitted for real cases")


class DatenabholungResult(BaseModel):
    """Response model for a VaSt Belegabruf"""
    # Both phases run server-side because they must share one transfer handle.
    belege: List[str] = []
    error_code: Optional[int] = None
    error_message: Optional[str] = None


class ElsterBrmCredentials(BaseModel):
    """Fields every ElsterBRM request carries"""
    cert_base64: str = Field(..., description="Base64-encoded certificate (.pfx) of the Datenabrufer")
    password: str = Field(..., description="Certificate password")
    hersteller_id: str = Field(..., description="ELSTER Hersteller-ID")
    datenlieferant: str = Field('Solobooks', description="DatenLieferant")


class SpezRechtAntragRequest(ElsterBrmCredentials):
    """Ask a Dateninhaber to authorise retrieval of their Belege"""
    idnr: str = Field(..., description="Steuer-IdNr of the Dateninhaber")
    date_of_birth: date = Field(..., description="Dateninhaber's date of birth, ISO 8601")
    valid_until: date = Field(..., description="GueltigBis, at least 10 days out")
    datenabrufer_mail: str = Field(..., description="DatenabruferMail")


class SpezRechtListeRequest(ElsterBrmCredentials):
    """List authorisations for the given Dateninhaber"""
    idnrs: List[str] = Field(..., min_length=1, description="Steuer-IdNrs to filter by")


class SpezRechtStornoRequest(ElsterBrmCredentials):
    """Withdraw an authorisation"""
    antrags_id: str = Field(..., description="AntragsID returned by SpezRechtAntrag")


class ElsterBrmResult(BaseModel):
    """Parsed ElsterBRM answer, or the ERiC/ELSTER failure"""
    data: Optional[Any] = None
    error_code: Optional[int] = None
    error_message: Optional[str] = None
