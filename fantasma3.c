#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "memoria.h"
#include "winsuport2.h"


#define MIN_FIL 7		/* definir limits de variables globals */
#define MAX_FIL 25
#define MIN_COL 10
#define MAX_COL 80
#define MAX_FANTASMES 9

				/* definir estructures d'informacio */
typedef struct {		/* per un objecte (menjacocos o fantasma) */
	int f;				/* posicio actual: fila */
	int c;				/* posicio actual: columna */
	int d;				/* direccio actual: [0..3] */
	float r;            	/* per indicar un retard relati */
	char a;			/* caracter anterior en pos. actual */
} objecte;

objecte f1[MAX_FANTASMES];	/* informacio del fantasma 1 */

int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */
int *p_fantasma, *p_fi1, *p_fi2;
 


/* funcio per moure un fantasma una posicio; retorna 1 si el fantasma   */
/* captura al menjacocos, 0 altrament					*/
int main (int n_args, char *ll_args[])
{
  int id_fant,id,n_fil1,n_col,retard;
  objecte seg;
  int k, vk, nd, vd[3]; 
  
  
  /*1-Mapeja referència memòria compartida utilitzant identificador*/
  id_fant = atoi(ll_args[1]);
  p_fantasma = map_mem(id_fant);
  	  
  p_fi1 = map_mem(atoi(ll_args[11]));
  p_fi2 = map_mem(atoi(ll_args[12]));
	  
	  	   
  if (p_fantasma == (int*) -1){
  	fprintf(stderr,"proces (%d): error en identificador de finestra\n",(int)getpid());
  	exit(0);
  }
	
  /*2- Invoca win_set amb l'adreça mapejada anteiorment i dimensions per arguments */  
  n_fil1 = atoi(ll_args[2]);
  n_col = atoi(ll_args[3]); 
  
  win_set(p_fantasma, n_fil1, n_col); 
  
  /*Info fantasma i retard joc*/
  id = atoi(ll_args[9]);
  f1[id].f = atoi(ll_args[4]);
  f1[id].c = atoi(ll_args[5]);
  f1[id].d = atoi(ll_args[6]);
  f1[id].r = atoi(ll_args[7]);
  f1[id].a = atoi(ll_args[8]);
  retard = atoi(ll_args[10]);
  
  /*3-Utilitzen funcions d'escriptura i consulta del camp */	  
  do{
  	*p_fi2=0; nd = 0;
	for (k=-1; k<=1; k++)		/* provar direccio actual i dir. veines */
	{
		vk = (f1[id].d + k) % 4;		/* direccio veina */
	    	if (vk < 0) vk += 4;		/* corregeix negatius */
	    	seg.f = f1[id].f + df[vk]; 		/* calcular posicio en la nova dir.*/
	     	seg.c = f1[id].c + dc[vk];
	   	seg.a = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
	    	if ((seg.a==' ') || (seg.a=='.') || (seg.a=='0'))
	    	{ vd[nd] = vk;			/* memoritza com a direccio possible */
	        nd++;
	    	}
	}
	if (nd == 0)				/* si no pot continuar, */
		f1[id].d = (f1[id].d + 2) % 4;		/* canvia totalment de sentit */
	else
	{ if (nd == 1)			/* si nomes pot en una direccio */
	  	f1[id].d = vd[0];			/* li assigna aquesta */
	  else				/* altrament */
	  	f1[id].d = vd[rand() % nd];		/* segueix una dir. aleatoria */
	    	
	seg.f = f1[id].f + df[f1[id].d];  /* calcular seguent posicio final */
	seg.c = f1[id].c + dc[f1[id].d];
	seg.a = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
	win_escricar(f1[id].f,f1[id].c,f1[id].a,NO_INV);	/* esborra posicio anterior */
	f1[id].f = seg.f; f1[id].c = seg.c; f1[id].a = seg.a;	/* actualitza posicio */
	win_escricar(f1[id].f,f1[id].c,'1'+id,NO_INV);		/* redibuixa fantasma */
	if (f1[id].a == '0') *p_fi1 = 1;		/* ha capturat menjacocos */
	}
	  
	win_retard(f1[id].r * retard); 
	  
  }while(!*p_fi1 && !*p_fi2);
  	
  return (0);
}


