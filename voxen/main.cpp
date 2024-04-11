#include <iostream>
#include "App.h"
#include <vector>
int main(void) {
	App app;
	
	if (!app.Initialize()) {
		std::cout << "Failed App Initialized" << std::endl;
		return -1;
	}
	app.Run();
	return 0;
}