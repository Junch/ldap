/* $Novell: starttls.c,v 1.7 2003/05/12 13:10:19 $ */
/**************************************************************************
*  Novell Software Developer Kit
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
    starttls.c
***************************************************************************
   Description: strt_tls.c demonstrates how to use the ldapssl_start_tls
   and ldapssl_stop_tls functions in the ldapssl library. The use of
   ldapssl_set_verify_mode(LDAPSSL_VERIFY_NONE) is for simplicity. It 
   allows this sample code to accept any certificate from an LDAP server.
   
   NOTE:  This sample requires the server to support Start/Stop TLS.
          Start/Stop TLS support was introduced in eDirectory 8.7.
***************************************************************************/

#ifdef _WINDOWS
#include <winsock2.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#ifndef _WINDOWS
#include <stdint.h>
#endif
#include <ldap_ssl.h>

#if defined(N_PLAT_NLM) && defined(LIBC)
#include <screen.h>
#endif

static char usage[] =
"\n Usage:   starttls <host name> [<port number> [<login dn> <password>]]\n"
"  host name      = ldap server name or IP address\n"
"  port number    = port to use - 389 default (clear text)\n" 
"  login dn       = user name to login as\n"
"  password       = user password\n\n"
"Examples:\n"
"          starttls www.openldap.org\n"
"          starttls www.openldap.org 389\n"
"          starttls Acme.com 389 cn=admin,o=Acme secret\n";

int doSimpleSearch
(
   LDAP *ld
);

int main(int argc, char *argv[])
{
    int   rc       = 0;
    int   version  = LDAP_VERSION3;
    int   ldapPort = 389;
    char *ldapHost = NULL;
    char *loginDN  = NULL;
    char *password = NULL;
    LDAP *ld       = NULL;
    struct timeval timeOut = {10,0};   /* 10 second connection timeout */

    #if defined(N_PLAT_NLM) && defined(LIBC)
    setscreenmode(SCR_NO_MODE);              /* Don't clear screen on exit */
    #endif
    
    /*
     * Check for proper parameters
     */
    if (2 != argc && 3 != argc && 5 != argc)
    {
        printf("%s", usage);
        exit(1);
    }

    ldapHost = argv[1];
    
    /* If there are 3 or more arguments, get the port number.
     */
    if (3 <= argc)
    {
        ldapPort = atoi(argv[2]);
    }
    /* If there are 5 arguments, get the login dn and password.
     */
    if (5 == argc)
    {
        loginDN  = argv[3];
        password = argv[4];
    }

    /* Set LDAP version to 3 and set connection timeout. */
    ldap_set_option( NULL, LDAP_OPT_PROTOCOL_VERSION, &version);
    ldap_set_option( NULL, LDAP_OPT_NETWORK_TIMEOUT, &timeOut);
    
    /* 
     * Create an SSL enabled LDAP handle
     */
    ld = (LDAP *)(intptr_t)ldap_init(ldapHost,       /* host name */
                      ldapPort);       /* port number */

    if (NULL == ld)
    {
        printf("ldap_init error\n");
        ldapssl_client_deinit();
        exit(1);
    }

    /*
     * Bind (connect) to the LDAP server.
     */
    rc = ldap_simple_bind_s(ld, loginDN, password);
    if (LDAP_SUCCESS != rc)
    {
        printf("ldap_simple_bind_s error: %d, %s\n",
               rc, ldap_err2string(rc));
        ldap_unbind_s(ld);
        ldapssl_client_deinit();
        exit(1);
    }
    
    printf("Bind successful.  Do a simple search.\n");
      
    
    rc = doSimpleSearch(ld);
    if (LDAP_SUCCESS != rc)
    {
        printf("doSimpleSearch error: %d, %s\n",
               rc, ldap_err2string(rc));
        ldap_unbind_s(ld);
        ldapssl_client_deinit();
        exit(1);
    }

    /*
     * Start TLS.
     */
    printf("\nNow start TLS.\n");
    rc = ldap_start_tls_s(ld, NULL, NULL);

    if (LDAP_SUCCESS != rc)
    {
       printf("ldap_start_tls_s error: %d, %s\n",
              rc, ldap_err2string(rc));
       ldap_unbind_s(ld);
       ldapssl_client_deinit();
       exit(1);
    }
    printf("Start TLS successful\n");
    printf("Performing search\n");
    rc = doSimpleSearch(ld);
    if (LDAP_SUCCESS != rc)
    {
        printf("doSimpleSearch error: %d, %s\n", rc, ldap_err2string(rc));
        ldap_unbind_s(ld);
        ldapssl_client_deinit();
        exit(1);
    }
    printf("\nStop TLS successful\n");
    printf("Performing search\n");
    rc = doSimpleSearch(ld);
    if (LDAP_SUCCESS != rc)
    {
        printf("doSimpleSearch error: %d, %s\n", rc, ldap_err2string(rc));
        ldap_unbind_s(ld);
        exit(1);
    }
       
    /*
     * Unbind
     */
    printf("\nUnbind and deinit\n");
    ldap_unbind_s(ld);

    return 0;
}

/* 
 * doSimpleSearch - just to verify things worked
 */
int doSimpleSearch(LDAP *ld)
{
    int          rc        = LDAP_SUCCESS;
    LDAPMessage *resultBuf = NULL;
    LDAPMessage *entry     = NULL;
    BerElement  *ber       = NULL;   
    char        *attribute = NULL;
    char        **values   = NULL;

    /* Reading the root DSE
     */
    rc = ldap_search_ext_s(ld, "" , LDAP_SCOPE_BASE, "(objectClass=*)",
                           NULL, 0,
                           NULL, NULL, NULL, LDAP_NO_LIMIT, &resultBuf);

    if(LDAP_SUCCESS == rc)
    {
       entry = ldap_first_entry(ld, resultBuf);

       if(entry != NULL)
       {          
          for(attribute = ldap_first_attribute(ld, entry, &ber);
              attribute != NULL; 
              attribute = ldap_next_attribute(ld, entry, ber))
          {
              /* Print only the first value
               */              
              if((values = (char **)(intptr_t)ldap_get_values(ld, entry, attribute)) != NULL)
              {
                 printf("  %s: %s\n", attribute, values[0]);
                 ldap_value_free(values);
                 ldap_memfree(attribute);
              }        
          }
          /* Zero means free memory
           */
          ber_free(ber, 0);
       }

       ldap_msgfree(resultBuf);
    }
    else
    {
       printf("ldap_search_ext_s error: %d, %s\n", rc, ldap_err2string(rc));
    }
    return rc;
}
