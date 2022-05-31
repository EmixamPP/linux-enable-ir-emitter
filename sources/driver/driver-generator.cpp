/*** DOCUMENTATION
- https://www.kernel.org/doc/html/latest/userspace-api/media/drivers/uvcvideo.html
    info 1: uvc queries are explained
    info 2: units can be found by parsing the uvc descriptor
- https://www.mail-archive.com/search?l=linux-uvc-devel@lists.berlios.de&q=subject:%22Re%5C%3A+%5C%5BLinux%5C-uvc%5C-devel%5C%5D+UVC%22&o=newest&f=1
    info 1: selector is on 8 bits and since the manufacturer does not provide a driver, it is impossible to know which value it is.
***/

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uvcvideo.h>
#include <iostream>
#include <thread>
#include <vector>
#include <iterator>
using namespace std;

// for triger_camera
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wconversion"
#include <opencv2/videoio.hpp>
#include <opencv2/core/utils/logger.hpp>
using namespace cv;
using namespace cv::utils::logging;
#pragma GCC diagnostic pop

#include "lenquery.h"
#include "getquery.h"
#include "setquery.h"
#include "executequery.h"
#include "driver.hpp"

constexpr unsigned CAMERA_TRIGER_TIME = 2; // triger the camera during how many seconds
constexpr unsigned FILE_DESCRIPTOR_ERROR = 126;

/**
 * @brief Print the control value in the standart output (without eol character)
 *
 * @param ctrl control value
 * @param len size of the control value
 */
void print_ctrl(const uint8_t *ctrl, const uint16_t len)
{
    for (uint16_t i = 0; i < len; ++i)
        cout << " " << (int)ctrl[i];
}

/**
 * @brief Execute shell command and return the ouput
 *
 * @param cmd command
 * @return output
 */
string *shell_exec(const string cmd)
{
    char buffer[128];
    string *result = new string();
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
    {
        *result = "error";
        return result;
    }

    while (fgets(buffer, 128 * sizeof(char), pipe.get()) != nullptr)
        *result += buffer;
    result->erase(result->end() - 1); // remove last \n
    return result;
}

/**
 * @brief Trigger the infrared emitter
 *
 * @param deviceID id of the camera device
 *
 * @throw int 126 if unable to open the camera device
 */
void triger_camera(const int deviceID)
{
    VideoCapture cap;
    cap.open(deviceID);
    if (!cap.isOpened())
    {
        cerr << "CRITICAL: Cannot access to /dev/video" << deviceID << endl;
        throw FILE_DESCRIPTOR_ERROR;
    }
    this_thread::sleep_for(chrono::seconds(CAMERA_TRIGER_TIME));
    cap.release();
}

/**
 * @brief Trigger the infrared emitter and ask the question:
 *        "Did you see the ir emitter flashing (not just turn on) ? Yes/No ? "
 *
 * @param deviceID id of the camera device
 *
 * @throw int 126 if unable to open the camera device
 *
 * @return true if the user has input Yes, false if the user has input No
 */
bool test_emitter(const int deviceID)
{
    triger_camera(deviceID);
    string answer;
    cout << "Did you see the ir emitter flashing (not just turn on) ? Yes/No ? ";
    cin >> answer;
    while (answer != "yes" && answer != "y" && answer != "Yes" && answer != "no" && answer != "n" && answer != "No")
    {
        cout << "Yes/No ? ";
        cin >> answer;
    }
    return answer == "yes" || answer == "y" || answer == "Yes";
}

/**
 * @brief Get all the extension unit ID of the camera device
 *
 * @param device path to the camera /dev/videoX
 *
 * @return vector<uint8_t>* list of units
 */
const vector<uint8_t> *get_units(const char *device)
{
    const string *vid = shell_exec("udevadm info " + string(device) + " | grep -oP 'E: ID_VENDOR_ID=\\K.*'");
    const string *pid = shell_exec("udevadm info " + string(device) + " | grep -oP 'E: ID_MODEL_ID=\\K.*'");
    const string *units = shell_exec("lsusb -d" + *vid + ":" + *pid + " -v | grep bUnitID | grep -Eo '[0-9]+'");
    auto *unitsList = new vector<uint8_t>;

    unsigned i = 0, j = 0;
    for (; j < units->length(); ++j)
        if (units->at(j) == '\n')
        {
            unitsList->push_back((uint8_t)stoi(units->substr(i, j - i)));
            i = j + 1;
        }
    unitsList->push_back((uint8_t)stoi(units->substr(i, j - i)));

    delete vid;
    delete pid;
    delete units;
    return unitsList;
}

/**
 * @brief Compute the next possible control value
 *
 * @param curCtrl last executed control value
 * @param resCtrl resolution control value
 * @param maxCtrl maximum control value
 * @param ctrlSize len of the control value
 * @return non zero if there is no more possible control value
 */
int get_next_curCtrl(uint8_t *curCtrl, const uint8_t *resCtrl, const uint8_t *maxCtrl, const uint16_t ctrlSize)
{
    if (!memcmp(curCtrl, maxCtrl, ctrlSize * sizeof(uint8_t))) // curCtrl == maxCtrl
        return 1;

    for (unsigned i = 0; i < ctrlSize; ++i)
    {
        curCtrl[i] = curCtrl[i] + resCtrl[i];
        if (curCtrl[i] > maxCtrl[i]) // resCtrl does not allow to reach maxCtrl
        {
            memcpy(curCtrl, maxCtrl, ctrlSize * sizeof(uint8_t)); // set maxCtrl
            return 0;
        }
    }
    return 0;
}

/**
 * Generate a driver for the infrared emitter
 *
 * usage: driver-generator [device] [negAnswerLimit] [driverFile] [debug]
 *        device           path to the infrared camera, /dev/videoX
 *        negAnswerLimit   after k negative answer the pattern will be skiped. Use 256 for unlimited
 *        driverFile       path where the driver will be written
 *        debug            1 for print debug information, otherwise 0
 *
 * See std output for debug information and stderr for error information
 *
 * Exit code: 0 Success
 *            1 Error
 *            126 Unable to open the camera device
 */
int main(int, const char *argv[])
{
    setLogLevel(LogLevel::LOG_LEVEL_SILENT);
    const char *device = argv[1];
    int deviceID;
    sscanf(device, "/dev/video%d", &deviceID);
    const int negAnswerLimit = atoi(argv[2]);
    const char *driverFile = argv[3];
    const bool debug = atoi(argv[4]);

    errno = 0;
    const int fd = open(device, O_WRONLY);
    if (fd < 0 || errno)
    {
        cerr << "CRITICAL: Cannot access to " << device << endl;
        exit(FILE_DESCRIPTOR_ERROR);
    }

    try
    {
        if (test_emitter(deviceID))
        {
            cerr << "ERROR: Your emiter is already working, skipping the configuration." << endl;
            return 1;
        }
    }
    catch (int code)
    {
        close(fd);
        exit(code);
    }

    int result;
    const vector<uint8_t> *units = get_units(device);
    for (const uint8_t &unit : *units)
        for (uint8_t selector = 0; selector < 255; ++selector) // it should be < 256, but it have no idea how to do it proprely (i.e. without cast)
        {
            // get the control instruction lenght
            const uint16_t ctrlSize = len_uvc_query(fd, unit, selector);
            if (!ctrlSize)
                continue;

            // get the current control value
            uint8_t curCtrl[ctrlSize];
            if (get_uvc_query(UVC_GET_CUR, fd, unit, selector, ctrlSize, curCtrl))
                continue;

            // check if the control value can be modified
            if (set_uvc_query(fd, unit, selector, ctrlSize, curCtrl))
                continue;

            // try to get the maximum control value (the value does not necessary exists)
            uint8_t maxCtrl[ctrlSize];
            if (get_uvc_query(UVC_GET_MAX, fd, unit, selector, ctrlSize, maxCtrl))
                memset(maxCtrl, 255, ctrlSize * sizeof(uint8_t)); // use the 255 array

            // try get the resolution control value (the value does not necessary exists)
            uint8_t resCtrl[ctrlSize];
            if (get_uvc_query(UVC_GET_RES, fd, unit, selector, ctrlSize, resCtrl))
            {
                if (debug)
                    cout << "DEBUG: Computing the resolution control." << endl;
                for (unsigned i = 0; i < ctrlSize; ++i)
                    resCtrl[i] = curCtrl[i] != maxCtrl[i]; // step of 0 or 1
            }

            // try get the minimum control value (the value does not necessary exists) to start from it
            uint8_t nextCtrl[ctrlSize];
            result = get_uvc_query(UVC_GET_MIN, fd, unit, selector, ctrlSize, nextCtrl);
            if (result || !memcmp(curCtrl, nextCtrl, ctrlSize * sizeof(uint8_t))) // or: the min control value is the current control
            {
                memcpy(nextCtrl, curCtrl, ctrlSize * sizeof(uint8_t));      // set the current value
                if (get_next_curCtrl(nextCtrl, resCtrl, maxCtrl, ctrlSize)) // and start from the next one
                    continue;                                               // the current control value is equal to the maximal control
            }

            if (debug)
            {
                cout << "DEBUG: unit: " << (int)unit << ", selector: " << (int)selector;
                cout << ", cur control:";
                print_ctrl(curCtrl, ctrlSize);
                cout << ", first control to test:";
                print_ctrl(nextCtrl, ctrlSize);
                cout << ", res control:";
                print_ctrl(resCtrl, ctrlSize);
                cout << ", max control:";
                print_ctrl(maxCtrl, ctrlSize);
                cout << endl;
            }

            // try to find the right control value
            int negAnswerCounter = 0;
            result = 0;
            while (!result && negAnswerCounter < negAnswerLimit)
            {
                result = set_uvc_query(fd, unit, selector, ctrlSize, nextCtrl);

                if (!result)
                    try
                    {
                        if (test_emitter(deviceID)) // found
                        {
                            close(fd);
                            return write_driver(driverFile, device, unit, selector, ctrlSize, nextCtrl);
                        }
                    }
                    catch (int code) // if error during test, reset the control and exit
                    {
                        set_uvc_query(fd, unit, selector, ctrlSize, curCtrl);
                        close(fd);
                        delete units;
                        exit(code);
                    }

                ++negAnswerCounter;
                result = get_next_curCtrl(nextCtrl, resCtrl, maxCtrl, ctrlSize);
            }

            if (debug && negAnswerCounter >= negAnswerLimit)
                cout << "DEBUG: Negative answer limit exceeded, skipping the pattern." << endl;

            // reset the control
            result = set_uvc_query(fd, unit, selector, ctrlSize, curCtrl);
            if (result)
                cout << "ERROR: Impossible to reset the control." << endl;
        }

    close(fd);
    delete units;
    return 1;
}
