#include <iostream>

int main() {
    while (true) {
        int exitCode = std::system("C:/Users/drkab/OneDrive/Desktop/C++/build/FreeRice.exe");

        if (exitCode != 0) {
            std::cerr << "Program ended with error code: " << exitCode << std::endl;
        } else {
            std::cout << "Program ended normally." << std::endl;
        }

        // Optional: Pause before restarting

        // HEY HEY, SOMETMES FREE RICE STOPS WHEN THERE'S SOME WAITING AFTER A REQUEST IS MADE, IT CANT FETCH THE QUESTION AND ANSWERS - THEN I GET A STRING ERROR BUT OF COURSE BECAUSE THERE'S NO STRING THAT I GET
        //i may want to be able to just restart the program, if it breaks haha! from here!
    }
    return 0;
}