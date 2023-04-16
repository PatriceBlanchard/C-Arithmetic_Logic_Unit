# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# define DEBUG if (0) fprintf

//Déclaration, définitions et initialisation
enum{MAX_MEM = 256};                                                                    // Le nombre d'index présent en mémoire
unsigned int *data;                                                                     // Un vecteur d'entiers oui seront enregistrés les opcodes et leur valeur
unsigned int taille = 0;                                                                // La taille du programme lu danns un fichier
unsigned int *ptr_taille = &taille;                                                     // Un pointeur sur taille

// Les prototypes de fonctions :
void traiter_ldc(int, char **);
void usage(char *, char *);
unsigned int charger_fichier(char *, unsigned int *);
void run(unsigned int, int);
void commandes(unsigned int, unsigned int, int *, unsigned int *);
int oui_non();
void help();
int stepper(unsigned int);
void enregistrer_fichier(unsigned int);

//Traitement du programme :
int main(int argc, char *argv[]){                                                       // Chargement du bootloader, et d'un programme passé en argument via la LDC
    data = malloc(sizeof(unsigned int *) * MAX_MEM);                                    // Allocation du vecteur data de taille MAX_MEM
    puts("Chargement du bootstrap...");
    //run(charger_fichier("paperboot.hexcode",ptr_taille), step_on);                    // Chargement du bootloader
    puts("Chargement du bootstrap terminé !\nDémarrage du programme...");
    traiter_ldc(argc, argv);                                                            // chargement du programme et de ses options suivant les arguments de la ldc
    return 0;
}

void traiter_ldc(int argc, char ** argv){                                               // Traitement des différents possible cas en ligne de commande
    int step_on;
    switch (argc){
        case 1 : 
            usage("","Usage : <nom du programme> [-d] <nom du fichier>\n"); break;
        case 2 : 
            if (!strcasecmp(argv[1],"-d")) 
                usage("","Usage : <nom du programme> [-d] <nom du fichier>\n");
            else {
                step_on = 0; 
                run(charger_fichier(argv[1], ptr_taille),step_on);                      // Chargement du programme sans stepper
            } 
            break;
        case 3 : 
            if (!strcasecmp(argv[1], "-d")){
                step_on = 1;
                run(charger_fichier(argv[2], ptr_taille),step_on);                      // Chargement du programme avec stepper
            }
            else 
                usage("","Usage : <nom du programme> [-d] <nom du fichier>\n");
        default :
            usage("","Usage : <nom du programme> [-d] <nom du fichier>\n");
    }
}

void usage(char *nom_fichier, char *message){                                           // Affichage un message d'erreur
    perror(nom_fichier);
    fprintf(stderr, message);
    exit(1);
}

unsigned int charger_fichier(char * fichier, unsigned int *ptr_taille){                 // Charger un fichier et initialise le vecteur data
    unsigned int offset;
    unsigned int i = 0;
    FILE *flux = fopen(fichier,"r");                                                    // Ouverture du fichier
    if (!flux) usage(fichier,"Une erreur s'est produite lors de l'ouverture du fichier\n");
    if(fscanf(flux , "%*s %X %*s", &offset) == EOF) usage(fichier, "Une erreur s'est produite lors de la lecture du fichier\n"); // Récupèrer l'offset en ligne 2 dans le fichier
    while(!feof (flux)) fscanf(flux, "%X", &data[offset + i++]);                        // Enregistrer les nombres sous forme hexadécimale dans le vecteur data
    *ptr_taille = i - 1;                                                                // Assigne la taille du programme 
    if (fclose(flux)) usage(fichier, "Echec lors de la fermeture du fichier, consulter la documentation utilisateur\n");
    for (int i = 0; i < 256; ++i)DEBUG(stderr,"%i.%X\n", i,data[i]);
    return offset;
}



void run (unsigned int PC, int step_on){                                                // Envoyer les opcodes présents dans le vecteur data à la fonction commandes pour interprétation
    int A = 0;                                                                          // Variable simulant la valeur du registre de l'accumulateur (A)
    int *ptr_A = &A;
    unsigned int cp_PC = PC;                                                            // une copie de la valeur simulant celle du Program Counter (PC)
    unsigned int *ptr_PC = &PC;
    puts("gdb - cx25.1\nVoulez-vous afficher la valeur de PC?");
    int test_PC = oui_non();                                                            // Affichage ou non de PC
    puts("Voulez-vous afficher les la valeur de A?");
    int test_A = oui_non();                                                             // Affichagez ou non de A
    if (step_on){puts("Voulez-vous afficher l'aide avant de commander ?"); if (oui_non()) help();}
    puts("\nDébut du programme :\n");
    getchar();
    while(PC < MAX_MEM + 1){                                                            // Boucler sur les opcodes présent dans le vecteur data
        
        if (test_PC && !test_A) printf("PC : %03i | ", PC);
        else if (test_A && !test_PC) printf("A : %03i | ", A);
        else if (!test_A && !test_PC) ;
        else printf("PC : %03i | A : %03i | ", PC, A);
        PC += 2;                                                                        // Incrémentation de 2 pour avoir accès à la prochaine instruction du Program Counter
        commandes(data[PC -2], data[(PC -2) + 1], ptr_A, ptr_PC);                       // Envoyer les opcodes et leur valeur à la fonction commandes
        printf("\n");
        if (step_on)stepper(cp_PC);                                                     // Affichage du stepper si step_on == 1
    
    }
}

int oui_non() {                                                                         // Cette fonction permet d'arrêter ou de continuer le programme, ou d'autoriser une option
    char option;
    while (1) {
        printf("(Continuer : O/N): ");
        if (scanf(" %c", &option) != 1) usage("","Une erreur s'est produite lors de la lecture de l'option\n");
        else if (option == 'o' || option == 'O') return 1;
        else if (option == 'n' || option == 'N') return 0;
        else printf("Seulement O ou N sont acceptables.\n");
  }
}

void help(){                                                                            // Cette fonction affiche l'aide
    puts("\n<Bienvenue dans l'aide de gdb cx35.1>\n");
    puts("Voici la liste des commandes : ");
    puts("\ndisplay <adresse>\nSignification : affiche la valeur à l adresse désignée.\nExemple d utilisation : diplay 45\n");
    puts("display all\nSignification : affiche toutes les valeurs de toutes les adresses du programme.\n");
    printf("store <adresse> <valeur>;\nSignification : enregistre une valeur saisie  en hexadecimal à l adresse designee. ");
    printf("Un fichier denommee : new_version est automatiquement cree avec la modification apportee.\nExemple d utilisation : store 40 2F\n\n");
    puts("quit");
    puts("Signification : Arrêt du programme.\n");
    puts("help");
    puts("Signification : afficher l'aide.");
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
    default: usage("","Erreur : commande inexistante\n"); exit(0);
    }
}

int stepper(unsigned int cp_PC){                                                        // Une fonction qui developpe quelques fonctionnalités de gdb
    char phrase[20];
    fgets(phrase, 20, stdin);                                                           // Saisir une commande
    int i = 0;
    char *mot[3];
    char * token = strtok(phrase, " " );                                                // split la phrase en un ou plusieurs mot

    while(token != NULL){
        mot[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    if (!strcmp(mot[0], "quit\n") && i == 1) exit(0);                                   // Si la commande saisie est : quit, le programme s'arrête
    else if (!strcmp(mot[0], "display") & i == 2){

        if (!strcasecmp(mot[1],"all\n")) {                                              // Si la commande saisie est : display all, le programme afiche l'ensemble du programme présent dans le vecteur data
            unsigned int j=cp_PC;
            for (int i = cp_PC; i < (cp_PC + *ptr_taille) ; ++i) fprintf(stdout, "data[%i] == %X\n", i, data[i]);
            getchar();
        }
        else{
            int adresse;
            adresse = strtoul(mot[1], NULL, 10);
            fprintf(stderr, "%X\n", data[adresse]);                                     // si la commande saisie est display suivie d'une adresse, le programme affiche la valeur à l'adresse concerné
            getchar();                                                                   
            return 0;
        }
    }
    else if (!strcmp(mot[0], "store") && i == 3){                                       // Si la commande saisie est store suivie d'une adresse, le programme enregistre la valeur à l'adresse choisie
        int adresse2;
        adresse2 = strtoul(mot[1], NULL, 10);
        unsigned int valeur = strtoul(mot[2], NULL, 16);
        data[adresse2] = valeur;
        enregistrer_fichier(cp_PC);                                                     // Suite à la modification de l'adresse, l'ensemble du vecteur data est enregistré dans un nouveau fichier
        getchar();
    }
    else if (!strcasecmp(mot[0], "help\n") && i == 1){                                  // Si la commande saisie est help, le programme affiche l'aide
        help();}
    else;
}

void enregistrer_fichier (unsigned int cp_PC){                                         // Cette fonction enregistre le vecteur data dans un fichier dénommé : new_version
    FILE* flux = fopen("new_version","w+");
    if (!flux) usage ("","Une erreur s'est produite lors de l'ouverture du fichier\n");
    fprintf(flux, "%s\n%X\n%s\n", "offset", cp_PC,"code");
    for (int i = cp_PC; i < (cp_PC + (taille-1)); i = i + 2) fprintf(flux, "%X %X\n", data[i], data[i + 1]);
    if (fclose(flux)) usage("new_version", "Echec lors de la fermeture du fichier, consulter la documentation utilisateur\n");
}