#pragma once

#include <fuzzing/exception.hpp>
#include <fuzzing/types.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace fuzzing {
namespace datasource  {

class Base
{
    protected:
        virtual std::vector<uint8_t> get(const size_t min, const size_t max, const uint64_t id = 0) = 0;
        virtual void put(const void* p, const size_t size, const uint64_t id = 0) = 0;
        std::vector<uint8_t> out;
    public:
        Base(void) = default;
        virtual ~Base(void) = default;

        template<class T> T Get(const uint64_t id = 0);
        template<class T> void Put(const T& v, const uint64_t id = 0);

        uint16_t GetChoice(const uint64_t id = 0);

        std::vector<uint8_t> GetData(const uint64_t id, const size_t min = 0, const size_t max = 0);
        void PutData(const std::vector<uint8_t>& data, const uint64_t id = 0);

        template <class T> std::vector<T> GetVector(const uint64_t id = 0);
        const std::vector<uint8_t>& GetOut(void) const { return out; }

        virtual size_t Left(void) const = 0;

        class OutOfData : public fuzzing::exception::FlowException {
            public:
                OutOfData() = default;
        };

        class DeserializationFailure : public fuzzing::exception::FlowException {
            public:
                DeserializationFailure() = default;
        };
};

#ifndef FUZZING_HEADERS_NO_IMPL
template<class T> T Base::Get(const uint64_t id)
{
    T ret;
    const auto v = get(sizeof(ret), sizeof(ret), id);
    memcpy(&ret, v.data(), sizeof(ret));
    return ret;
}

template<class T> void Base::Put(const T& v, const uint64_t id)
{
    put(&v, sizeof(v), id);
}

template <> bool Base::Get<bool>(const uint64_t id)
{
    uint8_t ret;
    const auto v = get(sizeof(ret), sizeof(ret), id);
    memcpy(&ret, v.data(), sizeof(ret));
    return (ret % 2) ? true : false;
}

template <> void Base::Put<bool>(const bool& v, const uint64_t id)
{
    const uint8_t _v = v ? 1 : 0;
    put(&_v, sizeof(_v), id);
}

template <> std::string Base::Get<std::string>(const uint64_t id)
{
    auto data = GetData(id);
    return std::string(data.data(), data.data() + data.size());
}

template <> std::vector<std::string> Base::Get<std::vector<std::string>>(const uint64_t id)
{
    std::vector<std::string> ret;
    while ( true ) {
        auto data = GetData(id);
        ret.push_back( std::string(data.data(), data.data() + data.size()) );
        if ( Get<bool>(id) == false ) {
            break;
        }
    }
    return ret;
}

uint16_t Base::GetChoice(const uint64_t id)
{
    return Get<uint16_t>(id);
}

std::vector<uint8_t> Base::GetData(const uint64_t id, const size_t min, const size_t max)
{
    return get(min, max, id);
}

void Base::PutData(const std::vector<uint8_t>& data, const uint64_t id)
{
    return put(data.data(), data.size(), id);
}

template <> types::String<> Base::Get<types::String<>>(const uint64_t id) {
    const auto data = GetData(id);
    types::String<> ret(data.data(), data.size());
    return ret;
}

template <> types::Data<> Base::Get<types::Data<>>(const uint64_t id) {
    const auto data = GetData(id);
    types::Data<> ret(data.data(), data.size());
    return ret;
}

template <class T>
std::vector<T> Base::GetVector(const uint64_t id) {
    std::vector<T> ret;

    while ( Get<bool>(id) == true ) {
        ret.push_back( Get<T>(id) );
    }

    return ret;
}
#endif

class Datasource : public Base
{
    private:
        const uint8_t* data;
        const size_t size;
        size_t idx;
        size_t left;
        std::vector<uint8_t> get(const size_t min, const size_t max, const uint64_t id = 0) override;
        void put(const void* p, const size_t size, const uint64_t id = 0) override;
    public:
        Datasource(const uint8_t* _data, const size_t _size);
        size_t Left(void) const override;
};

#ifndef FUZZING_HEADERS_NO_IMPL
Datasource::Datasource(const uint8_t* _data, const size_t _size) :
    Base(), data(_data), size(_size), idx(0), left(size)
{
}

std::vector<uint8_t> Datasource::get(const size_t min, const size_t max, const uint64_t id) {
    (void)id;

    uint32_t getSize;
    if ( left < sizeof(getSize) ) {
        throw OutOfData();
    }
    memcpy(&getSize, data + idx, sizeof(getSize));
    idx += sizeof(getSize);
    left -= sizeof(getSize);

    if ( getSize < min ) {
        getSize = min;
    }
    if ( max && getSize > max ) {
        getSize = max;
    }

    if ( left < getSize ) {
        throw OutOfData();
    }

    std::vector<uint8_t> ret(getSize);

    if ( getSize > 0 ) {
        memcpy(ret.data(), data + idx, getSize);
    }
    idx += getSize;
    left -= getSize;

    return ret;
}

void Datasource::put(const void* p, const size_t size, const uint64_t id) {
    (void)id;
    {
        const uint32_t _size = size;
        const auto oldSize = out.size();
        out.resize(oldSize + sizeof(_size) );
        memcpy(out.data() + oldSize, &_size, sizeof(_size));
    }

    if ( size == 0 ) {
        return;
    }

    {
        const auto oldSize = out.size();
        out.resize(oldSize + size);
        memcpy(out.data() + oldSize, p, size);
    }
}

size_t Datasource::Left(void) const {
    return left;
}
#endif

} /* namespace datasource */
} /* namespace fuzzing */
