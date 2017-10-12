from ldap3 import Server, Connection, ALL

server = Server('contsrv.jabberqa.cisco.com', get_info=ALL)
conn = Connection(server, user="contsrv\\user1", password="cisco123!@#", auto_bind=True)
print conn.extend.standard.who_am_i()
# if conn.search('dc=contsrv,dc=jabberqa,dc=cisco,dc=com', '(objectclass=person)'):
# 	print conn.entries

if conn.search('dc=contsrv,dc=jabberqa,dc=cisco,dc=com', '(objectclass=person)', attributes=['sn', 'mail', 'objectclass']):
	print conn.entries[9]
	print conn.entries[10]

conn.unbind()
