#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

bool isLeftClickHeld = false;

// Move the OS cursor
void updateMousePosition(int deltaX, int deltaY) {
    POINT currentPos;
    if (GetCursorPos(&currentPos)) {
        SetCursorPos(currentPos.x + deltaX, currentPos.y + deltaY);
    }
}

// Simulate physical left-click using Win32 API
void triggerLeftClick(bool pressDown) {
    INPUT input = {0};
    input.type = INPUT_MOUSE;

    if (pressDown && !isLeftClickHeld) {
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));
        isLeftClickHeld = true;
    } else if (!pressDown && isLeftClickHeld) {
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));
        isLeftClickHeld = false;
    }
}

int main() {
    // --- SETUP YOUR COM PORT HERE ---
    HANDLE hSerial = CreateFile("\\\\.\\COM3", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Could not open COM port. Check your port number." << std::endl;
        return 1;
    }

    // Configure serial port settings (115200 baud)
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    GetCommState(hSerial, &dcbSerialParams);
    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    SetCommState(hSerial, &dcbSerialParams);

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts);

    std::cout << "Talon HMI Driver Active. Listening for telemetry..." << std::endl;

    char buffer[256];
    DWORD bytesRead;
    std::string incomingData = "";

    while (true) {
        if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            incomingData += buffer;

            // Process the incoming data line by line
            size_t pos;
            while ((pos = incomingData.find('\n')) != std::string::npos) {
                std::string line = incomingData.substr(0, pos);
                incomingData.erase(0, pos + 1);

                // Parse the CSV line
                if (line.rfind("DATA,", 0) == 0) {
                    std::vector<int> vals;
                    std::stringstream ss(line.substr(5));
                    std::string item;
                    while (std::getline(ss, item, ',')) {
                        vals.push_back(std::stoi(item));
                    }

                    // Ensure we have all 15 values (12 mux + 3 IMU)
                    if (vals.size() >= 15) {
                        int indexPressure = vals[9]; // Multiplexer C9
                        int accelX = vals[12];       // IMU X
                        int accelZ = vals[14];       // IMU Z

                        // Map IMU rotation to mouse movement (Deadzone mapping)
                        int moveX = 0;
                        int moveY = 0;
                        if (accelZ > 5000) moveX = 5;
                        else if (accelZ < -5000) moveX = -5;
                        
                        if (accelX > 5000) moveY = 5;
                        else if (accelX < -5000) moveY = -5;

                        if (moveX != 0 || moveY != 0) {
                            updateMousePosition(moveX, moveY);
                        }

                        // Map fingertip pressure to left-click & haptic feedback
                        if (indexPressure > 2000) { // Adjust this threshold based on your specific FSR sensitivity
                            triggerLeftClick(true);
                            
                            // Send 'C' back to ESP32 to fire the haptic piezo disc!
                            DWORD bytesWritten;
                            WriteFile(hSerial, "C", 1, &bytesWritten, NULL);
                        } else {
                            triggerLeftClick(false);
                        }
                    }
                }
            }
        }
        Sleep(10); // Match ESP32 loop speed
    }
    CloseHandle(hSerial);
    return 0;
}
