from ldap3 import Server, Connection, ALL, NTLM

server = Server('contsrv.jabberqa.cisco.com', get_info=ALL)
conn = Connection(server, user="contsrv\\user1", password="cisco123!@#", authentication=NTLM, auto_bind=True)
print conn.extend.standard.who_am_i()
conn.unbind()
