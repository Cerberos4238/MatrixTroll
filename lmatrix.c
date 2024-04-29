//gcc lmatrix.c -o lmatrix -lncurses -lX11
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <X11/Xlib.h>

#define MY_DELAY 50000 // Microseconds
#define TRAIL_LENGTH 5 // Longueur des traînées

typedef struct {
    int y;          // Position actuelle sur l'axe y
    char *values;   // Valeurs de la colonne (0 ou 1)
} Column;

volatile sig_atomic_t quit = 0;

// Fonction de gestion du signal pour intercepter SIGINT (CTRL+C)
void handleSignal(int signum) {
    if (signum == SIGINT) {
        quit = 1;
    }
}

// Fonction pour bloquer l'interaction de l'utilisateur (Linux uniquement)
void blockUserInteraction() {
    system("sudo chmod 000 /dev/input/mice");
    system("sudo chmod 000 /dev/input/event*");
}

// Initialise les colonnes avec des espaces et des positions aléatoires sur l'axe y
void initializeColumns(Column columns[], int num_columns, int max_y) {
    for (int i = 0; i < num_columns; i++) {
        columns[i].y = rand() % max_y;
        columns[i].values = (char *)malloc(max_y * sizeof(char));
        for (int j = 0; j < max_y; j++) {
            columns[i].values[j] = ' ';
        }
    }
}

// Met à jour les colonnes, efface les caractères précédents, met à jour la position, ajoute les nouveaux caractères
void updateColumns(Column columns[], int num_columns, int max_y) {
    for (int i = 0; i < num_columns; i++) {
        // Efface les caractères précédents
        for (int j = 0; j < TRAIL_LENGTH; j++) {
            int prev_y = (columns[i].y - j + max_y) % max_y;
            columns[i].values[prev_y] = ' ';
        }

        // Met à jour la position et ajoute les nouveaux caractères
        columns[i].y = (columns[i].y + 1) % max_y;
        for (int j = 0; j < TRAIL_LENGTH; j++) {
            int new_y = (columns[i].y - j + max_y) % max_y;
            columns[i].values[new_y] = (rand() % 2) ? '0' : '1';
        }
    }
}

// Affiche les colonnes sur l'écran
void displayColumns(Column columns[], int num_columns, int max_y) {
    clear();
    for (int i = 0; i < num_columns; i++) {
        for (int j = 0; j < max_y; j++) {
            mvprintw(j, i * (COLS / num_columns), "%c", columns[i].values[j]);
        }
    }
    refresh();
}

int main() {
    initscr(); // Initialise l'environnement Ncurses
    curs_set(0); // Masque le curseur
    nodelay(stdscr, TRUE); // Mode non-bloquant pour getch()
    keypad(stdscr, TRUE); // Active les touches spéciales
    raw(); // Mode brut pour désactiver la mise en mémoire tampon des touches spéciales
    mousemask(0, NULL); // Désactive la gestion des événements de la souris
    Display *display;
    Window root;

    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Unable to open display.\n");
        return 1;
    }

    // Désactive l'affichage des touches (utile pour le pavé tactile)
    noecho();
    // Désactive la saisie des touches (utile pour le pavé tactile)
    timeout(0);

    // Configure la gestion du signal pour intercepter SIGINT (CTRL+C)
    signal(SIGINT, handleSignal);

    // Bloque l'interaction de l'utilisateur au démarrage
    blockUserInteraction();

     root = DefaultRootWindow(display);

     //Désactive la souris
     XGrabPointer(display, root, False, ButtonPressMask | ButtonReleaseMask |
                                  PointerMotionMask, GrabModeAsync,
                 GrabModeAsync, None, None, CurrentTime);

    // Désactive le clavier
    XGrabKeyboard(display, root, False, GrabModeAsync, GrabModeAsync, CurrentTime);


    srand(time(NULL));

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    int num_columns = max_x;  // Nombre de colonnes égal à la largeur de l'écran
    Column columns[num_columns];

    initializeColumns(columns, num_columns, max_y);

    while (!quit) {
        updateColumns(columns, num_columns, max_y);
        displayColumns(columns, num_columns, max_y);
        usleep(MY_DELAY);
    }

    // Restaure l'environnement terminal avant de quitter
    endwin();

    // Restaure l'interaction de l'utilisateur avant de quitter
    system("sudo chmod 644 /dev/input/mice");
    system("sudo chmod 644 /dev/input/event*");

    return 0;
}
