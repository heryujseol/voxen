#include <iostream>
#include "App.h"

int main(void) {
	App app = App();
	
	if (!app.Initialize()) {
		std::cout << "Failed App Initialized" << std::endl;
		return -1;
	}
	app.Run();
	return 0;
}