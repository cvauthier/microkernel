#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <virux_lib.h>

char lirecaractere();

int main()
{
    int i = 0, a = 0, compteur = 10, o = 0, choixmot = 0, choixmot2 = 0;
    char Malettre, mot[30], motprintf[30];
    FILE* fichier = NULL;

    srand(gettime());
    
    a = 0;
    o = 0;
    i = 0;
    choixmot = 0;
    choixmot2 = 0;
    compteur = 10;
    printf("BIENVENUE AU PENDU !!\nLe but du jeu est de trouver un mot cache en ayant un nombre d'erreur limite.\n");
    choixmot = (rand() % (30 - 1 + 1)) + 1;
    fichier = fopen("dict.txt", "r");
    if (fichier != NULL)
    {
    while (choixmot2 != choixmot)
    {
				fgets(mot, 30, fichier);
        choixmot2++;
    }
		int x = strchr(mot,'\n')-mot;
		mot[x] = mot[x-1] = 0; // Enlever \n et \r
    fclose(fichier);
    }
    else
    {
        printf("Plantage du programme.");
        exit(0);
    }
    compteur = 10;
    while(o < strlen(mot))
    {
        motprintf[o] = '*';
        o++;
    }
    motprintf[o] = '\0';
    printf("%s\n", motprintf);
    do
    {

    if(compteur > 0)
    {
       printf("Vous avez  %d vies.\nEntrez une lettre.\n", compteur, motprintf);
       Malettre = lirecaractere();
        while (i < strlen(mot))
        {
            if (mot[i] == Malettre)
            {
               motprintf[i] = Malettre;
               a++;
            }
            i++;
        }
        i = 0;
        if (a == 0)
        {
            compteur--;
            printf("Rate, dommage !\n");
            printf("%s\n", motprintf);
        }
        else
        {
            if (strcmp(mot, motprintf) == 0)
            {
                printf("BRAVO, VOUS AVEZ TROUVE LE MOT MYSTERE !!!!!\n");
                printf("Il vous restait %d vies. \nEssayez de faire un sans faute la prochaine fois !\n", compteur);
            }
            else
            {
            printf("Bravo, reussi !\n");
            printf("%s\n", motprintf);
            }
        }
        a = 0;
    }
    else
    {
       printf("Vous n'avez plus de vies !!\n");
       printf("%s\n", mot);
       strcpy(motprintf, mot);

    }
    } while (strcmp(mot, motprintf) != 0);

    return 0;
}

char lirecaractere()
{
    char caractere = 0;

    caractere = getchar();
    caractere = (caractere >= 'a' && caractere <= 'z') ? caractere-'a'+'A' : caractere;

    while(getchar() != '\n');

    return caractere;
}

