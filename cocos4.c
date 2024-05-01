/*****************************************************************************/
/*									                                         */
/*				     cocos2.c				                                 */
/*									                                         */
/*     Programa inicial d'exemple per a les practiques 2.1 i 2.2 de FSO.     */
/*     Es tracta del joc del menjacocos: es dibuixa un laberint amb una      */
/*     serie de punts (cocos), els quals han de ser "menjats" pel menja-     */
/*     cocos. Aquest menjacocos es representara amb el caracter '0', i el    */
/*     moura l'usuari amb les tecles 'w' (adalt), 's' (abaix), 'd' (dreta)   */
/*     i 'a' (esquerra). Simultaniament hi haura un conjunt de fantasmes,    */
/*     representats per numeros de l'1 al 9, que intentaran capturar al      */
/*     menjacocos. En la primera versio del programa, nomes hi ha un fan-    */
/*     tasma.								                                 */
/*     Evidentment, es tracta de menjar tots els punts abans que algun fan-  */
/*     tasma atrapi al menjacocos.					                         */
/*									                                         */
/*  Arguments del programa:						                             */
/*     per controlar la posicio de tots els elements del joc, cal indicar    */
/*     el nom d'un fitxer de text que contindra la seguent informacio:	     */
/*		n_fil1 n_col fit_tauler creq				                         */
/*		mc_f mc_c mc_d mc_r						                             */
/*		f1_f f1_c f1_d f1_r						                             */
/*									                                         */
/*     on 'n_fil1', 'n_col' son les dimensions del taulell de joc, mes una   */
/*     fila pels missatges de text a l'ultima linia. "fit_tauler" es el nom  */
/*     d'un fitxer de text que contindra el dibuix del laberint, amb num. de */
/*     files igual a 'n_fil1'-1 i num. de columnes igual a 'n_col'. Dins     */
/*     d'aquest fitxer, hi hauran caracter ASCCII que es representaran en    */
/*     pantalla tal qual, excepte el caracters iguals a 'creq', que es visua-*/
/*     litzaran invertits per representar la paret.			                 */
/*     Els parametres 'mc_f', 'mc_c' indiquen la posicio inicial de fila i   */
/*     columna del menjacocos, aixi com la direccio inicial de moviment      */
/*     (0 -> amunt, 1-> esquerra, 2-> avall, 3-> dreta). Els parametres	     */
/*     'f1_f', 'f1_c' i 'f1_d' corresponen a la mateixa informacio per al    */
/*     fantasma 1. El programa verifica que la primera posicio del menja-    */
/*     cocos o del fantasma no coincideixi amb un bloc de paret del laberint.*/
/*	   'mc_r' 'f1_r' son dos reals que multipliquen el retard del moviment.  */ 
/*     A mes, es podra afegir un segon argument opcional per indicar el      */
/*     retard de moviment del menjacocos i dels fantasmes (en ms);           */
/*     el valor per defecte d'aquest parametre es 100 (1 decima de segon).   */
/*									                                         */
/*  Compilar i executar:					  	                             */
/*     El programa invoca les funcions definides a 'winsuport.h', les        */
/*     quals proporcionen una interficie senzilla per crear una finestra     */
/*     de text on es poden escriure caracters en posicions especifiques de   */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser   */
/*     compilat amb la llibreria 'curses':				                     */
/*									                                         */
/*	   $ gcc cocos2.c winsuport.o -o cocos1 -lcurses -lpthread		             */
/*	   $ ./cocos2 fit_param [retard]				                         */
/*									                                         */
/*  Codis de retorn:						  	                             */
/*     El programa retorna algun dels seguents codis al SO:		             */
/*	0  ==>  funcionament normal					                             */
/*	1  ==>  numero d'arguments incorrecte 				                     */
/*	2  ==>  fitxer de configuracio no accessible			                 */
/*	3  ==>  dimensions del taulell incorrectes			                     */
/*	4  ==>  parametres del menjacocos incorrectes			                 */
/*	5  ==>  parametres d'algun fantasma incorrectes			                 */
/*	6  ==>  no s'ha pogut crear el camp de joc			                     */
/*	7  ==>  no s'ha pogut inicialitzar el joc			                     */
/*****************************************************************************/



#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> 
#include "memoria.h"
#include "missatge.h"
#include "semafor.h"
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


/* variables globals */
int compt=0, repetit=0, crear=0;
int n_fil1, n_col;		/* dimensions del camp de joc */
char tauler[70];		/* nom del fitxer amb el laberint de joc */
char c_req;			/* caracter de pared del laberint */
int num_fantasmes=0, temps; 
char strtemps[50], str_fi[30];

objecte mc;      		/* informacio del menjacocos */
objecte f1[MAX_FANTASMES];
pid_t tpid[MAX_FANTASMES]; 

int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */

int cocos;			/* numero restant de cocos per menjar */
int retard;		   	/* valor del retard de moviment, en mil.lisegons */

int *p_fi1, *p_fi2; 
int id_fi1, id_fi2;

pthread_mutex_t mutex;     	/* variable mutex */




/* funcio per realitzar la carrega dels parametres de joc emmagatzemats */
/* dins d'un fitxer de text, el nom del qual es passa per referencia a  */
/* 'nom_fit'; si es detecta algun problema, la funcio avorta l'execucio */
/* enviant un missatge per la sortida d'error i retornant el codi per-	*/
/* tinent al SO (segons comentaris al principi del programa).		    */
void carrega_parametres(const char *nom_fit)
{
  FILE *fit;

  fit = fopen(nom_fit,"rt");		/* intenta obrir fitxer */
  if (fit == NULL)
  {	fprintf(stderr,"No s'ha pogut obrir el fitxer \'%s\'\n",nom_fit);
  	exit(2);
  }

  if (!feof(fit)) fscanf(fit,"%d %d %s %c\n",&n_fil1,&n_col,tauler,&c_req);
  else {
	fprintf(stderr,"Falten parametres al fitxer \'%s\'\n",nom_fit);
	fclose(fit);
	exit(2);
	}
  if ((n_fil1 < MIN_FIL) || (n_fil1 > MAX_FIL) ||
	(n_col < MIN_COL) || (n_col > MAX_COL))
  {
	fprintf(stderr,"Error: dimensions del camp de joc incorrectes:\n");
	fprintf(stderr,"\t%d =< n_fil1 (%d) =< %d\n",MIN_FIL,n_fil1,MAX_FIL);
	fprintf(stderr,"\t%d =< n_col (%d) =< %d\n",MIN_COL,n_col,MAX_COL);
	fclose(fit);
	exit(3);
  }

  if (!feof(fit)) fscanf(fit,"%d %d %d %f\n",&mc.f,&mc.c,&mc.d,&mc.r);
  else {
	fprintf(stderr,"Falten parametres al fitxer \'%s\'\n",nom_fit);
	fclose(fit);
	exit(2);
	}
  if ((mc.f < 1) || (mc.f > n_fil1-3) ||
	(mc.c < 1) || (mc.c > n_col-2) ||
	(mc.d < 0) || (mc.d > 3))
  {
	fprintf(stderr,"Error: parametres menjacocos incorrectes:\n");
	fprintf(stderr,"\t1 =< mc.f (%d) =< n_fil1-3 (%d)\n",mc.f,(n_fil1-3));
	fprintf(stderr,"\t1 =< mc.c (%d) =< n_col-2 (%d)\n",mc.c,(n_col-2));
	fprintf(stderr,"\t0 =< mc.d (%d) =< 3\n",mc.d);
	fclose(fit);
	exit(4);
  } 
  int i=0; 
  do{
  	fscanf(fit,"%d %d %d %f\n",&f1[i].f,&f1[i].c,&f1[i].d,&f1[i].r);
      if ((f1[i].f < 1) || (f1[i].f > n_fil1-3) ||
	(f1[i].c < 1) || (f1[i].c > n_col-2) ||
	(f1[i].d < 0) || (f1[i].d > 3))
    {
	fprintf(stderr,"Error: parametres fantasma %d incorrectes:\n", i+1);
	fprintf(stderr,"\t1 =< f1.f (%d) =< n_fil1-3 (%d)\n",f1[i].f,(n_fil1-3));
	fprintf(stderr,"\t1 =< f1.c (%d) =< n_col-2 (%d)\n",f1[i].c,(n_col-2));
	fprintf(stderr,"\t0 =< f1.d (%d) =< 3\n",f1[i].d);
	fclose(fit);
	exit(5);
    }
  	i++;
  }while(!feof(fit));
  
  num_fantasmes = i; 
  
  if (num_fantasmes==0){
      fprintf(stderr,"Falten parametres al fitxer \'%s\'\n",nom_fit);
      fclose(fit); 
      exit(2); 
      } 
  
  fclose(fit);			/* fitxer carregat: tot OK! */
  printf("Joc del MenjaCocos\n\tTecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
		TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
  printf("prem una tecla per continuar:\n");
  getchar();
}



/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(void)
{
  int r,i,j;
  
  r = win_carregatauler(tauler,n_fil1-1,n_col,c_req);
  
  if (r == 0)
  {
    mc.a = win_quincar(mc.f,mc.c);
    if (mc.a == c_req) r = -6;		/* error: menjacocos sobre pared */
    else
    {
      for (int i=0; i < num_fantasmes; i++){
	       f1[i].a = win_quincar(f1[i].f,f1[i].c);
	       if (f1[i].a == c_req) {
	       	 r = -7;	/* error: fantasma sobre pared */	
	       	 break; 
	  }
      }
      		cocos = 0;			/* compta el numero total de cocos */
		for (i=0; i<n_fil1-1; i++)
		  for (j=0; j<n_col; j++)
		    if (win_quincar(i,j)=='.') cocos++;
		  
		win_escricar(mc.f,mc.c,'0',NO_INV);
		
		for (int i=0; i < num_fantasmes; i++){
			win_escricar(f1[i].f,f1[i].c,'1'+i,NO_INV);
		}

        if (mc.a == '.') cocos--;	/* menja primer coco */
       }
    }
  if (r != 0)
  {	
  	win_fi();
	fprintf(stderr,"Error: no s'ha pogut inicialitzar el joc:\n");
	switch (r)
	{ case -1: fprintf(stderr,"  nom de fitxer erroni\n"); break;
	  case -2: fprintf(stderr,"  numero de columnes d'alguna fila no coincideix amb l'amplada del tauler de joc\n"); break;
	  case -3: fprintf(stderr,"  numero de columnes del laberint incorrecte\n"); break;
	  case -4: fprintf(stderr,"  numero de files del laberint incorrecte\n"); break;
	  case -5: fprintf(stderr,"  finestra de camp de joc no oberta\n"); break;
	  case -6: fprintf(stderr,"  posicio inicial del menjacocos damunt la pared del laberint\n"); break;
	  case -7: fprintf(stderr,"  posicio inicial del fantasma damunt la pared del laberint\n"); break;
	}
	exit(7);
  }
}

void * control_temps(void *null)
{  
  while (!*p_fi1 && !*p_fi2){
	  temps++;
	  int minuts = temps/60; 
    	  int segons = temps % 60;
    	  sprintf(str_fi,"%02d:%02d", minuts, segons);   
	  sprintf(strtemps, "%02d:%02d|Cocos: %d", minuts, segons, cocos);
	  win_escristr(strtemps);  
	  sleep(1); 
  }
  pthread_exit(NULL); 
}




/* funcio per moure el menjacocos una posicio, en funcio de la direccio de   */
/* moviment actual; retorna -1 si s'ha premut RETURN, 1 si s'ha menjat tots  */
/* els cocos, i 0 altrament */
void *mou_menjacocos(void*null)
{
  objecte seg;
  int tec; 
  
  pthread_mutex_lock(&mutex); 
  tec = win_gettec();
  pthread_mutex_unlock(&mutex); 

  do{
	  if (tec != 0)
	   switch (tec)		/* modificar direccio menjacocos segons tecla */
	   {
	    case TEC_AMUNT:	  mc.d = 0; break;
	    case TEC_ESQUER:  mc.d = 1; break;
	    case TEC_AVALL:	  mc.d = 2; break;
	    case TEC_DRETA:	  mc.d = 3; break;
	    case TEC_RETURN:  *p_fi2 = -1; break;
	   }
	  
	  seg.f = mc.f + df[mc.d];	/* calcular seguent posicio */
	  seg.c = mc.c + dc[mc.d];
	  pthread_mutex_lock(&mutex); 
	  seg.a = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */ 
	  pthread_mutex_unlock(&mutex);
	  
	  if (seg.a == '+'){
	  	if (repetit != 1){ 
	  		compt++;
	  		if (compt == 2){
	  		crear = 1;
	  		compt = 0;
	  		} 
	  		repetit=1;

	  	}
	  }else{
	  	repetit=0; 
	  	if ((seg.a == ' ') || (seg.a == '.'))
		{ 
			win_escricar(mc.f,mc.c,' ',NO_INV);		/* esborra posicio anterior */
			mc.f = seg.f; mc.c = seg.c;			/* actualitza posicio */
			win_escricar(mc.f,mc.c,'0',NO_INV);		/* redibuixa menjacocos */
			if (seg.a == '.')
			{
				cocos--;
				if (cocos == 0) *p_fi2 = 1;
			}
		}
	  }
	  
	  
	    
	  pthread_mutex_unlock(&mutex); 
	  
	  
	  win_retard(retard*mc.r); 
	  
	  pthread_mutex_lock(&mutex); 
	  tec = win_gettec(); 
	  pthread_mutex_unlock(&mutex); 
	  
  }while(!*p_fi1 && !*p_fi2); 
  
  pthread_exit(NULL);  
}


/* programa principal				    */
int main(int n_args, const char *ll_args[])
{
  /* variables locals */ 
  int id_fantasma,n, mida_camp, id_sem, id_bustia, i=0;  
  char id_fant[20], str_files[30], str_columnes[30],str_fi1[20], str_fi2[20], ret_joc[30], semafor[20], bustia[20];
  void *p_fantasma;
  pthread_t tid_mc, tid_temps; 

  srand(getpid());		/* inicialitza numeros aleatoris */

  if ((n_args != 2) && (n_args !=3))
  {	fprintf(stderr,"Comanda: cocos0 fit_param [retard]\n");
  	exit(1);
  }
  carrega_parametres(ll_args[1]);

  if (n_args == 3) retard = atoi(ll_args[2]);
  else retard = 100;
  
  /* 1- Invoca a win_ini */
  mida_camp = win_ini(&n_fil1,&n_col,'+',INVERS);	/* intenta crear taulell */
 
 
  /* 2- Crea zona memoria compartida */
  id_fantasma = ini_mem(mida_camp); 
  p_fantasma = map_mem(id_fantasma);
	  
  id_fi1 = ini_mem(mida_camp); 
  p_fi1 = map_mem(id_fi1); 
  *p_fi1=0; 
	  
  id_fi2 = ini_mem(mida_camp); 
  p_fi2 = map_mem(id_fi2);
  *p_fi2=0; 

  
  
  /*3 - Invoca win_set() */
  win_set(p_fantasma, n_fil1, n_col); 
	    
	   
  if (mida_camp > 0){	
	   
  	inicialitza_joc();
	   
	*p_fi1=0; *p_fi2=0; 
	
	id_sem = ini_sem(1); 
  	sprintf(semafor, "%i", id_sem); 
  
  	id_bustia = ini_sem(1); 
  	sprintf(bustia, "%i", id_bustia);
  	
  	pthread_mutex_init(&mutex, NULL);	/* inicialitza el semafor */
		    	
	pthread_create(&tid_mc, NULL, mou_menjacocos, NULL);
	pthread_create(&tid_temps, NULL, control_temps, NULL);
	 
	/* 4- Crea processos fill amb identificador, n_files, n_columnes */ 
	sprintf(id_fant, "%i", id_fantasma);
	sprintf(str_files, "%i", n_fil1);
	sprintf(str_columnes, "%i", n_col);
	sprintf(ret_joc, "%i", retard);
	sprintf(str_fi1, "%i", id_fi1); 
	sprintf(str_fi2, "%i", id_fi2);
	n=0;   
	char fil_fant[20],col_fant[20],dir_fant[20],ret_fant[20],act_fant[20], id[20];

	do{ 
	if(crear == 1 || i==0){
		if (i<num_fantasmes){
			tpid[n] = fork(); 
			if (tpid[n] == (pid_t) 0){  
				sprintf(fil_fant, "%i", f1[i].f); 
				sprintf(col_fant, "%i", f1[i].c);  
				sprintf(dir_fant, "%i", f1[i].d);  
				sprintf(ret_fant, "%f", f1[i].r);
				sprintf(act_fant, "%i", f1[i].a); 
				sprintf(id, "%i",i);
				
				execlp("./fantasma4","fantasma4", id_fant, str_files, str_columnes,fil_fant, col_fant, dir_fant, ret_fant, act_fant,id,ret_joc,str_fi1, str_fi2, semafor, bustia,(char*)0); 
				exit(0); 
			}
			else if (tpid[n] > 0) n++;
			i++;
			crear=0; 
		}
	}
		
	/* 5- Executa un bucle principal cada 100 ms, invocant win_update */
		win_update(); 
		//fprintf(stderr,"%d", compt);
		win_retard(100); 
	}while(!*p_fi1 && !*p_fi2);
	
	for (int i=0; i<n; i++){
		waitpid(tpid[i], NULL, 0); 
	}
	
	pthread_join(tid_mc, NULL);
	pthread_join(tid_temps, NULL); 

	
	win_fi();  
	pthread_mutex_destroy(&mutex);        /* destrueix el semafor */ 
	elim_sem(id_sem); 
	elim_mem(id_fantasma);		/* elimina zones de memoria compartida */
		    
	printf("Temps de joc: %s\n", str_fi); 
    	printf("Cocos restants: %d\n", cocos); 
    	
	if (*p_fi2 == -1) printf("S'ha aturat el joc amb tecla RETURN!\n");
	else { if (!*p_fi1) printf("Ha guanyat l'usuari!\n");
		else printf("Ha guanyat l'ordinador!\n"); }

  }
  return(0);
}
