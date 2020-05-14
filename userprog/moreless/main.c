#include <stdio.h>
#include <stdlib.h>

#include <virux_lib.h>

int main ( int argc, char** argv )
{
	int nombreMystere = 0, nombreEntre = 0, compteur = 0, eof = 0;
  const int MAX = 100, MIN = 1;

  srand(gettime());
  nombreMystere = (rand() % (MAX - MIN + 1)) + MIN;

  do
  {
  	compteur ++ ;
    printf("Quel est le nombre ? ");
    
		nombreEntre = 0;
		int c = getchar();
		while (c >= 0 && c != '\n')
		{
			nombreEntre = nombreEntre*10 + (c-'0');
			c = getchar();
		}
		eof = (c < 0);

		if (nombreMystere > nombreEntre)
    	printf("C'est plus !\n\n");
    else if (nombreMystere < nombreEntre)
      printf("C'est moins !\n\n");
    else
      printf ("Bravo, vous avez trouve le nombre mystere !!!\nVous avez trouve le nombre en %d coups.\n\n", compteur);
   } while (nombreEntre != nombreMystere && !eof);
   
	 return 0;
}


