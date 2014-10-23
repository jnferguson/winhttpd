MessageIdTypeDef=DWORD

SeverityNames=(
				Success=0x0:STATUS_SEVERITY_SUCCESS
				Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
				Warning=0x2:STATUS_SEVERITY_WARNING
				Error=0x3:STATUS_SEVERITY_ERROR
)

LanguageNames=(
		English=0x409:MSG00409
		Neutral=0x0000:MSG00000
)

MessageId=0x1
SymbolicName=CONTROLLER_CATEGORY
Language=English
Controller
.

MessageId=0x2
SymbolicName=WORKER_CATEGORY
Language=English
Worker
.

MessageId=0x3
SymbolicName=SERVICE_CATEGORY
Language=English
Service 
.

MessageId=0x100   
SymbolicName=MSG_EL_INFO
Severity=Informational
Facility=Application
Language=English
INFORMATION: %1
.

MessageId=0x101   
SymbolicName=MSG_EL_WARNING
Severity=Warning
Facility=Application
Language=English
WARNING: %1
.

MessageId=0x102   
SymbolicName=MSG_EL_ERROR
Severity=Error
Facility=Application
Language=English
ERROR: %1
.

MessageId=0x103   
SymbolicName=MSG_EL_SUCCESS
Severity=Success
Facility=Application
Language=English
SUCCESS: %1
.

MessageId=0x104   
SymbolicName=MSG_EL_AUDIT_SUCCESS
Severity=Success
Facility=Application
Language=English
AUDIT SUCCESS: %1
.

MessageId=0x105
SymbolicName=MSG_EL_AUDIT_FAILURE
Severity=Error
Facility=Application
Language=English
AUDIT FAILURE: %1
.
