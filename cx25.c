# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# define DEBUG if (0) fprintf

//Déclaration, définitions et initialisation
enum{MAX_MEM = 256};																	// Le nombre d'index présent en mémoire
unsigned int *data;																		// Un vecteur d'entiers oui seront enregistrés les opcodes et leur valeur

// Les prototypes de fonctions :
void usage(char *, char *);
unsigned int charger_fichier(char *);
void run(unsigned int);
void commandes(unsigned int, unsigned int, int *, unsigned int *);
int oui_non();

//Traitement du programme :
int main(int argc, char *argv[]){														// Charge un bootstrap et un fichier donné en argv[1]
    data = malloc(sizeof(unsigned int *) * MAX_MEM);									// Allocation du vecteur data de taille MAX_MEM
    if (argc != 2) usage("","Usage : <nom du programme> [-d] <nom du fichier>\n");
    puts("Chargement du bootstrap...");
    //run(charger_fichier("paperboot.hexcode"));										// Chargement du bootloader
    puts("Chargement du bootstrap terminé !\nDémarrage du programme...\n");
    run(charger_fichier(argv[1]));														// Chargement du programme
    return 0;
}

void usage(char *nom_fichier, char *message){											// Affichage un message d'erreur
	perror(nom_fichier);
	fprintf(stderr, message);
	exit(1);
}

unsigned int charger_fichier(char * fichier){											// Charger un fichier et initialise le vecteur data
	unsigned int offset;
	int i = 0;
	FILE *flux = fopen(fichier,"r");													// Ouverture du fichier
	if (!flux) usage(fichier,"Une erreur s'est produite lors de l'ouverture du fichier\n");
	if(fscanf(flux , "%*s %X %*s", &offset) == EOF) usage(fichier, "Une erreur s'est produite lors de la lecture du fichier\n"); // Récupèrer l'offset en ligne 2 dans le fichier
	while(!feof (flux)) fscanf(flux, "%X", &data[offset + i++]);						// Enregistrer les nombres sous forme hexadécimale dans le vecteur data
	if (fclose(flux)) usage(fichier, "Echec lors de la fermeture du fichier, consulter la documentation utilisateur\n");
	for (int i = 0; i < 256; ++i)DEBUG(stderr,"%i.%X\n", i,data[i]);
	return offset;
}

void run (unsigned int PC){																// Envoyer les opcodes présents dans le vecteur data à la fonction commandes pour interprétation
    int A = 0;																			// Variable simulant la valeur du registre de l'accumulateur (A)
    int *ptr_A = &A;
    unsigned int *ptr_PC = &PC;															// un pointeur sur PC, simulant la valeur du Program Counter
    while(PC < MAX_MEM){																// Boucler sur les opcodes présent dans le vecteur data
        printf("PC : %03X | A : %03i | ", PC, A);
        PC += 2;																		// Incrémenter de 2 pour avoir accès à la prochaine instruction du Programm Counter
        commandes(data[PC -2], data[(PC - 2) + 1], ptr_A, ptr_PC);						// Envoyer les opcodes et leur valeur à la fonction commandes
        printf("\n");
    }
}

void commandes(unsigned int opcode, unsigned int arg, int *ptr_A, unsigned int *ptr_PC){// Interpréter les opcodes reçus en argument et modifie en conséquence les pointeurs A et PC
    switch (opcode){
    case 0x00: printf("LOAD #%02X   A = %02X", arg, arg); *ptr_A = arg; break;
    case 0x10: printf("JUMP %02X    PC = %02X", arg, arg); * ptr_PC = arg; break;
    case 0x11: printf("BRN %02X     Si A < 0 alors PC = %02X", arg, arg); if (*ptr_A < 0) *ptr_PC = arg; break;
    case 0x12: printf("BRZ %02X     Si A = 0 alors PC = %02X", arg, arg); if (!*ptr_A) *ptr_PC = arg; break;
    case 0x20: printf("ADD #%02X    A += %02X", arg, arg); *ptr_A += arg; break;
    case 0x21: printf("SUB #%02X    A -= %02X", arg, arg); *ptr_A -= arg; break;
    case 0x22: printf("NAND #%02X   A = ~[A & %02X]", arg, arg); *ptr_A = (~(*ptr_A & arg))% MAX_MEM; break; 
    case 0x40: printf("LOAD %02X    A = data[%02X]", arg, arg); *ptr_A = data[arg]; break ;
    case 0x41: printf("OUT %02X     print(data[%02X])\nOUT en décimal : %i\n",arg, arg, data[arg]); if (!oui_non()) exit(0); else break;
    case 0x48: printf("STORE %02X    data[%02X] = A", arg, arg); data[arg] = *ptr_A; break;
    case 0x49: printf("IN %02X      data[%02X] = input(val ?)\nSaisir une valeur en décimal : ", arg, arg); if(!scanf("%i", &data[arg])) usage("","Erreur de saisie, la valeur doit être un chiffre\n"); break;
    case 0x60: printf("ADD %02X     A += data[%02X]", arg, arg); *ptr_A += data[arg]; break;
    case 0x61: printf("SUB %02X     A -= (data[%02X]", arg, arg); *ptr_A -= data[arg]; break;
    case 0x62: printf("NAND %02X    A = ~[A & %02X]", arg, arg); *ptr_A = (~(*ptr_A & data[arg]))% MAX_MEM; break;
    case 0xC0: printf("LOAD *%02X   A = data[data[%02X]]", arg, arg); *ptr_A = data[data[arg]]; break;
    case 0xC1: printf("OUT *%02X    print(data[data[%02X]]\nOUT en décimal - %i\n)", arg, arg, data[data[arg]]); if (!oui_non()) exit(0); else break;
    case 0xC8: printf("STORE *%02X  data[data[%02X]] = A", arg, arg); data[data[arg]] = *ptr_A; break;
    case 0xC9: printf("IN *%02X)    data[data[%02X]] = input(val ?)\n Saisir une valeur en décimal : ",  arg, arg); puts("val (en décimal) : "); if(!scanf("%i", &data[data[arg]])) usage("","Erreur de saisie : la valeur doit être un chiffre\n"); break;
    case 0xE0: printf("ADD *%02X    A += data[data[%02X]]", arg, arg); *ptr_A += data[data[arg]]; break;
    case 0xE1: printf("SUB *%02X    A -= data[data[%02X]]", arg, arg); *ptr_A -= data[data[arg]]; break;
    case 0xE2: printf("NAND *%02X   A = ~[A & data[data[%02X]]]", arg, arg); *ptr_A = (~(*ptr_A & data[data[arg]]))% MAX_MEM; break;
    default: usage("","Erreur : commande inexistante"); exit(0);
    }
}

int oui_non() { 																		// Cette fonction permet d'arrêter ou de continuer le programme ou autoriser une option
	char option;
	while (1) {
		printf("(Continuer : O/N): ");
		if (scanf(" %c", &option) != 1) usage("","Une erreur s'est produite lors de la lecture de l'option\n");
		else if (option == 'o' || option == 'O') return 1;
		else if (option == 'n' || option == 'N') return 0;
		else printf("Seulement O ou N sont acceptables.\n");
  }
}