/**************************************************************************
*  Novell Software Developer Kit
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
*  Copyright (C) 2002-2004 Novell, Inc. All Rights Reserved.
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
*
***************************************************************************
	backup.c
***************************************************************************
    Description: The backup.c sample demonstrates how to backup and restore
    attribute anmes and values for anobject.  The backup/restore APIs are 
    object based. It is the developer's responsibility to decide how to
    deicide how to store the information so it can be restored when calling
    ldap_restore_object. If any of one the attributes in the user object has 
    been encrypted  user needs to supply a password for encryption, same
    password  should be supplied for decryption. If non of the attributes are 
    encrypted an empty string ie "" needs to passed as part of the 3rd
    parameter in both the functions, backup and restore. 

    eDirectory 8.8 or newer is required.
***************************************************************************/
#ifdef _WINDOWS
#include <winsock2.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WINDOWS
#include <stdint.h>
#endif
#include <ldap.h>
#include <ldapx.h>

static char usage[] =
    "\n Usage:   backup <host name> <port number> <login dn> <password>"
    "\n\t  <object dn> <object Encryption passwd>\n"
    "\n Example: backup acme.com 389 cn=admin,o=acme secret"
    "\n\t  cn=James,ou=sales,o=acme password\n";


int main(int argc, char **argv)
{
    int   rc, i, version, ldapPort;
    char  *ldapHost;
    char  *loginDN;
    char  *password = NULL;
    char  *encrypt = NULL;
    char  *dn;
    char  *objectInfo=NULL, *stateInfo=NULL, *chunckSize=NULL;
    LDAP  *ld;
    int   size, err = LDAP_SUCCESS;

    struct timeval timeOut = {10,0};   /* 10 second connection timeout */

    if (argc != 6 && argc != 7)
    {
        printf("%s", usage);
        return(1);
    }

    ldapHost   =  argv[1];
    ldapPort   =  atoi(argv[2]);
    loginDN    =  argv[3];
    password   =  argv[4];
    dn         =  argv[5];

    if(argc == 7) 
        encrypt    =  argv[6];

    /* Set LDAP version to 3 and set connection timeout. */
    version = LDAP_VERSION3;
    ldap_set_option( NULL, LDAP_OPT_PROTOCOL_VERSION, &version);
    ldap_set_option( NULL, LDAP_OPT_NETWORK_TIMEOUT, &timeOut);

    /* Initialize the LDAP session */
    printf( "    Getting the handle to the LDAP connection...\n" );
    if ( (ld = (LDAP *)(intptr_t) ldap_init( ldapHost, ldapPort )) == NULL ) {
        perror( "ldap_init" );
        return( 1 );
    }

    /* Authenticate to the server */
    printf( "    Binding to the directory...\n" );
    rc = ldap_simple_bind_s( ld, loginDN, password );
    if ( rc != LDAP_SUCCESS ) {
        printf( "ldap_simple_bind_s: %s\n", ldap_err2string(rc));
        ldap_unbind_s( ld );
        return( 1 );
    }

    /* Backing up the dn supplied */
    printf( "    Backing up the directory object: %s ...\n", dn);
    rc = ldap_backup_object ( ld,
                              dn,
                              encrypt,
                              &stateInfo, 
                              &objectInfo, 
                              &chunckSize,
                              &size);

    if (rc  != LDAP_SUCCESS) {
        printf("ldap_backup: %s \n", ldap_err2string(rc));
        goto done;
    }

    printf("    Backup done successfully.\n\n");
    printf("    Restoring the directory object: %s ...\n", dn );
    printf("    Press any key to continue ... ");getchar();
    /* Call the ldap_extended_operation (synchronously)  */
    rc = ldap_restore_object ( ld, 
                               dn, 
                               encrypt,
                               objectInfo,
                               chunckSize,
                               size);

    if (rc  != LDAP_SUCCESS) {
        printf("ldap_restore: %s \n", ldap_err2string(rc));
        goto done;
    }

    printf("\n    Object restored successfully.\n\n");

    ldap_unbind_s( ld );


done:
    if (objectInfo)
       ldapx_memfree( objectInfo );
    if (stateInfo)
       ldapx_memfree( stateInfo );

    return (rc);
}
