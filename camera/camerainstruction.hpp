#pragma once

#include <string>
#include <vector>
using namespace std;

#include <yaml-cpp/yaml.h>

class Camera;

class CameraInstruction
{
protected:
    bool corrupted = false;
    uint8_t unit;
    uint8_t selector;
    vector<uint8_t> curCtrl;
    vector<uint8_t> initCtrl;
    vector<uint8_t> maxCtrl;
    vector<uint8_t> minCtrl;

public:
    CameraInstruction() = default;

    explicit CameraInstruction(Camera &camera, uint8_t unit, uint8_t selector);

    ~CameraInstruction() = default;

    CameraInstruction &operator=(const CameraInstruction &) = default;

    CameraInstruction(const CameraInstruction &) = default;

    CameraInstruction &operator=(CameraInstruction &&) = default;

    CameraInstruction(CameraInstruction &&) = default;

    bool next() noexcept;

    bool isCorrupted() const noexcept;

    uint8_t getUnit() const noexcept;

    uint8_t getSelector() const noexcept;

    const vector<uint8_t> &getCur() const noexcept;

    const vector<uint8_t> &getMax() const noexcept;

    const vector<uint8_t> &getMin() const noexcept;

    const vector<uint8_t> &getInit() const noexcept;

    void setCorrupted(bool isCorrupted) noexcept;

    bool setCur(const vector<uint8_t> &newCur) noexcept;

    bool setMinAsCur() noexcept;

    bool setMaxAsCur() noexcept;

    void reset() noexcept;

    friend struct YAML::convert<CameraInstruction>;
};

string to_string(const CameraInstruction &inst);

string to_string(const vector<uint8_t> &vec);

class CameraInstructionException : public exception
{
private:
    string message;

public:
    explicit CameraInstructionException(const string &device, uint8_t unit, uint8_t selector);

    const char *what() const noexcept override;
};

namespace YAML
{
    template <>
    struct convert<CameraInstruction>
    {
        static Node encode(const CameraInstruction &obj)
        {
            Node node;
            node["corrupted"] = obj.corrupted;
            node["unit"] = static_cast<int>(obj.unit);
            node["selector"] = static_cast<int>(obj.selector);

            for (const auto &v : obj.curCtrl)
                node["current"].push_back(static_cast<int>(v));

            if (obj.curCtrl != obj.initCtrl)
                for (const auto &v : obj.initCtrl)
                    node["initial"].push_back(static_cast<int>(v));

            for (const auto &v : obj.maxCtrl)
                node["maximum"].push_back(static_cast<int>(v));

            for (const auto &v : obj.minCtrl)
                node["minimum"].push_back(static_cast<int>(v));

            return node;
        }

        static bool decode(const Node &node, CameraInstruction &obj)
        {
            obj.corrupted = node["corrupted"].as<bool>();
            obj.unit = node["unit"].as<uint8_t>();
            obj.selector = node["selector"].as<uint8_t>();

            obj.curCtrl = node["current"].as<vector<uint8_t>>();

            if (node["initial"])
                obj.initCtrl = node["initial"].as<vector<uint8_t>>();
            else
                obj.initCtrl.assign(obj.curCtrl.begin(), obj.curCtrl.end());

            if (node["maximum"])
                obj.maxCtrl = node["maximum"].as<vector<uint8_t>>();

            if (node["minimum"])
                obj.minCtrl = node["minimum"].as<vector<uint8_t>>();

            return true;
        }
    };
}