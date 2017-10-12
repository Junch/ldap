from ldap3 import Server, Connection, ALL

server = Server('contsrv.jabberqa.cisco.com', get_info=ALL)
conn = Connection(server, auto_bind=True)
print server.info
print server.schema