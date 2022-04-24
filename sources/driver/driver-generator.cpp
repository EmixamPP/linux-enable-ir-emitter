/*** DOCUMENTATION
- https://www.kernel.org/doc/html/latest/userspace-api/media/drivers/uvcvideo.html
    info 1: uvc queries are explained
    info 2: units can be found by parsing the uvc descriptor
- https://www.mail-archive.com/search?l=linux-uvc-devel@lists.berlios.de&q=subject:%22Re%5C%3A+%5C%5BLinux%5C-uvc%5C-devel%5C%5D+UVC%22&o=newest&f=1
    info 1: selector is on 8 bits and since the manufacturer does not provide a driver, it is impossible to know which value it is.
***/
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

/**
 * @brief Print the control value in the standart output (without eol character)
 *
 * @param ctrl control value
 * @param len size of the control value
 */
void print_ctrl(__u8 *ctrl, __u16 len)
{
    for (__u16 i = 0; i < len; ++i)
        cout << " " << (int)ctrl[i];
}

/**
 * @brief Execute shell command and return the ouput
 *
 * @param cmd command
 * @return output
 */
string *shell_exec(string cmd)
{
    char buffer[128];
    string *result = new string();
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
        throw runtime_error("popen() failed!");
    while (fgets(buffer, 128 * sizeof(char), pipe.get()) != nullptr)
        *result += buffer;
    result->erase(result->end() - 1); // remove last \n
    return result;
}

/**
 * @brief Trigger the infrared emitter
 *
 * @param deviceID id of the camera device
 * @param time triger during how many seconds (default=2)
 */
void triger_camera(int deviceID, int time = 2)
{
    VideoCapture cap;
    cap.open(deviceID);
    if (!cap.isOpened())
    {
        cerr << "Cannot access to /dev/video" << deviceID << endl;
        exit(126);
    }
    this_thread::sleep_for(chrono::seconds(time));
    cap.release();
}

/**
 * @brief Trigger the infrared emitter and ask the question:
 *        "Did you see the ir emitter flashing (not just turn on) ? Yes/No ? "
 *
 * @param deviceID id of the camera device
 * @return true  if the user has input Yes
 * @return false if the user has input No
 */
bool test_emitter(int deviceID)
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
 * @return vector<__u8>* list of units
 */
vector<__u8> *get_units(const char *device)
{
    const string *vid = shell_exec("udevadm info " + string(device) + " | grep -oP 'E: ID_VENDOR_ID=\\K.*'");
    const string *pid = shell_exec("udevadm info " + string(device) + " | grep -oP 'E: ID_MODEL_ID=\\K.*'");
    const string *units = shell_exec("lsusb -d" + *vid + ":" + *pid + " -v | grep bUnitID | grep -Eo '[0-9]+'");
    auto *unitsList = new vector<__u8>;

    unsigned i = 0, j = 0;
    for (; j < units->length(); ++j)
        if (units->at(j) == '\n')
        {
            unitsList->push_back((__u8)stoi(units->substr(i, j - i)));
            i = j + 1;
        }
    unitsList->push_back((__u8) stoi(units->substr(i, j - i)));

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
int get_next_curCtrl(__u8 *curCtrl, const __u8 *resCtrl, const __u8 *maxCtrl, __u16 ctrlSize)
{
    for (unsigned i = 0; i < ctrlSize; ++i)
    {
        curCtrl[i] = curCtrl[i] + resCtrl[i];
        if (curCtrl[i] > maxCtrl[i])
            return 1;
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
 *            265 Unable to open the camera device
 */
int main(int, char *argv[])
{
    setLogLevel(LogLevel::LOG_LEVEL_SILENT);
    const char *device = argv[1];
    int deviceID;
    sscanf(device, "/dev/video%d", &deviceID);
    int negAnswerLimit = atoi(argv[2]);
    const char *driverFile = argv[3];
    bool debug = atoi(argv[4]);

    if (test_emitter(deviceID))
    {
        cerr << "ERROR: Your emiter is already working, skipping the configuration." << endl;
        return 1;
    }

    int res;
    for (__u8 &unit : *get_units(device))
        for (__u8 selector = 0; selector < 255; ++selector)
        { // TODO 256
            // get the control instruction lenght
            const __u16 ctrlSize = len_uvc_query(device, unit, selector);
            if (!ctrlSize)
                continue;

            // get the current control value
            __u8 curCtrl[ctrlSize];
            res = get_uvc_query(UVC_GET_CUR, device, unit, selector, ctrlSize, curCtrl);
            if (res)
                continue;

            // check if the control value can be modified
            res = set_uvc_query(device, unit, selector, ctrlSize, curCtrl);
            if (res)
                continue;

            // try the max control value (the value does not necessary exists)
            __u8 maxCtrl[ctrlSize];
            res = get_uvc_query(UVC_GET_MAX, device, unit, selector, ctrlSize, maxCtrl);
            if (res)
                memset(maxCtrl, 255, ctrlSize * sizeof(__u8)); // use the 255 array

            // try get the resolution control value (the value does not necessary exists)
            __u8 resCtrl[ctrlSize];
            res = get_uvc_query(UVC_GET_RES, device, unit, selector, ctrlSize, resCtrl);
            if (res)
                for (unsigned i = 0; i < ctrlSize; ++i) // compute the step from the current and max control
                    resCtrl[i] = curCtrl[i] != maxCtrl[i]; // step of 0 or 1

            // try get the min control value (the value does not necessary exists) to test it
            __u8 nextCtrl[ctrlSize];
            res = get_uvc_query(UVC_GET_MIN, device, unit, selector, ctrlSize, nextCtrl);
            if (res || !memcmp(curCtrl, nextCtrl, ctrlSize * sizeof(__u8))) // or: the min control value is the current control
            {
                memcpy(nextCtrl, curCtrl, ctrlSize * sizeof(__u8)); // assign the next value of the current one
                res = get_next_curCtrl(nextCtrl, resCtrl, maxCtrl, ctrlSize);
                if (res) // the current control value is equal to the maximal control
                    continue;
            }

            if (debug){
                cout << "DEBUG: unit: " << (int) unit << ", selector: " << (int) selector;
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
            res = 0;
            while (!res && negAnswerCounter < negAnswerLimit)
            {
                set_uvc_query(device, unit, selector, ctrlSize, nextCtrl);

                if (test_emitter(deviceID))
                    return write_driver(driverFile, device, unit, selector, ctrlSize, nextCtrl);

                ++negAnswerCounter;
                res = get_next_curCtrl(nextCtrl, resCtrl, maxCtrl, ctrlSize);
            }

            if (debug && negAnswerCounter >= negAnswerLimit)
                cout << "DEBUG: Negative answer limit exceeded, skipping the pattern." << endl;

            // reset the control
            set_uvc_query(device, unit, selector, ctrlSize, curCtrl);
        }

    return 1;
}