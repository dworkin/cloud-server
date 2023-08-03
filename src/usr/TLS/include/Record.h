# define Data			  object "/usr/TLS/lib/record/Data"
# define Record			  object "/usr/TLS/lib/record/Record"
# define RemoteRecord		  object "/usr/TLS/lib/record/RemoteRecord"
# define Alert			  object "/usr/TLS/lib/record/Alert"
# define RemoteAlert		  object "/usr/TLS/lib/record/RemoteAlert"
# define Handshake		  object "/usr/TLS/lib/record/Handshake"
# define RemoteHandshake	  object "/usr/TLS/lib/record/RemoteHandshake"
# define ClientHello		  object "/usr/TLS/lib/record/ClientHello"
# define RemoteClientHello	  object "/usr/TLS/lib/record/RemoteClientHello"
# define ServerHello		  object "/usr/TLS/lib/record/ServerHello"
# define RemoteServerHello	  object "/usr/TLS/lib/record/RemoteServerHello"
# define NewSessionTicket	  object "/usr/TLS/lib/record/NewSessionTicket"
# define RemoteNewSessionTicket	  object "/usr/TLS/lib/record/RemoteNewSessionTicket"
# define Extensions		  object "/usr/TLS/lib/record/Extensions"
# define RemoteExtensions	  object "/usr/TLS/lib/record/RemoteExtensions"
# define Certificates		  object "/usr/TLS/lib/record/Certificates"
# define RemoteCertificates	  object "/usr/TLS/lib/record/RemoteCertificates"
# define CertificateRequest	  object "/usr/TLS/lib/record/CertificateRequest"
# define RemoteCertificateRequest object "/usr/TLS/lib/record/RemoteCertificateRequest"
# define CertificateVerify	  object "/usr/TLS/lib/record/CertificateVerify"
# define RemoteCertificateVerify  object "/usr/TLS/lib/record/RemoteCertificateVerify"
# define Finished		  object "/usr/TLS/lib/record/Finished"
# define RemoteFinished		  object "/usr/TLS/lib/record/RemoteFinished"
# define KeyUpdate		  object "/usr/TLS/lib/record/KeyUpdate"
# define RemoteKeyUpdate	  object "/usr/TLS/lib/record/RemoteKeyUpdate"


# define TLS_VERSION_10					"\x03\x01"
# define TLS_VERSION_12					"\x03\x03"
# define TLS_VERSION_13					"\x03\x04"

# define RECORD_CHANGE_CIPHER_SPEC			20
# define RECORD_ALERT					21
# define RECORD_HANDSHAKE				22
# define RECORD_APPLICATION_DATA			23


# define ALERT_WARNING					1
# define ALERT_FATAL					2

# define ALERT_UNEXPECTED_MESSAGE			10
# define ALERT_BAD_RECORD_MAC				20
# define ALERT_RECORD_OVERFLOW				22
# define ALERT_HANDSHAKE_FAILURE			40
# define ALERT_BAD_CERTIFICATE				42
# define ALERT_UNSUPPORTED_CERTIFICATE			43
# define ALERT_CERTIFICATE_REVOKED			44
# define ALERT_CERTIFICATE_EXPIRED			45
# define ALERT_CERTIFICATE_UNKNOWN			46
# define ALERT_ILLEGAL_PARAMETER			47
# define ALERT_UNKNOWN_CA				48
# define ALERT_ACCESS_DENIED				49
# define ALERT_DECODE_ERROR				50
# define ALERT_DECRYPT_ERROR				51
# define ALERT_PROTOCOL_VERSION				70
# define ALERT_INSUFFICIENT_SECURITY			71
# define ALERT_INTERNAL_ERROR				80
# define ALERT_INAPPROPRIATE_FALLBACK			86
# define ALERT_USER_CANCELED				90
# define ALERT_MISSING_EXTENSION			109
# define ALERT_UNSUPPORTED_EXTENSION			110
# define ALERT_UNRECOGNIZED_NAME			112
# define ALERT_BAD_CERTIFICATE_STATUS_RESPONSE		113
# define ALERT_UNKNOWN_PSK_IDENTITY			115
# define ALERT_CERTIFICATE_REQUIRED			116
# define ALERT_NO_APPLICATION_PROTOCOL			120


# define HANDSHAKE_CLIENT_HELLO				1
# define HANDSHAKE_SERVER_HELLO				2
# define HANDSHAKE_NEW_SESSION_TICKET			4
# define HANDSHAKE_END_OF_EARLY_DATA			5
# define HANDSHAKE_EXTENSIONS				8
# define HANDSHAKE_CERTIFICATES				11
# define HANDSHAKE_CERTIFICATE_REQUEST			13
# define HANDSHAKE_CERTIFICATE_VERIFY			15
# define HANDSHAKE_FINISHED				20
# define HANDSHAKE_KEY_UPDATE				24
# define HANDSHAKE_MESSAGE_HASH				254
