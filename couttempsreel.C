#include <stdio.h>

// Fonction qui calcule le coût en fonction des actions prises et de leurs niveaux de graduation
int calcul_cout(int* actions, int taille_actions) {
    int cout = 0;
    int i;
    for (i = 0; i < taille_actions; i++) {
        switch (actions[i]) {
            case 1: // Aération niveau 1
                cout += 1;
                break;
            case 2: // Aération niveau 2
                cout += 2;
                break;
            case 3: // Aération niveau 3
                cout += 3;
                break;
            case 4: // Ventilation niveau 1
                cout += 6;
                break;
            case 5: // Ventilation niveau 2
                cout += 9;
                break;
            case 6: // Injection de gaz
                cout += 20;
                break;
            case 7: // Commande d'annulation
                cout += 0;
                break;
            default:
                printf("Action invalide");
                return -1;
        }
    }
    return cout;
}

// Exemple d'utilisation de la fonction
int main() {
    int actions[] = {1, 2, 4}; // Aération niveau 1, Aération niveau 2, Ventilation niveau 1
    int taille_actions = 3;
    int cout = calcul_cout(actions, taille_actions);
    printf("Le coût total des actions est : %d", cout);
    return 0;
}
