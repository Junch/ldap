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
    threadSafe.c
***************************************************************************
  Description: The threadSafe.c is a muilt-threaded prg for adding and 
	       searching objects , this program shows the usage of ldap_dup 
	       and ldap_destroy
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#ifndef _WINDOWS
#include <stdint.h>
#endif
#include <string.h>
#include <ldap.h>


#if defined (UNIX) || defined (LIBC)
#include <pthread.h>
#elif defined ( _WIN32)
#include <windows.h>
#include <process.h>    /* _beginthread, _endthread */
#endif

char        *searchBase;

void *Add(void *ld_ptr) ;
void *Search(void *ld_ptr) ;

#if defined(N_PLAT_NLM) && defined(LIBC)
#include <screen.h>
#endif

#if defined (CLIB)
 int thread_count=2;
#endif

static char usage[] =
"\n Usage:   threadSafe <host name> <port number> <login dn> <password>"
"\n\t  <continer name> \n" 
"\n Example: threadSafe Acme.com 389 cn=admin,o=Acme secret"
" ou=Sales,o=Acme\n";



int main( int argc, char **argv)
{
    int         version, ldapPort, rc; 
    char        *ldapHost, *loginDN, *password, *dn = NULL;     
    LDAP        *ld;
    LDAP *ld_clone,*ld_clone1 ;



    #if defined (_WIN32) || defined (CLIB)

     int handle[2];

    #elif defined (UNIX) || defined (LIBC)
    pthread_t searchThread[2] ;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    #endif

    #if defined(N_PLAT_NLM) && defined(LIBC)
    setscreenmode(SCR_NO_MODE);              /* Don't clear screen on exit */
    #endif



	if (argc != 6)
    	{             
        printf("%s", usage);
        return(1);          
    	}                       
                  
    ldapHost          = argv[1];
    ldapPort          = atoi(argv[2]);
    loginDN           = argv[3];
    password          = argv[4];
    searchBase        = argv[5];




    /* Set LDAP version to 3 and set connection timeout. */
    version = LDAP_VERSION3;
    ldap_set_option( NULL, LDAP_OPT_PROTOCOL_VERSION, &version);

    /* Initialize the LDAP session */
    if (( ld = (LDAP *)(intptr_t)ldap_init( ldapHost, ldapPort )) == NULL)
    {
        printf("\n\tLDAP session initialization failed\n");
        return( 1 );
    }
    printf ( "\n\tLDAP session initialized\n");

    /* Bind to the server */
    rc = ldap_simple_bind_s( ld, loginDN, password );
    if (rc != LDAP_SUCCESS )
    {
        printf("ldap_simple_bind_s: %s\n", ldap_err2string( rc ));
        ldap_unbind_s ( ld );
        return( 1 );
    }
    printf("\n\tBind successful\n");

/* duplicate ld using ldap_dup and swap the threads*/

	ld_clone = ldap_dup(ld) ;



	#if defined (UNIX) || defined (LIBC)


          if(pthread_create(&searchThread[0], &attr, Add, (void *)ld_clone)) {
             printf("\n Can't create  thread ");
                         return(-1);
                   }



	#elif defined (_WIN32)

         handle[0]= _beginthread( Add, 0, (void *) (ld_clone));

	#elif defined (CLIB) 

	  	 handle[0]=BeginThread( Add,  NULL,8192, (void *) (ld_clone));
	#endif

	ld_clone1 = ldap_dup(ld) ;



	#if defined (UNIX) || defined (LIBC)
         if(pthread_create(&searchThread[1], &attr, Search, (void *)ld_clone1)) {
              printf("\n Can't create thread ");
                return(-1);
           }


	#elif defined (_WIN32) 

         handle[1]= _beginthread( Search, 0, (void *) (ld_clone1));

	#elif defined (CLIB) 

	  	 handle[1]=BeginThread( Search,  NULL,8192, (void *) (ld_clone1));
	#endif


/* join the threads */




	#if defined (UNIX) || defined (LIBC)


           if( pthread_join(searchThread[0], NULL) == 0)
               printf("\n %d thread returned \n",searchThread[0]);

           if( pthread_join(searchThread[1], NULL) == 0)
               printf("\n %d thread returned \n",searchThread[1]);



	#elif defined (_WIN32)

	   if( WaitForSingleObject( (HANDLE) handle[0], INFINITE ) == 0)
               printf("\n %d thread returned  \n",handle[0]);


           if( WaitForSingleObject( (HANDLE) handle[1], INFINITE ) == 0)
               printf("\n %d thread returned  \n",handle[1]);



	#elif defined (CLIB)

	while(thread_count >0)
		ThreadSwitchWithDelay();

	#endif


        ldap_unbind(ld) ;
	return (0);
}

void *Add(void *ld)
{

    char        *vclass[5], *vcn[4], *vsn[2],*dn="cn=james,o=novell";
    LDAPMod     modClass, modCN,modSN;
    LDAPMod     *mods[8];
    int rc;

	printf("Add thread started\n");

    modClass.mod_op       =   LDAP_MOD_ADD;
    modClass.mod_type     =   "objectclass";
    vclass[0]             =   "inetOrgPerson";
    vclass[1]             =   NULL;
    modClass.mod_values   =   vclass;

    modCN.mod_op          =   LDAP_MOD_ADD;
    modCN.mod_type        =   "cn";
    vcn[0]                =   "james";
    vcn[1]                =   NULL;
    modCN.mod_values      =   vcn;

    modSN.mod_op          =   LDAP_MOD_ADD;
    modSN.mod_type        =   "sn";
    vsn[0]                =   "Smith";
    vsn[1]                =   NULL;
    modSN.mod_values      =   vsn;

    mods[0] = &modClass;
    mods[1] = &modCN;
    mods[2] = &modSN;
    mods[3] = NULL;


    rc = ldap_add_ext_s( ld,     /* LDAP session handle */ 
                         dn,     /* dn of the object to be created */
                         mods,   /* array of add options and values */
                         NULL,   /* server control */
                         NULL ); /* client control */


	if ( rc != LDAP_SUCCESS ) {
        	printf("\n\tldap_add_ext_s: %s\n", ldap_err2string( rc ));
	 	goto cleanup;
    	}
    
    	printf("\n\tEntry successfully added: %s\n", dn );

cleanup: 

	ldap_destroy(ld);

	#ifdef CLIB
	thread_count--;
	#endif

	return NULL;
}

void *Search(void *ld)
{

  int         rc,i,entryCount;
  LDAPMessage *searchResult, *entry;
  char *dn ,*attribute,**values;
  BerElement  *ber;

	printf("Search thread started \n");

  rc = ldap_search_ext_s(
                    ld,                    /* LDAP session handle */
                    searchBase,            /* container to search */
                    LDAP_SCOPE_SUBTREE,   /* search scope */
                    "(objectclass=*)",     /* search filter */
                    NULL,                  /* return all attributes */
                    0,                     /* return attributes and values */
                    NULL,                  /* server controls */
                    NULL,                  /* client controls */
                    NULL,              /* search timeout */
                    LDAP_NO_LIMIT,         /* no size limit */
                    &searchResult );


	
  if ( rc != LDAP_SUCCESS )
    {
        printf("ldap_search_ext_s: %s\n", ldap_err2string( rc ));
	goto cleanup;
    }
     

    /* Go through the search results by checking entries */
    for (   entry   =   ldap_first_entry( ld, searchResult );
            entry   !=  NULL;
            entry   =   ldap_next_entry( ld, entry ) )
    {
        if (( dn = ldap_get_dn( ld, entry )) != NULL )
        {
            printf("\n   dn: %s\n", dn );
            ldap_memfree( dn );
        }
       
        for (   attribute = ldap_first_attribute( ld, entry, &ber );
                attribute != NULL;
                attribute = ldap_next_attribute( ld, entry, ber ) )
        {
            /* Get values and print.  Assumes all values are strings. */
            if (( values = (char **)(intptr_t)ldap_get_values( ld, entry, attribute)) != NULL )
            {
                for ( i = 0; values[i] != NULL; i++ )
                    printf("        %s: %s\n", attribute, values[i] );
                ldap_value_free( values );
            }
            ldap_memfree( attribute );
        }

        ber_free(ber, 0);

    }
    entryCount = ldap_count_entries( ld, searchResult );

    printf("\n    Search completed successfully.\n    Entries  returned: %d\n",
             entryCount);


cleanup:
	ldap_msgfree( searchResult );
	ldap_destroy(ld);

	#ifdef CLIB
	thread_count--;
	#endif

	return NULL;

}
