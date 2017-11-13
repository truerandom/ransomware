#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <crypt.h>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>				//ver si el archivo existe

int doesFileExist(char filename[]);
int main(int argc, char *argv[]){
	Display *display;
	Window root;
	int len = 0, i;
	XEvent ev;

	if ((display = XOpenDisplay(NULL)) == NULL) {
		fprintf(stderr, "could not connect to $DISPLAY\n");
		exit(1);
	}

	root = DefaultRootWindow(display);
	XGrabPointer(display, root, 1, ButtonPress, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
	XGrabKeyboard(display, root, 0, GrabModeAsync, GrabModeAsync, CurrentTime);
	XSelectInput(display, root, KeyPressMask);

	while (XNextEvent(display, &ev), 1) {
		//si el archivo .unlock existe -> desbloqueamos la pantalla
		if (fileExist(".unlock")){
			XUngrabKeyboard(display, CurrentTime);
			XUngrabPointer(display, CurrentTime);
			exit(0);
		}
    	}
}

//regresa 1 si el archivo existe 0 en otro caso
int fileExist(char filename[]) {
	struct stat st;
	int result = stat(filename, &st);
	return result == 0;
}
