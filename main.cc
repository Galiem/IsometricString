

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <sys/time.h>
#include <cstdint>


#include <divsufsort.h>                                           // include header for suffix sort

#include <sdsl/bit_vectors.hpp>					  // include header for bit vectors
#include <sdsl/rmq_support.hpp>	

using namespace sdsl;
using namespace std;

typedef int32_t INT;

#define SOLONmax(a,b) ((a) > (b)) ? (a) : (b)
#define SOLONmin(a,b) ((a) < (b)) ? (a) : (b)

//function signatures
char * strPrefix(char * str, int r);
INT twoErrorOverlaps(unsigned char * f, INT * twoeolens, INT * nTwoeolens, INT ** allerrpos,INT * totalErrors, INT * nAllerrpos, INT * LCP, INT * invSA, rmq_succinct_sct<> rmq);
unsigned int LCParray ( unsigned char *text, INT n, INT * SA, INT * ISA, INT * LCP );
int leeDistance(unsigned char a, unsigned char b, int d);
INT LCA(INT l, INT r, INT * invSA, INT * LCP, rmq_succinct_sct<> rmq);
bool condPlus(char * f, INT r, INT i, INT j, INT * LCP, INT * invSA, rmq_succinct_sct<> rmq);
INT stringBuilder(char * f, INT prefixLenght, string * s, char * addOn);
INT witnessesConstructor( char * f, INT l, INT * errpos, INT nErrpos, string * u, string * v, INT * LCP, INT * invSA, rmq_succinct_sct<> rmq);


int main(int argc, char const *argv[]){
    INT * SA;
	INT * invSA;
    INT * LCP;
    unsigned char word[128];
    //strcpy((char *) word, "03012");
    //strcpy((char *) word, "1243124132421323");
    //strcpy((char *) word, "030201");
    //strcpy((char *) word, "2302013022130");
    //strcpy((char *) word, "132103102");
    //strcpy((char *) word, "00000000000");
    bool flag = true;

    do {
        cout << endl << "Inserisci la parola (alfabeto da 0 a 3): ";
        cin >> word;
        flag = false;
        for (int i = 0; i < strlen((char *)word); i++)
        {
            if ((word[i] != '0' && word[i] != '1' && word[i] != '2' && word[i] != '3'))
            {
                cout << "La parola non è valida.";
                flag = true;
                break;
            }
            
        }   
    } while (flag);
    


    INT N = strlen((char *) word);
    
	//Compute the suffix array 

    SA = ( INT * ) malloc( ( N ) * sizeof( INT ) );
    if( SA == NULL )
    {
            fprintf(stderr, " Error: Cannot allocate memory for SA.\n" );
            return ( 0 );
    }

    if( divsufsort(word, SA,  N ) != 0 )
    {
            fprintf(stderr, " Error: SA computation failed.\n" );
            exit( EXIT_FAILURE );
    }

    //Compute the inverse SA array 
    invSA = ( INT * ) calloc( N , sizeof( INT ) );
    if( invSA == NULL )
    {
            fprintf(stderr, " Error: Cannot allocate memory for invSA.\n" );
            return ( 0 );
    }

    for ( INT i = 0; i < N; i ++ )
    {
            invSA [SA[i]] = i;
    }

    // print SA array
    for(int i = 0; i < N; ++i) {
        printf("SA[%2d] = %2d: ", i, SA[i]);
        for(int j = SA[i]; j < N; ++j) {
            printf("%c", word[j]);
        }
        printf("$\n");
    }

	LCP = ( INT * ) calloc  ( N, sizeof( INT ) );
    if( LCP == NULL )
    {
            fprintf(stderr, " Error: Cannot allocate memory for LCP.\n" );
            return ( 0 );
    }

    // Compute the LCP array 
    if( LCParray(word, N, SA, invSA, LCP ) != 1 )
    {
            fprintf(stderr, " Error: LCP computation failed.\n" );
            exit( EXIT_FAILURE );
    }
    
    int_vector<> vect( N , 0 ); // create a vector of length n and initialize it with 0s
    for ( INT i = 0; i < N; i ++ )
	{
		vect[i] = LCP[i];
	}
    rmq_succinct_sct<> rmq(&vect);
	util::clear(vect);

    //print LCP array
    for ( INT i = 0; i < N; i ++ )
    {
        printf("LCP: %d ", LCP[i]);
    }
    

    INT * twoeolens, *allerrpos[N], nTwoeolens, nAllerrpos[N], I, totalErrors;
    twoeolens = (INT *) malloc(N * sizeof(INT));
    if(twoeolens == NULL){
        fprintf(stderr, " Error: Cannot allocate memory for twoeolens.\n" );
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < N; i++)
    {
        allerrpos[i] = (INT *) malloc(N * sizeof(INT));
        if(allerrpos[i] == NULL){
            fprintf(stderr, " Error: Cannot allocate memory for allerrpos.\n" );
            exit(EXIT_FAILURE);
        }
    }
    
   
 
    if(twoErrorOverlaps(word, twoeolens, &nTwoeolens, allerrpos, &totalErrors, nAllerrpos, LCP, invSA, rmq) != 0){
        cout << "Errore nella ricerca dei 2-error-overlaps" << endl;
    }

    cout<< "nTwoeolens: " << nTwoeolens <<endl;
    for (int i = 0; i < nTwoeolens; i++)
    {
        cout<< "twoeolens["<< i <<"]: " << twoeolens[i];
        cout << " ";
    }
    cout << endl;
    
    for (int i = 0; i < totalErrors; i++)
    {
        cout << "allerrpos[" <<i << "]: ";
        for (int j = 0; j < nAllerrpos[i]; j++)
        {
            cout << "errpos[" << j<< "]: " << allerrpos[i][j] << " ";
        }
        cout << endl;
    }
    
    cout<< "nAllerpos: " << nAllerrpos[0] <<endl;

    string utmp, vtmp, u, v;

    if(nTwoeolens != 0){
        I = 2*N;
        cout << "word: " << (char *) word << endl;
        for ( INT i = 0; i < nTwoeolens; i ++ )
        {
            string utmp(2*N+1, '0'); //messi oltre l'upper bound di lenght per il caso 2 c
            string vtmp(2*N+1, '0');
            witnessesConstructor((char *) word, twoeolens[i], allerrpos[i], nAllerrpos[i], &utmp, &vtmp, LCP, invSA, rmq);
            if (utmp.length() < I){
                I = utmp.length();// Lee index è la lunghezza dei testimoni? nel codice originale viene messo u invece di utmp
                u = utmp;
                v = vtmp;
            }
        }

        
        cout << "I: " << I << endl;
        cout << "u: ";
        for (int i = 0; i < I; i++)
        {
            cout<< u[i];
        }
        cout << endl;

        cout << "v: ";
        for (int i = 0; i < I; i++)
        {
            cout<< v[i];
        }
        cout << endl;

    } else {
        printf("Non ci sono 2-error-overlaps\n");
    }
    return 0;
}

INT twoErrorOverlaps(unsigned char * f, INT * twoeolens, INT * nTwoeolens, INT ** allerrpos,INT * totalErrors, INT * nAllerrpos, INT * LCP, INT * invSA, rmq_succinct_sct<> rmq){
    INT n = strlen((char *) f);

    *nTwoeolens = 0;
    *nAllerrpos = 0;
    *totalErrors = 0;

    INT l = 0, d = 0;

    for (INT i = 1; i < n; i++){
        l = 0;
        d = 0;
        INT * allerrpostmp = (INT *) malloc(n * sizeof(INT));
        if(allerrpostmp == NULL){
            fprintf(stderr, " Error: Cannot allocate memory for allerpostmp.\n" );
            exit(EXIT_FAILURE);
        }
        INT nAllerrpostmp = 0;

        while (d <= 2){
            cout << "l:" << l << endl;
            if (i+l < n)
            {
                INT lca = LCA(l, i+l, invSA, LCP, rmq);
                l = l + lca;
                cout << "LCA(" << l << ", " << i+l << "): "<< lca <<endl;
            }

            cout << "l + LCA: " << l << endl;
            cout << "d: " << d << endl;

            if (l < n - i){
                allerrpostmp[nAllerrpostmp] = l;//+1
                nAllerrpostmp++;
            }
            if (d == 2 && l == n - i){
                cout<< "ho trovato un two error overlap" << endl;
                twoeolens[*nTwoeolens] = l;
                *(nTwoeolens) = *nTwoeolens + 1;
                std::copy(allerrpostmp, allerrpostmp + nAllerrpostmp, allerrpos[*totalErrors]);
                nAllerrpos[*totalErrors] = nAllerrpostmp;
                *totalErrors = *totalErrors + 1;
            }
            if(d < 2 && l < n - i){
                d = d + leeDistance(f[l], f[i+l], 4);
                l++;
            } else {
                break;
            }
        }
    }

    return 0;
}

INT witnessesConstructor( char * f, INT l, INT * errpos, INT nErrpos, string * u, string * v, INT * LCP, INT * invSA, rmq_succinct_sct<> rmq){
    INT n = strlen(f);
    INT r = n-l;
    INT i = errpos[0];

    if (nErrpos == 1){
        char * falfa1 = (char *) malloc((n+1) * sizeof(char));
        if(falfa1 == NULL){
            fprintf(stderr, " Error: Cannot allocate memory for falfa1.\n" );
            exit(EXIT_FAILURE);
        }
        char * fbeta1 = (char *) malloc((n+1) * sizeof(char));
        if(fbeta1 == NULL){
            fprintf(stderr, " Error: Cannot allocate memory for fbeta1.\n" );
            exit(EXIT_FAILURE);
        }
        strcpy(falfa1, f);
        strcpy(fbeta1, f);

        if(falfa1[i] != '0'){
            falfa1[i] = (((falfa1[i] - '0') - 1) % 4) + '0';
        } else {
            falfa1[i] = '3';
        }
             
        fbeta1[i] = (((fbeta1[i] - '0') + 1) % 4) + '0';

        if(stringBuilder(f, r, u, falfa1) != 0){
            fprintf(stderr, "Error: building the string for witnessConstructor");
            exit(EXIT_FAILURE);
        }

        if(stringBuilder(f, r, v, fbeta1) != 0){
            fprintf(stderr, "Error: building the string for witnessConstructor");
            exit(EXIT_FAILURE);
        }
    } else {
        INT j = errpos[1];
        bool cplus = condPlus(f, r, errpos[0], errpos[1], LCP, invSA, rmq);
        cout << "cplus: " << cplus <<endl;

        if (cplus == false){
            char * falfa = (char *) malloc (n * sizeof(char));
            char * fbeta = (char *) malloc (n * sizeof(char));
            strcpy(falfa, f);
            strcpy(fbeta, f);

            falfa[i] = f[r + i];
            fbeta[j] = f[r + j];

            if(stringBuilder(f, r, u, falfa) != 0){
                fprintf(stderr, "Error: building the string for witnessConstructor");
                exit(EXIT_FAILURE);
            }

            if(stringBuilder(f, r, v, fbeta) != 0){
                fprintf(stderr, "Error: building the string for witnessConstructor");
                exit(EXIT_FAILURE);
            }
            cout <<"u: " << *u << " - " << *v << " r: "<< r << endl;
        } else {
            cout<< "sono entrato a fare eta e gamma: "<< *u << " - " << *v << " r: "<< r << "i: " << i<< endl;
            if (i <= r/2){
                char * feta = (char *) malloc (n * sizeof(char));
                char * fgamma = (char *) malloc (n * sizeof(char));
                strcpy(feta, f);
                strcpy(fgamma, f);

                feta[i] = f[r + i];
                fgamma[j] = f[r + j];
                fgamma[(r/2) + j] = f[i];

                if(stringBuilder(f, r, u, feta) != 0){
                    fprintf(stderr, "Error: building the string for witnessConstructor");
                    exit(EXIT_FAILURE);
                }

                if(stringBuilder(f, r, v, fgamma) != 0){
                    fprintf(stderr, "Error: building the string for witnessConstructor");
                    exit(EXIT_FAILURE);
                }
                string suff(f+(n)-(r/2));

                *u = *u + suff;
                *v = *v + suff;// il suffisso nel codice dell'articolo non c'è, ho sbagliato io ad interpretare?
                
            }
        }

    }
    
    cout <<"alla fine u: " << *u << " - " << *v << " r: "<< r << endl;
    return 0;
}

bool condPlus(char * f, INT r, INT i, INT j, INT * LCP, INT * invSA, rmq_succinct_sct<> rmq){
    bool cond1 = false, cond2 = false, condr3 = false;
    INT n = strlen(f);

    if (r % 2 == 0){
        if (j-i == r/2){
            cond1 = true;
            if (f[r+i] == f[r+j]){
                cond2 = true;
                if (j < n)
                {
                    INT lca = LCA(i, j, invSA, LCP, rmq);
                    cout << "LCA(" << i << ", " << j << "): " << lca <<endl;
                    if( lca >= r/2){
                        condr3 = true;
                    } else if ( 0 == r/2){ //r può essere 0?
                        condr3 = true;
                    }
                }
            }
        }
    }

    return (cond1 && cond2 && condr3);
}


// Function that takes a string and returns its prefix of lenght r
char * strPrefix(char * str, int r){
    char * prefix = (char *) malloc(r * sizeof(char));
    if(prefix == NULL){
        fprintf(stderr, " Error: Cannot allocate memory for prefix.\n" );
        exit(EXIT_FAILURE);
    }
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


int leeDistance(unsigned char a, unsigned char b, int d){
    int distance = 0, aInt, bInt;
    cout<<"a: "<<a <<" b: "<<b <<endl;
    aInt = a - '0';
    bInt = b - '0';
    distance += abs(aInt - bInt) <= (d - abs(aInt - bInt)) ? abs(aInt - bInt) : (d - abs(aInt - bInt));
    cout << "distance: " << distance << endl;

    return distance;
}

INT LCA(INT l, INT r, INT * invSA, INT * LCP, rmq_succinct_sct<> rmq){
    INT lmin = SOLONmin ( invSA[ l ], invSA[ r ] );
    INT rmax = SOLONmax ( invSA[ l ], invSA[ r ] );
    return LCP[rmq(lmin+1, rmax)];
}

INT stringBuilder(char * f, INT prefixLenght, string * s, char * addOn){
    *s = strPrefix(f, prefixLenght);
    string suff(addOn);
    *s = *s + suff;
    return 0;
}