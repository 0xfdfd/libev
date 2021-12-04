#ifndef __EV_WINAPI_INTERNAL_H__
#define __EV_WINAPI_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "ev-common.h"
#include <windows.h>

#ifndef _NTDEF_
typedef _Return_type_success_(return >= 0)  LONG NTSTATUS;
#endif

#ifndef FACILITY_NTWIN32
#   define FACILITY_NTWIN32                 0x7
#endif

#ifndef NT_SUCCESS
#   define NT_SUCCESS(Status)               (((NTSTATUS)(Status)) >= 0)
#endif

#ifndef STATUS_SUCCESS
#   define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#endif

#ifndef STATUS_OBJECT_TYPE_MISMATCH
#   define STATUS_OBJECT_TYPE_MISMATCH      ((NTSTATUS)0xC0000024L)
#endif

#ifndef STATUS_INSUFFICIENT_RESOURCES
#   define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)
#endif

#ifndef STATUS_PAGEFILE_QUOTA
#   define STATUS_PAGEFILE_QUOTA            ((NTSTATUS)0xC0000007L)
#endif

#ifndef STATUS_COMMITMENT_LIMIT
#   define STATUS_COMMITMENT_LIMIT          ((NTSTATUS)0xC000012DL)
#endif

#ifndef STATUS_WORKING_SET_QUOTA
#   define STATUS_WORKING_SET_QUOTA         ((NTSTATUS)0xC00000A1L)
#endif

#ifndef STATUS_QUOTA_EXCEEDED
#   define STATUS_QUOTA_EXCEEDED            ((NTSTATUS)0xC0000044L)
#endif

#ifndef STATUS_TOO_MANY_PAGING_FILES
#   define STATUS_TOO_MANY_PAGING_FILES     ((NTSTATUS)0xC0000097L)
#endif

#ifndef STATUS_REMOTE_RESOURCES
#   define STATUS_REMOTE_RESOURCES          ((NTSTATUS)0xC000013DL)
#endif

#ifndef STATUS_TOO_MANY_ADDRESSES
#   define STATUS_TOO_MANY_ADDRESSES        ((NTSTATUS)0xC0000209L)
#endif

#ifndef STATUS_SHARING_VIOLATION
#   define STATUS_SHARING_VIOLATION         ((NTSTATUS)0xC0000043L)
#endif

#ifndef STATUS_ADDRESS_ALREADY_EXISTS
#   define STATUS_ADDRESS_ALREADY_EXISTS    ((NTSTATUS)0xC000020AL)
#endif

#ifndef STATUS_LINK_TIMEOUT
#   define STATUS_LINK_TIMEOUT              ((NTSTATUS)0xC000013FL)
#endif

#ifndef STATUS_IO_TIMEOUT
#   define STATUS_IO_TIMEOUT                ((NTSTATUS)0xC00000B5L)
#endif

#ifndef STATUS_GRACEFUL_DISCONNECT
#   define STATUS_GRACEFUL_DISCONNECT       ((NTSTATUS)0xC0000237L)
#endif

#ifndef STATUS_REMOTE_DISCONNECT
#   define STATUS_REMOTE_DISCONNECT         ((NTSTATUS)0xC000013CL)
#endif

#ifndef STATUS_CONNECTION_RESET
#   define STATUS_CONNECTION_RESET          ((NTSTATUS)0xC000020DL)
#endif

#ifndef STATUS_LINK_FAILED
#   define STATUS_LINK_FAILED               ((NTSTATUS)0xC000013EL)
#endif

#ifndef STATUS_CONNECTION_DISCONNECTED
#   define STATUS_CONNECTION_DISCONNECTED   ((NTSTATUS)0xC000020CL)
#endif

#ifndef STATUS_PORT_UNREACHABLE
#   define STATUS_PORT_UNREACHABLE          ((NTSTATUS)0xC000023FL)
#endif

#ifndef STATUS_HOPLIMIT_EXCEEDED
#   define STATUS_HOPLIMIT_EXCEEDED         ((NTSTATUS)0xC000A012L)
#endif

#ifndef STATUS_LOCAL_DISCONNECT
#   define STATUS_LOCAL_DISCONNECT          ((NTSTATUS)0xC000013BL)
#endif

#ifndef STATUS_TRANSACTION_ABORTED
#   define STATUS_TRANSACTION_ABORTED       ((NTSTATUS)0xC000020FL)
#endif

#ifndef STATUS_CONNECTION_ABORTED
#   define STATUS_CONNECTION_ABORTED        ((NTSTATUS)0xC0000241L)
#endif

#ifndef STATUS_BAD_NETWORK_PATH
#   define STATUS_BAD_NETWORK_PATH          ((NTSTATUS)0xC00000BEL)
#endif

#ifndef STATUS_NETWORK_UNREACHABLE
#   define STATUS_NETWORK_UNREACHABLE       ((NTSTATUS)0xC000023CL)
#endif

#ifndef STATUS_PROTOCOL_UNREACHABLE
#   define STATUS_PROTOCOL_UNREACHABLE      ((NTSTATUS)0xC000023EL)
#endif

#ifndef STATUS_HOST_UNREACHABLE
#   define STATUS_HOST_UNREACHABLE          ((NTSTATUS)0xC000023DL)
#endif

#ifndef STATUS_CANCELLED
#   define STATUS_CANCELLED                 ((NTSTATUS)0xC0000120L)
#endif

#ifndef STATUS_REQUEST_ABORTED
#   define STATUS_REQUEST_ABORTED           ((NTSTATUS)0xC0000240L)
#endif

#ifndef STATUS_BUFFER_OVERFLOW
#   define STATUS_BUFFER_OVERFLOW           ((NTSTATUS)0x80000005L)
#endif

#ifndef STATUS_INVALID_BUFFER_SIZE
#   define STATUS_INVALID_BUFFER_SIZE       ((NTSTATUS)0xC0000206L)
#endif

#ifndef STATUS_BUFFER_TOO_SMALL
#   define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)
#endif

#ifndef STATUS_DEVICE_NOT_READY
#   define STATUS_DEVICE_NOT_READY          ((NTSTATUS)0xC00000A3L)
#endif

#ifndef STATUS_REQUEST_NOT_ACCEPTED
#   define STATUS_REQUEST_NOT_ACCEPTED      ((NTSTATUS)0xC00000D0L)
#endif

#ifndef STATUS_INVALID_NETWORK_RESPONSE
#   define STATUS_INVALID_NETWORK_RESPONSE  ((NTSTATUS)0xC00000C3L)
#endif

#ifndef STATUS_UNEXPECTED_NETWORK_ERROR
#   define STATUS_UNEXPECTED_NETWORK_ERROR  ((NTSTATUS)0xC00000C4L)
#endif

#ifndef STATUS_NETWORK_BUSY
#   define STATUS_NETWORK_BUSY              ((NTSTATUS)0xC00000BFL)
#endif

#ifndef STATUS_NO_SUCH_DEVICE
#   define STATUS_NO_SUCH_DEVICE            ((NTSTATUS)0xC000000EL)
#endif

#ifndef STATUS_NO_SUCH_FILE
#   define STATUS_NO_SUCH_FILE              ((NTSTATUS)0xC000000FL)
#endif

#ifndef STATUS_OBJECT_PATH_NOT_FOUND
#   define STATUS_OBJECT_PATH_NOT_FOUND     ((NTSTATUS)0xC000003AL)
#endif

#ifndef STATUS_OBJECT_NAME_NOT_FOUND
#   define STATUS_OBJECT_NAME_NOT_FOUND     ((NTSTATUS)0xC0000034L)
#endif

#ifndef STATUS_INVALID_CONNECTION
#   define STATUS_INVALID_CONNECTION        ((NTSTATUS)0xC0000140L)
#endif

#ifndef STATUS_REMOTE_NOT_LISTENING
#   define STATUS_REMOTE_NOT_LISTENING      ((NTSTATUS)0xC00000BCL)
#endif

#ifndef STATUS_CONNECTION_REFUSED
#   define STATUS_CONNECTION_REFUSED        ((NTSTATUS)0xC0000236L)
#endif

#ifndef STATUS_PIPE_DISCONNECTED
#   define STATUS_PIPE_DISCONNECTED         ((NTSTATUS)0xC00000B0L)
#endif

#ifndef STATUS_CONFLICTING_ADDRESSES
#   define STATUS_CONFLICTING_ADDRESSES     ((NTSTATUS)0xC0000018L)
#endif

#ifndef STATUS_INVALID_ADDRESS
#   define STATUS_INVALID_ADDRESS           ((NTSTATUS)0xC0000141L)
#endif

#ifndef STATUS_INVALID_ADDRESS_COMPONENT
#   define STATUS_INVALID_ADDRESS_COMPONENT ((NTSTATUS)0xC0000207L)
#endif

#ifndef STATUS_NOT_SUPPORTED
#   define STATUS_NOT_SUPPORTED             ((NTSTATUS)0xC00000BBL)
#endif

#ifndef STATUS_NOT_IMPLEMENTED
#   define STATUS_NOT_IMPLEMENTED           ((NTSTATUS)0xC0000002L)
#endif

#ifndef STATUS_ACCESS_DENIED
#   define STATUS_ACCESS_DENIED             ((NTSTATUS)0xC0000022L)
#endif

typedef enum _FILE_INFORMATION_CLASS {
    FileDirectoryInformation = 1,
    FileFullDirectoryInformation,
    FileBothDirectoryInformation,
    FileBasicInformation,
    FileStandardInformation,
    FileInternalInformation,
    FileEaInformation,
    FileAccessInformation,
    FileNameInformation,
    FileRenameInformation,
    FileLinkInformation,
    FileNamesInformation,
    FileDispositionInformation,
    FilePositionInformation,
    FileFullEaInformation,
    FileModeInformation,
    FileAlignmentInformation,
    FileAllInformation,
    FileAllocationInformation,
    FileEndOfFileInformation,
    FileAlternateNameInformation,
    FileStreamInformation,
    FilePipeInformation,
    FilePipeLocalInformation,
    FilePipeRemoteInformation,
    FileMailslotQueryInformation,
    FileMailslotSetInformation,
    FileCompressionInformation,
    FileObjectIdInformation,
    FileCompletionInformation,
    FileMoveClusterInformation,
    FileQuotaInformation,
    FileReparsePointInformation,
    FileNetworkOpenInformation,
    FileAttributeTagInformation,
    FileTrackingInformation,
    FileIdBothDirectoryInformation,
    FileIdFullDirectoryInformation,
    FileValidDataLengthInformation,
    FileShortNameInformation,
    FileIoCompletionNotificationInformation,
    FileIoStatusBlockRangeInformation,
    FileIoPriorityHintInformation,
    FileSfioReserveInformation,
    FileSfioVolumeInformation,
    FileHardLinkInformation,
    FileProcessIdsUsingFileInformation,
    FileNormalizedNameInformation,
    FileNetworkPhysicalNameInformation,
    FileIdGlobalTxDirectoryInformation,
    FileIsRemoteDeviceInformation,
    FileAttributeCacheInformation,
    FileNumaNodeInformation,
    FileStandardLinkInformation,
    FileRemoteProtocolInformation,
    FileMaximumInformation
} FILE_INFORMATION_CLASS, * PFILE_INFORMATION_CLASS;

typedef struct _IO_STATUS_BLOCK {
#pragma warning(push)
#pragma warning(disable: 4201) // we'll always use the Microsoft compiler
    union {
        NTSTATUS Status;
        PVOID Pointer;
    } DUMMYUNIONNAME;
#pragma warning(pop)

    ULONG_PTR Information;
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;

typedef struct _FILE_ACCESS_INFORMATION {
    ACCESS_MASK AccessFlags;
} FILE_ACCESS_INFORMATION, * PFILE_ACCESS_INFORMATION;

typedef struct _FILE_MODE_INFORMATION {
    ULONG Mode;
} FILE_MODE_INFORMATION, * PFILE_MODE_INFORMATION;

/**
 * @brief The NtQueryInformationFile routine returns various kinds of information about a file object.
 * @see https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-ntqueryinformationfile
 */
typedef NTSTATUS (NTAPI* fn_NtQueryInformationFile)(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock,
    PVOID FileInformation,ULONG Length,FILE_INFORMATION_CLASS FileInformationClass);

/**
 * @brief Converts the specified NTSTATUS code to its equivalent system error code.
 * @see https://docs.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-rtlntstatustodoserror
 */
typedef ULONG (NTAPI* fn_RtlNtStatusToDosError)(NTSTATUS Status);

/**
 * @brief Initialize WinAPI
 */
API_LOCAL void ev__winapi_init(void);

#ifdef __cplusplus
}
#endif
#endif
