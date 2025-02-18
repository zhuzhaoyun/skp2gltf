/*
 * @Author: yaol 
 * @Date: 2025-02-18 17:31:24 
 * @Last Modified by:   yaol 
 * @Last Modified time: 2025-02-18 17:31:24 
 */

#ifndef DATATRANSFORM
#define DATATRANSFORM
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <unordered_map>
#include <iostream>
#include <unordered_set>
#include <stdio.h>
#include <algorithm>
#include <map>
#include <fstream>
#include <functional>
#include <thread>
// #include <unistd.h>
#include <unordered_set>
#include <iostream>
#include <vector>
using uchar = unsigned char;
using uint = unsigned int;
using unshort = unsigned short;
namespace convertTogltf
{
    enum class DataType
    {
        typeByte = 5120,
        typeUnByte = 5121,
        typeInt = 5124,
        typeUnInt = 5125,
        typeShort = 5122,
        typeUnShort = 5123,
        typeDouble = 5130,
        typeFloat = 5126
    };
    class AccessorsData
    {
    public:
        int accessorsIndex;
        virtual void insertData(const std::vector<uchar> &data) {}
        virtual void getBufferData(std::vector<uchar> &data) {}
        virtual DataType type() = 0;
        virtual ~AccessorsData() {}
    };
    class AccessorsDataByte : public AccessorsData
    {
    public:
        std::vector<char> accessorsData;
        void insertData(const std::vector<uchar> &data)
        {
            accessorsData.emplace_back(static_cast<char>(data[0]));
        }
        void getBufferData(std::vector<uchar> &data)
        {
            data.reserve(accessorsData.size());
            for (int i = 0; i < accessorsData.size(); ++i)
            {
                data.emplace_back(static_cast<uchar>(accessorsData[i]));
            }
        }
        DataType type() { return DataType::typeByte; }
    };
    class AccessorsDataUnByte : public AccessorsData
    {
    public:
        void insertData(const std::vector<uchar> &data)
        {
            accessorsData.emplace_back(static_cast<char>(data[0]));
        }
        void getBufferData(std::vector<uchar> &data)
        {
            data.reserve(accessorsData.size());
            for (int i = 0; i < accessorsData.size(); ++i)
            {
                data.emplace_back(accessorsData[i]);
            }
        }
        std::vector<uchar> accessorsData;
        DataType type() { return DataType::typeUnByte; }
    };
    class AccessorsDataInt : public AccessorsData
    {
    public:
        void insertData(const std::vector<uchar> &data)
        {
            int d;
            uchar temp[4];
            memset(temp, 0, sizeof(uchar) * 4);
            for (int i = 0; i < 4; ++i)
            {
                temp[i] = data[i];
            }
            memcpy(&d, &data[0], 4);
            accessorsData.emplace_back(d);
        }
        void getBufferData(std::vector<uchar> &data)
        {
            data.reserve(accessorsData.size()*4);
            for (int i = 0; i < accessorsData.size(); ++i)
            {
                uchar indec[4];
                int n = accessorsData[i];
                memcpy(&indec, &n, 4);
                for (int i = 0; i < 4; ++i)
                {
                    data.emplace_back(indec[i]);
                }
            }
        }
        std::vector<int> accessorsData;
        DataType type() { return DataType::typeInt; }
    };
    class AccessorsDataUnInt : public AccessorsData
    {
    public:
        void insertData(const std::vector<uchar> &data)
        {
            uint d;
            uchar temp[4];
            memset(temp, 0, sizeof(uchar) * 4);
            for (int i = 0; i < 4; ++i)
            {
                temp[i] = data[i];
            }
            memcpy(&d, &data[0], 4);
            accessorsData.emplace_back(d);
        }
        void getBufferData(std::vector<uchar> &data)
        {
            data.reserve(accessorsData.size()*4);
            for (int i = 0; i < accessorsData.size(); ++i)
            {
                uchar indec[4];
                uint n = accessorsData[i];
                memcpy(&indec, &n, 4);
                for (int i = 0; i < 4; ++i)
                {
                    data.emplace_back(indec[i]);
                }
            }
        }
        std::vector<uint> accessorsData;
        DataType type() { return DataType::typeUnInt; }
    };
    class AccessorsDataS2Unint : public AccessorsData
    {
    public:
        void getBufferData(std::vector<uchar> &data)
        {
            data.reserve(accessorsData.size()*4);
            for (int i = 0; i < accessorsData.size(); ++i)
            {
                uchar indec[4];
                uint n = accessorsData[i];
                memcpy(&indec, &n, 4);
                for (int i = 0; i < 4; ++i)
                {
                    data.emplace_back(indec[i]);
                }
            }
        }
        std::vector<size_t> accessorsData;
        DataType type() { return DataType::typeUnInt; }
    };
    class AccessorsDataShort : public AccessorsData
    {
    public:
        void insertData(const std::vector<uchar> &data)
        {
            short d;
            uchar temp[2];
            memset(temp, 0, sizeof(uchar) * 2);
            for (int i = 0; i < 2; ++i)
            {
                temp[i] = data[i];
            }
            memcpy(&d, &data[0], 2);
            accessorsData.emplace_back(d);
        }
        void getBufferData(std::vector<uchar> &data)
        {
            data.reserve(accessorsData.size()*2);
            for (int i = 0; i < accessorsData.size(); ++i)
            {
                uchar indec[2];
                short n = accessorsData[i];
                memcpy(&indec, &n, 2);
                for (int i = 0; i < 2; ++i)
                {
                    data.emplace_back(indec[i]);
                }
            }
        }
        std::vector<short> accessorsData;
        DataType type() { return DataType::typeShort; }
    };
    class AccessorsDataUnShort : public AccessorsData
    {
    public:
        void insertData(const std::vector<uchar> &data)
        {
            unshort d;
            uchar temp[2];
            memset(temp, 0, sizeof(uchar) * 2);
            for (int i = 0; i < 2; ++i)
            {
                temp[i] = data[i];
            }
            memcpy(&d, &data[0], 2);
            accessorsData.emplace_back(d);
        }
        void getBufferData(std::vector<uchar> &data)
        {
            data.reserve(accessorsData.size()*2);
            for (int i = 0; i < accessorsData.size(); ++i)
            {
                uchar indec[2];
                unshort n = accessorsData[i];
                memcpy(&indec, &n, 2);
                for (int i = 0; i < 2; ++i)
                {
                    data.emplace_back(indec[i]);
                }
            }
        }
        std::vector<unshort> accessorsData;
        DataType type() { return DataType::typeUnShort; }
    };
    class AccessorsDataDouble : public AccessorsData
    {
    public:
        void insertData(const std::vector<uchar> &data)
        {
            double d;
            uchar temp[8];
            memset(temp, 0, sizeof(uchar) * 8);
            for (int i = 0; i < 8; ++i)
            {
                temp[i] = data[i];
            }
            memcpy(&d, &data[0], 8);
            accessorsData.emplace_back(d);
        }
         void getBufferData(std::vector<uchar> &data)
        {
            data.reserve(accessorsData.size()*8);
            for (int i = 0; i < accessorsData.size(); ++i)
            {
                uchar indec[8];
                float n = accessorsData[i];
                memcpy(&indec, &n, 8);
                for (int i = 0; i < 8; ++i)
                {
                    data.emplace_back(indec[i]);
                }
            }
        }
        std::vector<double> accessorsData;
        DataType type() { return DataType::typeDouble; }
    };
    class AccessorsDataFloat : public AccessorsData
    {
    public:
        void insertData(const std::vector<uchar> &data)
        {
            float d;
            uchar temp[4];
            memset(temp, 0, sizeof(uchar) * 4);
            for (int i = 0; i < 4; ++i)
            {
                temp[i] = data[i];
            }
            memcpy(&d, &data[0], 4);
            accessorsData.emplace_back(d);
        }
        void getBufferData(std::vector<uchar> &data)
        {
            data.reserve(accessorsData.size()*4);
            for (int i = 0; i < accessorsData.size(); ++i)
            {
                uchar indec[4];
                float n = accessorsData[i];
                memcpy(&indec, &n, 4);
                for (int i = 0; i < 4; ++i)
                {
                    data.emplace_back(indec[i]);
                }
            }
        }
        std::vector<float> accessorsData;
        DataType type() { return DataType::typeFloat; }
    };
    class AccessorsDataD2F : public AccessorsData
    {
    public:
        void getBufferData(std::vector<uchar> &data)
        {
            data.reserve(accessorsData.size()*4);
            for (int i = 0; i < accessorsData.size(); ++i)
            {
                uchar indec[4];
                float n = accessorsData[i];
                memcpy(&indec, &n, 4);
                for (int i = 0; i < 4; ++i)
                {
                    data.emplace_back(indec[i]);
                }
            }
        }
        std::vector<double> accessorsData;
        DataType type() { return DataType::typeFloat; }
    };
    class DataFactory
    {
    public:
        static AccessorsData *createAccessorsData(DataType type)
        {
            switch (type)
            {
            case DataType::typeByte:
                return (new AccessorsDataByte);
                break;
            case DataType::typeUnByte:
                return (new AccessorsDataUnByte);
                break;
            case DataType::typeInt:
                return (new AccessorsDataInt);
                break;
            case DataType::typeUnInt:
                return (new AccessorsDataUnInt);
                break;
            case DataType::typeShort:
                return (new AccessorsDataShort);
                break;
            case DataType::typeUnShort:
                return (new AccessorsDataUnShort);
                break;
            case DataType::typeFloat:
                return (new AccessorsDataFloat);
                break;
            case DataType::typeDouble:
                return (new AccessorsDataDouble);
                break;
            default:
                return nullptr;
                break;
            }
        }
    };
} // namespace convertTogltf
#endif