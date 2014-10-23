//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: CONTROLLER_CATEGORY
//
// MessageText:
//
// Controller
//
#define CONTROLLER_CATEGORY              ((DWORD)0x00000001L)

//
// MessageId: WORKER_CATEGORY
//
// MessageText:
//
// Worker
//
#define WORKER_CATEGORY                  ((DWORD)0x00000002L)

//
// MessageId: SERVICE_CATEGORY
//
// MessageText:
//
// Service 
//
#define SERVICE_CATEGORY                 ((DWORD)0x00000003L)

//
// MessageId: MSG_EL_INFO
//
// MessageText:
//
// INFORMATION: %1
//
#define MSG_EL_INFO                      ((DWORD)0x40000100L)

//
// MessageId: MSG_EL_WARNING
//
// MessageText:
//
// WARNING: %1
//
#define MSG_EL_WARNING                   ((DWORD)0x80000101L)

//
// MessageId: MSG_EL_ERROR
//
// MessageText:
//
// ERROR: %1
//
#define MSG_EL_ERROR                     ((DWORD)0xC0000102L)

//
// MessageId: MSG_EL_SUCCESS
//
// MessageText:
//
// SUCCESS: %1
//
#define MSG_EL_SUCCESS                   ((DWORD)0x00000103L)

//
// MessageId: MSG_EL_AUDIT_SUCCESS
//
// MessageText:
//
// AUDIT SUCCESS: %1
//
#define MSG_EL_AUDIT_SUCCESS             ((DWORD)0x00000104L)

//
// MessageId: MSG_EL_AUDIT_FAILURE
//
// MessageText:
//
// AUDIT FAILURE: %1
//
#define MSG_EL_AUDIT_FAILURE             ((DWORD)0xC0000105L)

