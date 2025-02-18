/*
 * @Description:
 * @Author:
 * @Date: 2021-03-03 17:57:08
 * @LastEditTime: 2021-03-03 18:07:03
 * @LastEditors: yaol
 * @FilePath: /gftf-to-instance/tinygltf/jsontool.hpp
 */
#include <vector>
#include "json.hpp"
template <class T>
std::vector<T> jsonToVector(const nlohmann::json &j)
{
    std::vector<T> resVec;
    for (auto &item : j)
    {
        resVec.emplace_back(item.get<T>());
    }
    return resVec;
}