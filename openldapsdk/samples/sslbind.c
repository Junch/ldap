/* $Novell: sslbind.c,v 1.16 2003/05/12 13:09:10 $ */
/**************************************************************************
*  Novell Software Developer Kit
*
****************************************************************************
* Copyright (c) 2006 Novell, Inc.
* All Rights Reserved.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; version 2.1 of the license.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, contact Novell, Inc.
*
* To contact Novell about this file by physical or electronic mail,
* you may find current contact information at www.novell.com
*
****************************************************************************
*
*  Copyright (C) 2002-2003 Novell, Inc. All Rights Reserved.
*
*  THIS WORK IS SUBJECT TO U.S. AND INTERNATIONAL COPYRIGHT LAWS AND TREATIES.
*  USE AND REDISTRIBUTION OF THIS WORK IS SUBJECT TO THE LICENSE AGREEMENT
*  ACCOMPANYING THE SOFTWARE DEVELOPER KIT (SDK) THAT CONTAINS THIS WORK.
*  PURSUANT TO THE SDK LICENSE AGREEMENT, NOVELL HEREBY GRANTS TO DEVELOPER A
*  ROYALTY-FREE, NON-EXCLUSIVE LICENSE TO INCLUDE NOVELL'S SAMPLE CODE IN ITS
*  PRODUCT. NOVELL GRANTS DEVELOPER WORLDWIDE DISTRIBUTION RIGHTS TO MARKET,
*  DISTRIBUTE, OR SELL NOVELL'S SAMPLE CODE AS A COMPONENT OF DEVELOPER'S
*  PRODUCTS. NOVELL SHALL HAVE NO OBLIGATIONS TO DEVELOPER OR DEVELOPER'S
*  CUSTOMERS WITH RESPECT TO THIS CODE.
*
***************************************************************************
    sslbind.c
***************************************************************************
   Description: Demonstrates connecting to a to an ldap server using SSL
***************************************************************************/

#ifdef _WINDOWS
#include <winsock2.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ldap_ssl.h>

#define LDAP_OPT_IO_FUNCS         0x7001

static char usage[] =
"\n Usage:   sslbind <host name> <port number> <login dn> <password>"
"\n\t<cert file> <file type> \n"
"  host name      = ldap server name or IP address\n"
"  port number    = port to use - 636 is ldap ssl default\n" 
"  login dn       = user name to login as\n"
"  password       = user password\n"
"  cert file      = trusted root certificate file\n"
"  file type      = DER=der encoded file, B64=b64 encoded file\n"
"\n Example: sslbind Acme.com 636 cn=admin,o=Acme secret myKey.der DER\n";

int main(int argc, char *argv[])
{

    struct timeval timeOut = {10,0};   /* 10 second connection timeout */
    int  version, rc, ldapPort;
    char *ldapHost;
    char *loginDN;
    char *password;
    char *certFile;
    char *fileType;
    int   fileEncoding;

    LDAP           *ld;
    LDAPMessage    *resultBuf = NULL;
    LDAPMessage    *entry = NULL;
    char           *attrs[] = { LDAP_NO_ATTRS, NULL };
    char           *dn;
    char  *ldapuri = NULL;
    LDAPURLDesc  url ;
   
    if (argc != 7)
    {
        printf("%s", usage);
        exit(1);
    }

    ldapHost     = argv[1];
    ldapPort     = atoi(argv[2]);
    loginDN      = argv[3];
    password     = argv[4];
    certFile     = argv[5];
    fileType     = argv[6];
   
    /* Set LDAP version to 3 and set connection timeout. */
    version = LDAP_VERSION3;
    ldap_set_option( NULL, LDAP_OPT_PROTOCOL_VERSION, &version);
    ldap_set_option( NULL, LDAP_OPT_NETWORK_TIMEOUT, &timeOut);

    /* 
     * Validate the fileType argument
     */
    if ( (0 == strcmp(fileType, "DER")) || (0 == strcmp(fileType, "der")))
       fileEncoding = LDAPSSL_CERT_FILETYPE_DER;
    else if ((0 == strcmp(fileType, "B64")) ||(0 == strcmp(fileType, "b64")))
      fileEncoding = LDAPSSL_CERT_FILETYPE_B64;
    else
    {
       printf("Invalid certificate file type.\n");
       printf("%s", usage);
       exit(1);
    }

    /* 
     * Initialize the ssl library.  The first parameter of 
     * ldapssl_client_init, if specified, must be a DER-encoded
     * certificate file.  If NULL is given for this parameter, the
     * certificate file may be specified by the ldapssl_add_trusted_cert
     * API, which accepts either DER or B64-encoded files.
     *
     * ldapssl_client_init is an application level initialization not a 
     * thread level initilization and should be done once.
     */
    rc = ldapssl_client_init(   NULL,       /* DER encoded cert file  */ 
                                NULL );     /* reserved, use NULL */

    if (rc != LDAP_SUCCESS)
    {
        printf("ldapssl_client_init error: %d\n", rc);
        exit(1);
    }

    /* 
     * Add the trusted root certificate file.  Multiple trusted root 
     * certificates can be added with ldapssl_add_trusted_cert.  Applications
     * can use this functionality to specify groups of trusted servers.
     */   
    rc = ldapssl_add_trusted_cert(certFile, fileEncoding);
    if (rc != LDAP_SUCCESS)
    {
        printf("ldapssl_add_trusted_cert error: %d\n", rc);
        ldapssl_client_deinit();
        exit(1);
    }

    /* 
     * create a LDAP session handle that is enabled for ssl connection
     */
    url.lud_scheme = "ldaps";
    url.lud_host=ldapHost;
    url.lud_port=ldapPort;
    url.lud_scope= LDAP_SCOPE_DEFAULT;
    url.lud_dn = NULL ;
    url.lud_filter = NULL ;
    url.lud_attrs = NULL ;
    url.lud_exts = NULL ; 
    ldapuri = ldap_url_desc2str ( &url );

    rc= ldap_initialize( &ld , ldapuri );
    if (rc != LDAP_SUCCESS)
    {
        printf("\n ldap_initialize  error: %d\n", rc);
        ldapssl_client_deinit();
        exit(1);
    }
    if (ld == NULL )
    {
        printf("ldap_initialize error\n" );
        ldapssl_client_deinit();
        exit(1);
    }

    /*
     *  Bind (connect) to the LDAP server.
     */
    rc = ldap_simple_bind_s( ld, loginDN, password);
   
    if (rc != LDAP_SUCCESS )
    {
        printf("ldap_simple_bind_s error: %d, %s\n", rc, ldap_err2string( rc ));
        ldap_unbind_s( ld );
        ldapssl_client_deinit();
        exit(1);
    }

    printf("SSL bind successful - performing search\n");

    /* 
     * do a simple search just to verify things worked
     */
    rc = ldap_search_ext_s(ld, "" , LDAP_SCOPE_ONELEVEL, "(objectClass=*)",
                           attrs, 0, NULL, NULL, NULL, 
                           LDAP_NO_LIMIT, &resultBuf);

    if( rc == LDAP_SUCCESS)
    {
       entry = ldap_first_entry(ld, resultBuf);

       while (entry != NULL)
       {
          dn = ldap_get_dn(ld, entry);
          printf("\tObject: %s\n", dn);

          ldap_memfree(dn);

          entry = ldap_next_entry(ld, entry);
       }

       ldap_msgfree( resultBuf );
    }
    else
    {
       printf("ldap_search_s error: %d, %s\n", rc, ldap_err2string(rc));
    }

    ldap_unbind_s( ld );

    /* 
     * Uninitialize the LDAP ssl library
     */
    ldapssl_client_deinit();

    return 0;
}

