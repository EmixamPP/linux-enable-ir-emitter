#pragma once

#include <string>
#include <vector>
using namespace std;

#include "utils/yaml.hpp"

class Camera;

class CameraInstruction
{
protected:
    bool corrupted_ = false;
    uint8_t unit_;
    uint8_t selector_;
    vector<uint8_t> cur_ctrl_;
    vector<uint8_t> init_ctrl_;
    vector<uint8_t> max_ctrl_;
    vector<uint8_t> min_ctrl_;

public:
    CameraInstruction() = default;

    explicit CameraInstruction(Camera &camera, uint8_t unit, uint8_t selector);

    ~CameraInstruction() = default;

    CameraInstruction &operator=(const CameraInstruction &) = default;

    CameraInstruction(const CameraInstruction &) = default;

    CameraInstruction &operator=(CameraInstruction &&) = default;

    CameraInstruction(CameraInstruction &&) = default;

    bool next() noexcept;

    bool is_corrupted() const noexcept;

    uint8_t unit() const noexcept;

    uint8_t selector() const noexcept;

    const vector<uint8_t> &cur() const noexcept;

    const vector<uint8_t> &max() const noexcept;

    const vector<uint8_t> &min() const noexcept;

    const vector<uint8_t> &init() const noexcept;

    void set_corrupted(bool is_corrupted) noexcept;

    bool set_cur(const vector<uint8_t> &cur) noexcept;

    bool set_min_cur() noexcept;

    bool set_max_cur() noexcept;

    void reset() noexcept;

    friend struct YAML::convert<CameraInstruction>;
};

string to_string(const CameraInstruction &inst);

string to_string(const vector<uint8_t> &vec);

using CameraInstructions = vector<CameraInstruction>;

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
            node["corrupted"] = obj.corrupted_;
            node["unit"] = static_cast<int>(obj.unit_);
            node["selector"] = static_cast<int>(obj.selector_);

            for (const auto &v : obj.cur_ctrl_)
                node["current"].push_back(static_cast<int>(v));

            if (obj.cur_ctrl_ != obj.init_ctrl_)
                for (const auto &v : obj.init_ctrl_)
                    node["initial"].push_back(static_cast<int>(v));

            for (const auto &v : obj.max_ctrl_)
                node["maximum"].push_back(static_cast<int>(v));

            for (const auto &v : obj.min_ctrl_)
                node["minimum"].push_back(static_cast<int>(v));

            return node;
        }

        static bool decode(const Node &node, CameraInstruction &obj)
        {
            try
            {
                obj.corrupted_ = node["corrupted"].as<bool>();
                obj.unit_ = node["unit"].as<uint8_t>();
                obj.selector_ = node["selector"].as<uint8_t>();

                obj.cur_ctrl_ = node["current"].as<vector<uint8_t>>();

                if (node["initial"])
                    obj.init_ctrl_ = node["initial"].as<vector<uint8_t>>();
                else
                    obj.init_ctrl_.assign(obj.cur_ctrl_.begin(), obj.cur_ctrl_.end());

                if (node["maximum"])
                    obj.max_ctrl_ = node["maximum"].as<vector<uint8_t>>();

                if (node["minimum"])
                    obj.min_ctrl_ = node["minimum"].as<vector<uint8_t>>();
            }
            catch (...)
            {
                return false;
            }

            return true;
        }
    };
}
