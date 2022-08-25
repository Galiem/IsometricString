

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <sys/time.h>
#include <cstdint>

#include "divsufsort.h"
#include "bit_vectors.hpp"	  // include header for bit vectors
#include "rmq_support.hpp"

using namespace sdsl;
using namespace std;

typedef int32_t INT;

#define SOLONmax(a,b) ((a) > (b)) ? (a) : (b)
#define SOLONmin(a,b) ((a) < (b)) ? (a) : (b)

char * strPrefix(char * str, int r);


// Function that takes a string and returns its prefix of lenght r
char * strPrefix(char * str, int r){
    char * prefix = (char *) malloc(r * sizeof(char));
    for (int i = 0; i < r; i++)
    {
        prefix[i] = str[i];
    }
    return prefix;
}



unsigned int LCParray ( unsigned char *text, INT n, INT * SA, INT * ISA, INT * LCP )
{										
	INT i=0, j=0;

	LCP[0] = 0;
	for ( i = 0; i < n; i++ ) // compute LCP[ISA[i]]
		if ( ISA[i] != 0 ) 
		{
			if ( i == 0) j = 0;
			else j = (LCP[ISA[i-1]] >= 2) ? LCP[ISA[i-1]]-1 : 0;
			while ( text[i+j] == text[SA[ISA[i]-1]+j] )
				j++;
			LCP[ISA[i]] = j;
		}

	return ( 1 );
}

unsigned int initializeSuffixArray(unsigned char * word, INT * LCP){
    INT * SA;
	INT * invSA;

    INT N = strlen((char *) word);
    
	/* Compute the suffix array */

        SA = ( INT * ) malloc( ( N ) * sizeof( INT ) );
        if( ( SA == NULL) )
        {
                fprintf(stderr, " Error: Cannot allocate memory for SA.\n" );
                return ( 0 );
        }

        if( divsufsort(word, SA,  N ) != 0 )
        {
                fprintf(stderr, " Error: SA computation failed.\n" );
                exit( EXIT_FAILURE );
        }

        /*Compute the inverse SA array */
        invSA = ( INT * ) calloc( N , sizeof( INT ) );
        if( ( invSA == NULL) )
        {
                fprintf(stderr, " Error: Cannot allocate memory for invSA.\n" );
                return ( 0 );
        }

        for ( INT i = 0; i < N; i ++ )
        {
                invSA [SA[i]] = i;
        }

	LCP = ( INT * ) calloc  ( N, sizeof( INT ) );
        if( ( LCP == NULL) )
        {
                fprintf(stderr, " Error: Cannot allocate memory for LCP.\n" );
                return ( 0 );
        }

        /* Compute the LCP array */
        if( LCParray( word, N, SA, invSA, LCP ) != 1 )
        {
                fprintf(stderr, " Error: LCP computation failed.\n" );
                exit( EXIT_FAILURE );
        }
        
    return 1;
}

int leeDistance(unsigned char * a, unsigned char * b, int d){
    int distance = 0;
    cout<<"a: "<<a <<" b: "<<b <<endl;
    /*
    for (int i = 0; i < n-1; i++)
    {
       distance += abs((a[i] - '0') - (b[i] - '0')) <= (d - abs((a[i] - '0') - (b[i] - '0'))) ? abs((a[i] - '0') - (b[i] - '0')) : (d - abs((a[i] - '0') - (b[i] - '0')));
    }
*/

    distance += abs((a[0] - '0') - (b[0] - '0')) <= (d - abs((a[0] - '0') - (b[0] - '0'))) ? abs((a[0] - '0') - (b[0] - '0')) : (d - abs((a[0] - '0') - (b[0] - '0')));
    cout << "distance: " << distance << endl;

    return distance;
}

unsigned int LCPlenght (unsigned char * word, INT * LCP, INT l, INT r)
{
    INT * SA;
	INT * invSA;

    INT N = strlen((char *) word);
    
	/* Compute the suffix array */

        SA = ( INT * ) malloc( ( N ) * sizeof( INT ) );
        if( ( SA == NULL) )
        {
                fprintf(stderr, " Error: Cannot allocate memory for SA.\n" );
                return ( 0 );
        }

        if( divsufsort(word, SA,  N ) != 0 )
        {
                fprintf(stderr, " Error: SA computation failed.\n" );
                exit( EXIT_FAILURE );
        }

        /*Compute the inverse SA array */
        invSA = ( INT * ) calloc( N , sizeof( INT ) );
        if( ( invSA == NULL) )
        {
                fprintf(stderr, " Error: Cannot allocate memory for invSA.\n" );
                return ( 0 );
        }

        for ( INT i = 0; i < N; i ++ )
        {
                invSA [SA[i]] = i;
        }

        /* print SA array*/
        for(int i = 0; i < N; ++i) {
        printf("SA[%2d] = %2d: ", i, SA[i]);
        for(int j = SA[i]; j < N; ++j) {
            printf("%c", word[j]);
        }
        printf("$\n");
        }

        for(int i = 0; i < N; ++i) {
        printf("invSA[%2d] = %2d: ", i, invSA[i]);
        for(int j = invSA[i]; j < N; ++j) {
            printf("%c", word[j]);
        }
        printf("$\n");
        }

	LCP = ( INT * ) calloc  ( N, sizeof( INT ) );
        if( ( LCP == NULL) )
        {
                fprintf(stderr, " Error: Cannot allocate memory for LCP.\n" );
                return ( 0 );
        }

        /* Compute the LCP array */
        if( LCParray( word, N, SA, invSA, LCP ) != 1 )
        {
                fprintf(stderr, " Error: LCP computation failed.\n" );
                exit( EXIT_FAILURE );
        }

int_vector<> v( N , 0 ); // create a vector of length n and initialize it with 0s
    for ( INT i = 0; i < N; i ++ )
	{
		v[i] = LCP[i];
	}
    rmq_succinct_sct<> rmq(&v);
	util::clear(v);

    //print LCP array
    for ( INT i = 0; i < N; i ++ )
    {
        printf("LCP: %d ", LCP[i]);
    }

    int rq = rmq ( invSA[l]+1 , invSA[r] );
    printf("RMQ: %d ", rq);
	INT LCE = LCP[rmq ( invSA[l]+1 , invSA[r] )];

    int d = 0;
    l = 0;
    cout << "N: " << N << endl;
    for (int i = 1; i <= N - 1; i++)
    {
        l = 0;
        d = 0;
        while (d <= 2)
        {
            cout << "l before: " << l << endl;
            /*Io voglio trovare l'LCP tra il suffisso u con i e il suffisso u con j invece sto trovando l'lcp tra il suffisso
            in posizione i e il suffisso in posizione j del SA che è in ordine alfabetico*/
            
           
            if (i+l < N)
            {
                INT lmin = SOLONmin ( invSA[ l ], invSA[ i+l ] );
                INT rmax = SOLONmax ( invSA[ l ], invSA[ i+l] );

                cout << "lmin: " <<lmin <<" rmax: "<< rmax <<endl;
                l = l + LCP[rmq(lmin+1, rmax)];
                cout << "LCP: " << LCP[rmq(lmin+1, rmax)] <<endl;
            }
            
           

            
            
            cout << "l after: " << l << endl;
            cout << "N-i: " <<  N - i << endl;
           
            if ((d == 2) && (l == (N - i))){
                cout << "Ho trovato un 2-error-border" << endl;
                return (LCE);
            }
            if (d < 2 && (l == (N - i))){
                break;
            }
            if (d == 2 && (l < (N - i))){
                break;
            }
            cout<< "non ho breakato" << endl;
            d = d + leeDistance(word+l, word+l+i, 4);
            l++;
        }
        
    }
    

  	return (LCE);
}




int main(int argc, char const *argv[])
{
    char str[11];
    // create 10 random strings
    for (int i = 0; i < 2; i++)
    {
        // create a random string of length 10
        for (int j = 0; j < 10; j++)
        {
            // generate a random number between 0 and 25
            int random = rand() % 26;
            // convert the random number to a letter
            char letter = 'a' + random;
            str[j] = letter;
            // print the letter
            printf("%c", letter);
        }
        // print a new line
        printf("\n");
        char * prefix = strPrefix(str, 4);
        printf("Il suo prefisso è: %s\n", prefix);
    }

    unsigned char stringa[20];
    //strcpy((char *) stringa, "101011");
    //strcpy((char *) stringa, "124312413242132");
    //strcpy((char *) stringa, "0301");
    strcpy((char *) stringa, "132");
    //strcpy((char *) stringa, "3333333333");
    
    INT * LCP;

    if(initializeSuffixArray(stringa, LCP) != 1){
        printf("Errore nella creazione dell'array\n");
    }

    
    unsigned int result = LCPlenght(stringa, LCP, 0, 1);
    cout << "Lunghezza LCE: " << result << endl;
    

    return 0;
} 