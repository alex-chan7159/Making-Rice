#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
//#include "../curl/include/curl/curl.h"
#include <curl/curl.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>  // For STL support
#include <thread>
#include <chrono>
//#include "C:\Users\drkab\OneDrive\Documents\GitHub\curl\include\curl\curl.h"
using namespace std;

//C:\Users\drkab\OneDrive\Desktop\C++\build>cmake --build .

/*
Free Rice Program
Author: Alex Chan
Date: October 4th, 2024
Version: October 20th, 2024
Desc: Interacts with Free Rice HTTPS API to answer questions which donate 10 grains of rice upon completion to The United Nations World Food Programme!
-Uses libcurl for API requests
-Uses spacy NLP model from python to identify correct answers to questions
-Makes requests at 6 second intervals, generating over 1 million grains of rice weekly.
(Note: A 66.7% increase in donation efficiency compared to a human that answers questions 24 hours a day.)
*/

string userId;
string token;
string gameId;
string questionWord;
string questionId;
string previousQuestion = "";
vector<string> answers;
unordered_map<string, string> hash_map = {
    {"temp", "temp"}
};

string json;

namespace py = pybind11;  // Declare the namespace

string extractString(string st, const string key, const string end) {
    size_t startPos = st.find(key);
    
    if (startPos == std::string::npos) {
        return ""; // Token not found
    }
    
    startPos += key.length(); // Move past the key
    size_t endPos = st.find(end, startPos); // Find the end of the token
    
    if (endPos == std::string::npos) {
        return ""; // Malformed JSON, no closing quote
    }
    //cout << "DID WE FIND ANY RICE" << endl;
    return st.substr(startPos, endPos - startPos); // Extract the token
}

size_t write_data(char *buffer, size_t itemsize, size_t nitems, void *ignorethis) {
    size_t bytes = itemsize * nitems;
    int linenumber = 1;
    //printf("New chunk (%zu bytes)\n", bytes);
    //printf("%d:\t", linenumber);
    //for (int i=0; i < bytes; i++) {
        //printf("%c", buffer[i]);
        //if (buffer[i] == '\n') {
            //linenumber++;
            //printf("%d:\t", linenumber);
        //}
    //}
    
    json.append(buffer, bytes); // Append the buffer content to the global json string
   // for (int i=0; i < bytes; i++) {
        //cout << buffer[i];
    //}
    //printf("\n\n");
    return bytes;
}

CURLcode login(CURL *curl, std::string &readBuffer) {
    // Set the URL for the login request
    curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.freerice.com/auth/login?_format=json");
    // Specify the JSON data for login
    const char* jsonData = "{\"username\":\"Alex C 7159\", \"password\":\"freeRiceAccount\"}";
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData);

    // Handle response data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    // Enable cookie handling
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, ""); // Enable cookie management
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookies.txt"); // Save cookies to this file
    
    // Set options for follow redirects (if needed)
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

     // Set User-Agent to mimic a browser
    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform the request        
    CURLcode result = curl_easy_perform(curl);
    return result;
}

CURLcode apiRequestWithToken(CURL *curl, const char* endPoint, string action) {
    curl_easy_setopt(curl, CURLOPT_URL, endPoint); // Replace with your API endpoint
    
    if (action == "get") {
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); // Set to use GET method
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET"); // Set to use GET method
    }
    else if (action == "post") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L); // Set to use POST method
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST"); // Change to use POST method
    }

    struct curl_slist *headers = nullptr;
    string authHeader = "Authorization: Bearer " + token;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Handle response data
    string readBuffer;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    
    CURLcode result = curl_easy_perform(curl);
    
    curl_slist_free_all(headers); // Free the header list
    return result;
}

// Function to submit the answer
CURLcode submitAnswer(CURL *curl, const string &questionId, const string &selectedOptionId) { //, const string &gameId, 
    // Create the JSON payload with both gameId and questionId
    string jsonData = "{\"answer\":\"" + selectedOptionId + "\",\"question\":\"" + questionId + "\",\"user\":\"" + userId + "\"}";

    // Set the URL for submitting the answer
    string url = "https://engine.freerice.com/games/" + gameId + "/answer";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Specify the JSON data for the answer
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH"); // Set to use PATCH method

    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json"); // Set content type
    headers = curl_slist_append(headers, "Accept: application/vnd.api+json; version=2"); // Updated Accept header
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");

    // Add Authorization header using the global token
    string authHeader = "Authorization: Bearer " + token;
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); // Set headers

    // Handle response data
    string readBuffer;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); // Assuming you have a write_data function to handle responses
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    // Perform the request
    CURLcode result = curl_easy_perform(curl);
    
    if (result == CURLE_OK) {
        cout << "Response: " << readBuffer << endl; // Output the response
    } else {
        cerr << "Error: " << curl_easy_strerror(result) << endl; // Handle errors
    }

    //cout << json;
    cout << "We have made " << extractString(json, "\"rice\":", ",") << " rice with this game id!" << endl;
    cout << "We have made " << extractString(json, "\"user_rice_total\":", ",") << " total rice!" << endl;

    // Cleanup
    curl_slist_free_all(headers); // Free the headers list

    return result;
}

void extractGame() {

    if (!hash_map.empty()) {
        hash_map.clear();
    }
    if (!answers.empty()) {
        answers.clear();
    }
    gameId = extractString(json, "games\",\"id\":\"", "\"");
    //unordered_map<string, string> hash_map;
    questionWord = extractString(json, "\"text\":\"", " means:"); //word means!!! dont include the means
    cout << "question word: " << questionWord << "!!" << endl;
    questionId = extractString(json, "\"question_id\":\"", "\"");
    //cout << "question word is " << questionWord << "and id is " << questionId << endl;

    //hash_map[questionWord] = questionId;

    const string key = "\"options\":[";
    size_t startPos = json.find(key);
    startPos += key.length();
    size_t endPos = json.find("]", startPos);
    string optionsStr = json.substr(startPos, endPos - startPos);
    size_t optionStart = 0;

    while ((optionStart = optionsStr.find("{", optionStart)) != string::npos) { //find answers and their respective ids, store into hash_mapmap
        size_t optionEnd = optionsStr.find("}", optionStart);
        if (optionEnd == string::npos) break;

        string optionJson = optionsStr.substr(optionStart, optionEnd - optionStart + 1);

        // Extract ID
        const string idKey = "\"id\":\"";
        size_t idPos = optionJson.find(idKey);
        string id;
        if (idPos != string::npos) {
            idPos += idKey.length();
            size_t idEndPos = optionJson.find("\"", idPos);
            id = optionJson.substr(idPos, idEndPos - idPos);
        }

        // Extract text
        const string textKey = "\"text\":\"";
        size_t textPos = optionJson.find(textKey);
        string text;
        if (textPos != string::npos) {
            textPos += textKey.length();
            size_t textEndPos = optionJson.find("\"", textPos);
            text = optionJson.substr(textPos, textEndPos - textPos);
        }

        //options.push_back({id, text});
        hash_map[text] = id;
        answers.push_back(text);
        optionStart = optionEnd + 1; // Move past the current option
    }
    //for (const auto &pair : hash_map) {
        //cout << pair.first << ": " << pair.second << endl;
    //}
    //cout << "game id is: " << gameId << endl;
}

map<string, float> getSimilarities(py::object nlp_model, string questionWord, vector<string> answers) {
    // Call the compare similarity method
    py::str pythonQuestionWord(questionWord);
    py::dict similarities = nlp_model.attr("compare_similarity")(pythonQuestionWord, answers);
    if (similarities.empty()) {
        exit(1);
    }

    // Convert the returned dictionary to a C++ map for easier access
    map<string, float> similarities_map;
    for (const auto& item : similarities) {
        string answer = item.first.cast<string>();  // Key (answer)
        float score = item.second.cast<float>();              // Value (similarity score)
        similarities_map[answer] = score;                     // Store in C++ map
    }

     // Print the results
    //for (const auto& pair : similarities_map) {
        //cout << "Answer: " << pair.first << ", Similarity Score: " << pair.second << endl;
    //}

    return similarities_map;
}

int main(void) {

    // Initialize Python interpreter
    py::scoped_interpreter guard{};

    // Import the module
    py::module synonyms = py::module::import("synonyms");

    // Create the NLP model
    py::object nlp_model = synonyms.attr("NLPModel")();

    map<string, float> answerSimilarities;
    //map<string, float> answerSimilarities = getSimilarities(questionWord, answers);

    CURL *curl = curl_easy_init();
    if(curl) {
        std::string readBuffer;
        // Perform the request        
        CURLcode result = login(curl, readBuffer);
        
        if (token.empty()) {
                token = extractString(json, "\"token\":\"", "\"");
            }
            
            if (userId.empty()) {
                userId = extractString(json, "\"uuid\":\"", "\"");
            }

                if (result != CURLE_OK) {
                    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(result) << std::endl;
                } else {
                    std::cout << "Response: " << readBuffer << std::endl;
                }

                cout << "userid is" << userId << "\n the token is " << token  << endl;
                
                result = apiRequestWithToken(curl, "https://engine.freerice.com/games/ce4d922a-29e3-4bcc-a10d-4ea1723155b8", "get"); //note!!! if you use a game id for too long, eventually the words become very difficult
                do {
                    // Now you can make another API request using the token
                //result = apiRequestWithToken(curl, "https://engine.freerice.com/users/23add682-202f-4054-951c-7249e4e65917?_format=json", "get");
                //cout << "blah blah blah" << json << endl;
                //const char* gameEndpoint = ("https://engine.freerice.com/games/" + gameId).c_str();
                
                //result = apiRequestWithToken(curl, gameEndpoint, "get");

                //string questionId = "b755e843-8142-5cbd-81db-c8cfc3d3b51d"; // Example question ID
                //string selectedOptionId = "609c5d21-29a0-4ef7-af90-b77e4857931a"; // Example selected option ID

                //submitAnswer(curl, questionId, selectedOptionId); // Submit the answer
                extractGame();
                json.clear();
                // Ensure that we have a valid question
                if (questionWord == previousQuestion) {
                    cout << "Failed to retrieve new question." << endl;
                    break; // Exit if we can't get a new question
                }
                //for (string answer : answers) {
                //cout << answer << endl;
                //}
                answerSimilarities = getSimilarities(nlp_model, questionWord, answers);
                string maxKey;
                float maxValue = -1; // Initialize to the lowest possible float, wait no similarity sometimes returns negative

                for (const auto& pair : answerSimilarities) {
                    if (pair.second > maxValue) {
                        maxValue = pair.second;
                        maxKey = pair.first;
                    }
                }

                //cout << maxKey << " has a sim of " << maxValue << endl;
                //cout << questionId << endl;
                //cout << questionId << "     " << hash_map[maxKey] << endl;
                submitAnswer(curl, questionId, hash_map[maxKey]);
                previousQuestion = questionWord;

                answerSimilarities.clear();
                this_thread::sleep_for(chrono::seconds(5));
                //cout << "AHHHHHHHHHHHHH" << endl;
                //result = apiRequestWithToken(curl, "https://engine.freerice.com/games/cb2902c4-8085-43ad-a32c-139799b21ad2", "get");
                //result = apiRequestWithToken(curl, gameEndpoint, "get");
                
                cout << "trying again" << endl;
                } while (result == CURLE_OK);

                if (result != CURLE_OK) {
                    std::cerr << "API request failed: " << curl_easy_strerror(result) << std::endl;
                }

                // Clean up
                //curl_slist_free_all(headers); // Free the header list
                curl_easy_cleanup(curl); // Clean up curl session

    } else {
        std::cerr << "Curl initialization failed" << std::endl;
    }

    return EXIT_SUCCESS;
}