from ldap3 import Server, Connection, Tls
import ssl

tls_configuration = Tls(validate=ssl.CERT_NONE, version=ssl.PROTOCOL_TLSv1)
server = Server('contsrv.jabberqa.cisco.com', use_ssl=True, tls=tls_configuration)
conn = Connection(server)
conn.bind()
print conn.bound
