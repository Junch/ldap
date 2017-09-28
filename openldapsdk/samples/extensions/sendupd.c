/* $Novell: sendupd.c,v 1.12 2003/05/12 13:23:30 $ */
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
    sendupd.c
***************************************************************************
   Description: The sendupd.c sample requests that a specified replica 
                on a specified server send all updates to the replica ring.       
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
"\n Usage:   sendupd <host name> <port number> <login DN> <password>" 
"\n\t  <partition root> <from server DN>\n"
"\n Example: sendupd Acme.com 389 cn=admin,o=Acme secret ou=Sales,o=Acme"
"\n\t  cn=myServer,o=acme\n";

int main(int argc, char **argv)
{      
    int   rc, version, ldapPort;   
    char  *ldapHost;         
    char  *loginDN;
    char  *password;
    char  *partitionRoot;      
    char  *fromServerDN;
    char  *msg;
    LDAP  *ld;
    struct timeval timeOut = {10,0};   /* 10 second connection timeout */

    if (argc != 7)
    {
        printf("%s", usage);
        return(1);
    }


    ldapHost      =  argv[1];
    ldapPort      =  atoi(argv[2]);
    loginDN       =  argv[3];
    password      =  argv[4];
    partitionRoot =  argv[5];      
    fromServerDN  =  argv[6];   

    /* Set LDAP version to 3 and set connection timeout. */
    version = LDAP_VERSION3;
    ldap_set_option( NULL, LDAP_OPT_PROTOCOL_VERSION, &version);
    ldap_set_option( NULL, LDAP_OPT_NETWORK_TIMEOUT, &timeOut);

    /* Initialize the LDAP session */
    printf( "    Initializing the LDAP session handle ...\n" );
    if ( (ld =(LDAP *) (intptr_t)ldap_init( ldapHost, ldapPort )) == NULL )
    {
        perror( "ldap_init" );
        return( 1 );  
    }

    /* Authenticate to the server */
    printf( "    Binding to the directory...\n" );
    rc = ldap_simple_bind_s( ld, loginDN, password );
    if ( rc != LDAP_SUCCESS )
    {
        printf( "ldap_simple_bind_s: %s\n", ldap_err2string(rc));
        ldap_unbind_s( ld );
        return( 1 );
    }

    /* 
     * Novell extended operation to send out all updates made in this replica.
     */
    printf( "    Calling ldap_send_all_updates...\n" );
    rc = ldap_send_all_updates(
                              ld,             /* LDAP session handle */  
                              partitionRoot,  /* partition root */
                              fromServerDN);  /* originating server */

    if ( rc != LDAP_SUCCESS )
    {
        /* print out LDAP server message */         
        if(ldap_get_option (ld, LDAP_OPT_ERROR_STRING, &msg ) == 
                                                      LDAP_OPT_SUCCESS ) {
            if ( (msg != NULL) && (strlen(msg) != 0) )
             {
                printf("\n    LDAP server message: %s", msg);
                ldap_memfree(msg);
             }
        }
        /* print out LDAP error message */
        printf("    LDAP error  message: ldap_send_all_updates:" 
                            " %s (%d)\n", ldap_err2string( rc ), rc);
        
        printf("\n    ldap_send_all_updates failed\n\n");
        ldap_unbind_s( ld );
        return( 1 );
    }
           
    printf("\n    ldap_send_all_updates succeeded\n\n");
 
    ldap_unbind_s( ld );
    return 0;
}
