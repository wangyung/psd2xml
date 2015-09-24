#pragma once
#ifndef PSDRESOURCE_H
#define PSDRESOURCE_H

class PsdResource
{
    friend class PsdFile;
public:
    uint16_t getId() const
    {
        return mId;
    }

    const char *getName() const
    {
        return mName.c_str();
    }

    const uint8_t *getData() const
    {
        return mData;
    }

    uint32_t getSize() const
    {
        return mSize;
    }

private:
    PsdResource(uint16_t id, const string &name, const uint8_t *address, uint32_t size)
        : mId(id), mName(name), mData(address), mSize(size)
    {
    }

    uint16_t mId;
    string mName;
    const uint8_t *mData;
    uint32_t mSize;
};

typedef shared_ptr<map<uint16_t, shared_ptr<PsdResource> > > PsdResources;

#endif
