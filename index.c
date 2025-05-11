#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_MOT 30
#define MAX_UTILISATEURS 100
#define MAX_NIVEAUX 4
#define MAX_MOTS_PAR_NIVEAU 100

typedef struct Utilisateur {
    char nom[160];
    char prenom[160];
    char mot_de_passe[60];
    int age;
    int niveau;
    int score;
    struct Utilisateur *suivant;
} Utilisateur;

Utilisateur *liste_utilisateurs = NULL;

void vider_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void str_to_lower(char *str) {
    for (int i = 0; str[i]; i++)
        str[i] = tolower((unsigned char)str[i]);
}

void ajouter_utilisateur(Utilisateur *user) {
    user->suivant = liste_utilisateurs;
    liste_utilisateurs = user;
}

Utilisateur* trouver_utilisateur(const char *nom) {
    Utilisateur *courant = liste_utilisateurs;
    while (courant) {
        if (strcmp(courant->nom, nom) == 0)
            return courant;
        courant = courant->suivant;
    }
    return NULL;
}

void charger_utilisateurs() {
    FILE *f = fopen("users.dat", "rb");
    if (!f) return;

    Utilisateur tmp;
    while(fread(&tmp, sizeof(Utilisateur), 1, f) == 1) {
        Utilisateur *nouveau = malloc(sizeof(Utilisateur));
        if (!nouveau) {
            fclose(f);
            return;
        }
        *nouveau = tmp;
        nouveau->suivant = NULL;
        ajouter_utilisateur(nouveau);
    }
    fclose(f);
}

void sauvegarder_utilisateurs() {
    FILE *f = fopen("user.dat", "wb");
    if (!f) return;

    Utilisateur *courant = liste_utilisateurs;
    while (courant) {
        fwrite(courant, sizeof(Utilisateur), 1, f);
        courant = courant->suivant;
    }
    fclose(f);
}

void liberer_utilisateurs() {
    Utilisateur *courant = liste_utilisateurs;
    while (courant) {
        Utilisateur *tmp = courant;
        courant = courant->suivant;
        free(tmp);
    }
    liste_utilisateurs = NULL;
}

Utilisateur* creer_utilisateur() {
    Utilisateur *user = malloc(sizeof(Utilisateur));
    if (!user) {
        printf("Erreur .\n");
        exit(1);
    }
    printf("Creation de compte:\n");
    while(1) {
        printf("Votre nom : ");
        if (!fgets(user->nom, sizeof(user->nom), stdin)) continue;
        user->nom[strcspn(user->nom, "\n")] = 0;
        if (strlen(user->nom) == 0) {
            printf("Le nom ne peut pas être vide.\n");
            continue;
        }
        if (trouver_utilisateur(user->nom)) {
            printf("Ce nom est deja utilise. Veuillez en choisir un autre.\n");
            continue;
        }
        break;
    }

    printf("Votre prenom: ");
    fgets(user->prenom, sizeof(user->prenom), stdin);
    user->prenom[strcspn(user->prenom, "\n")] = 0;

    char mdp1[60], mdp2[60];
    while(1) {
        printf("Votre mot de passe: ");
        fgets(mdp1, sizeof(mdp1), stdin);
        mdp1[strcspn(mdp1, "\n")] = 0;

        printf("Confirmez le mot de passe: ");
        fgets(mdp2, sizeof(mdp2), stdin);
        mdp2[strcspn(mdp2, "\n")] = 0;
        if (strcmp(mdp1, mdp2) != 0) {
            printf("Les mots de passe ne correspondent pas.\n");
            continue;
        }
        strcpy(user->mot_de_passe, mdp1);
        break;
    }

    printf("Votre age: ");
    while(scanf("%d", &user->age) != 1 || user->age <= 0 || user->age > 120) {
        vider_buffer();
        printf("age invalide, reessayez: ");
    }
    vider_buffer();

    user->niveau = 1;
    user->score = 0;
    user->suivant = NULL;

    ajouter_utilisateur(user);
    sauvegarder_utilisateurs();

    printf("Compte cree avec succes !\n");
    return user;
}

Utilisateur* connexion_utilisateur() {
    char nom[160];
    char mdp[60];
    printf("Connexion utilisateur:\n");
    printf("Nom: ");
    fgets(nom, sizeof(nom), stdin);
    nom[strcspn(nom, "\n")] = 0;

    Utilisateur *user = trouver_utilisateur(nom);
    if (!user) {
        printf("Utilisateur non trouve.\n");
        return NULL;
    }

    printf("Mot de passe : ");
    fgets(mdp, sizeof(mdp), stdin);
    mdp[strcspn(mdp, "\n")] = 0;
    if (strcmp(mdp, user->mot_de_passe) == 0) {
        printf("Bienvenue %s %s !\n", user->prenom, user->nom);
        return user;
    } else {
        printf("Mot de passe incorrect.\n");
    }

    return NULL;
}

int mot_repete(char mots[][MAX_MOT], int n, const char *mot) {
    for (int i = 0; i < n; i++) {
        if (strcmp(mots[i], mot) == 0) {
            return 1;
        }
    }
    return 0;
}

int mot_valide(const char *mot, int niveau) {
    char niveau_fichier[50];
    switch(niveau) {
        case 1: strcpy(niveau_fichier, "niveau1.txt"); break;
        case 2: strcpy(niveau_fichier, "niveau2.txt"); break;
        case 3: strcpy(niveau_fichier, "niveau3.txt"); break;
        case 4: strcpy(niveau_fichier, "niveau4.txt"); break;
        default:
            return 0;
    }

    FILE *f = fopen(niveau_fichier, "r");
    if (!f) {
        printf("Impossible d'ouvrir %s.\n", niveau_fichier);
        return 0;
    }
    char buffer[MAX_MOT];
    int trouve = 0;
    while (fgets(buffer, sizeof(buffer), f)) {
        buffer[strcspn(buffer, "\r\n")] = 0;
        str_to_lower(buffer);
        if (strcmp(mot, buffer) == 0) {
            trouve = 1;
            break;
        }
    }
    fclose(f);
    return trouve;
}

int choi_niveau(int niveau) {
    printf("Options pour le niveau %d:\n", niveau);
    printf("1. Continuer le niveau actuel\n");
    if (niveau > 1) printf("2. Retour au niveau precedent\n");
    printf("3. Recommencer ce niveau\n");
    printf("Votre choix: ");
    int choix;
    if (scanf("%d", &choix) != 1) {
        vider_buffer();
        return 0;
    }
    vider_buffer();
    return choix;
}

int verifier_mot_lettres(const char *mot, int longueur_mot) {
    if ((int)strlen(mot) != longueur_mot) return 0;
    for (int i = 0; mot[i]; i++) {
        if (!isalpha((unsigned char)mot[i]))
            return 0;
    }
    return 1;
}

void sauvegarder_progression(Utilisateur *user) {
    Utilisateur *trouve = trouver_utilisateur(user->nom);
    if (trouve) {
        *trouve = *user;
    }
    sauvegarder_utilisateurs();
}

void intro() {
    printf("              BIENVENUE DANS LEXIWORDS           \n\n");
    printf("  Developpe par 'HAMI WISSAM' et 'IDIR CELINE'\n\n");
    printf("  Chaque bonne reponse rapporte 20 points. Tapez 'q' pour arreter le jeu.\n\n");
    printf("----------------------------------------------------------------------------------\n\n\n");
}

void game(Utilisateur *user) {
    intro();
    printf("Salut %s %s, niveau %d, score %d\n", user->prenom, user->nom, user->niveau, user->score);

    int niveau = user->niveau;
    const int t_mots[] = {4, 5, 6, 7};
    const int nbr_mots = 10;
    char mots_utilisee[MAX_MOTS_PAR_NIVEAU][MAX_MOT];

    while (niveau <= MAX_NIVEAUX) {
        int option = choi_niveau(niveau);
        if (option == 2 && niveau > 1) {
            niveau--;
            printf("Retour au niveau %d.\n", niveau);
        } else if (option == 3) {
            printf("Redemarrage du niveau %d.\n", niveau);
        } else if (option != 1) {
            printf("Option invalide, continuation du niveau %d.\n", niveau);
        }

        int longueur_mot = t_mots[niveau - 1];
        printf("\n--- Niveau %d ---\n", niveau);
        printf("Veuillez saisir %d mots de %d lettres.\n", nbr_mots, longueur_mot);
        printf("Chaque mot valide gagne 20 points.\n");
        printf("Tapez 'q' pour quitter.\n");

        int mots_valide = 0;
        int mots_entre = 0;
        char buffer[MAX_MOT];

        while (mots_valide < nbr_mots) {
            printf("Mot (%d/%d): ", mots_valide + 1, nbr_mots);
            if (!fgets(buffer, sizeof(buffer), stdin)) {
                printf("Erreur de lecture, veuillez reessayer.\n");
                continue;
            }
            buffer[strcspn(buffer, "\n")] = 0;
            str_to_lower(buffer);

            if (strcmp(buffer, "q") == 0) {
                printf("Sauvegarde et sortie...\n");
                user->niveau = niveau;
                sauvegarder_progression(user);
                return;
            }

            if (!verifier_mot_lettres(buffer, longueur_mot)) {
                printf("Mot invalide.\n");
                continue;
            }

            if (mot_repete(mots_utilisee, mots_entre, buffer)) {
                printf("Mot deja utilise, veuillez entrer un autre.\n");
                continue;
            }

            if (!mot_valide(buffer, niveau)) {
                printf("Mot non trouve, essayez encore.\n");
                continue;
            }

            strcpy(mots_utilisee[mots_entre], buffer);
            mots_entre++;
            mots_valide++;
            user->score += 20;

            printf("Mot accepte! +20 points. Score actuel : %d\n", user->score);
        }
        printf("Bravo! Niveau %d termine avec %d points.\n", niveau, user->score);
        niveau++;
        user->niveau = niveau;
        sauvegarder_progression(user);
    }
    printf("\nBravo vous avez termine tous les niveaux !\n");
    printf("Score final : %d points\n", user->score);
}

int main() {
    srand((unsigned int)time(NULL));
    charger_utilisateurs();
    intro();

    while (1) {
        printf("\n--- Menu Principal ---\n");
        printf("1. Se connecter\n");
        printf("2. Creer un compte\n");
        printf("3. Quitter\n");
        printf("Votre choix: ");

        int choix;
        if (scanf("%d", &choix) != 1) {
            vider_buffer();
            printf("Veuillez reessayer.\n");
            continue;
        }
        vider_buffer();

        if (choix == 1) {
            Utilisateur *user = connexion_utilisateur();
            if (user != NULL) {
                game(user);
            }
        } else if (choix == 2) {
            Utilisateur *user = creer_utilisateur();
            if (user != NULL) {
                game(user);
            }
        } else if (choix == 3) {
            printf("Merci d'avoir joue !\n");
            liberer_utilisateurs();
            break;
        } else {
            printf("Veuillez reessayer.\n");
        }
    }
    return 0;
}

