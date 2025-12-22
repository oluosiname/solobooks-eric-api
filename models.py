"""Pydantic models for request/response validation"""
from pydantic import BaseModel, Field
from typing import Optional, List, Dict, Any


class ValidationRequest(BaseModel):
    """Request model for XML validation"""
    xml: str = Field(..., description="The XML content to validate")
    datenartversion: Optional[str] = Field(None, description="ERiC data version (e.g., UStVA_2025). Auto-detected if not provided.")


class ValidationResult(BaseModel):
    """Response model for XML validation"""
    valid: bool
    message: Optional[str] = None
    validation_result: Optional[str] = None
    error_code: Optional[int] = None
    error_message: Optional[str] = None


class SubmissionRequest(BaseModel):
    """Request model for VAT return submission"""
    xml: str = Field(..., description="The XML content to submit")
    cert_base64: str = Field(..., description="Base64-encoded certificate (.pfx file)")
    password: str = Field(..., description="Certificate password")
    datenartversion: Optional[str] = Field(None, description="ERiC data version (e.g., UStVA_2025). Auto-detected if not provided.")
    return_pdf: bool = Field(True, description="Whether to return PDF as file download (true) or base64 in JSON (false)")


class SubmissionResult(BaseModel):
    """Response model for VAT return submission (JSON format)"""
    status: str
    transfer_handle: Optional[int] = None
    pdf_base64: Optional[str] = None
    server_response: Optional[str] = None
    message: str


class HealthStatus(BaseModel):
    """Response model for health check"""
    status: str

