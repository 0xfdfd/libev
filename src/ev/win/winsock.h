#ifndef __EV_WINSOCK_INTERNAL_H__
#define __EV_WINSOCK_INTERNAL_H__

#define AFD_OVERLAPPED                      0x00000002
#define AFD_RECEIVE                         5
#define AFD_RECEIVE_DATAGRAM                6

#ifndef TDI_RECEIVE_NORMAL
#   define TDI_RECEIVE_PARTIAL              0x00000010
#   define TDI_RECEIVE_NORMAL               0x00000020
#   define TDI_RECEIVE_PEEK                 0x00000080
#endif

#define FSCTL_AFD_BASE                      FILE_DEVICE_NETWORK

#define _AFD_CONTROL_CODE(operation, method) \
    ((FSCTL_AFD_BASE) << 12 | (operation << 2) | method)

#define IOCTL_AFD_RECEIVE \
    _AFD_CONTROL_CODE(AFD_RECEIVE, METHOD_NEITHER)

#define IOCTL_AFD_RECEIVE_DATAGRAM \
    _AFD_CONTROL_CODE(AFD_RECEIVE_DATAGRAM, METHOD_NEITHER)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _AFD_RECV_DATAGRAM_INFO {
    LPWSABUF BufferArray;
    ULONG BufferCount;
    ULONG AfdFlags;
    ULONG TdiFlags;
    struct sockaddr* Address;
    int* AddressLength;
} AFD_RECV_DATAGRAM_INFO, * PAFD_RECV_DATAGRAM_INFO;

typedef struct _AFD_RECV_INFO {
    LPWSABUF BufferArray;
    ULONG BufferCount;
    ULONG AfdFlags;
    ULONG TdiFlags;
} AFD_RECV_INFO, * PAFD_RECV_INFO;

extern int ev_tcp_non_ifs_lsp_ipv4;
extern int ev_tcp_non_ifs_lsp_ipv6;

extern struct sockaddr_in ev_addr_ip4_any_;
extern struct sockaddr_in6 ev_addr_ip6_any_;

EV_LOCAL int WSAAPI ev__wsa_recv_workaround(SOCKET socket, WSABUF* buffers,
    DWORD buffer_count, DWORD* bytes, DWORD* flags, WSAOVERLAPPED* overlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_routine);

/**
 * @brief 
 */
EV_LOCAL int WSAAPI ev__wsa_recvfrom_workaround(SOCKET socket, WSABUF* buffers,
    DWORD buffer_count, DWORD* bytes, DWORD* flags, struct sockaddr* addr,
    int* addr_len, WSAOVERLAPPED* overlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_routine);

/**
 * @brief Initialize winsock.
 */
EV_LOCAL void ev__winsock_init(void);

/**
 * @brief Convert typeof NTSTATUS error to typeof WinSock error
 * @param[in] status  NTSTATUS error
 * @return WinSock error
 */
EV_LOCAL int ev__ntstatus_to_winsock_error(NTSTATUS status);

#ifdef __cplusplus
}
#endif
#endif
