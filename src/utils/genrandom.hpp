/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean,

   Random Generators

*/

#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdint>

#include "utils/log.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/sugar/noncopyable.hpp"


class Random : noncopyable
{
public:
             Random() = default;
    virtual ~Random() = default;

    virtual void random(void * dest, size_t size) = 0;

    uint32_t rand32()
    {
        uint32_t ret;
        this->random(&ret, sizeof(ret));
        return ret;
    }
};


class UdevRandom : public Random
{
public:
    // TODO See if it wouldn't be better to always leave random source open. Maybe another class with that behaviour, to use when we need many random numbers/many randoms block. Unlikely in our use case.
    UdevRandom() = default;

    void random(void * dest, size_t size) override
    {
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd == -1){
            LOG(LOG_INFO, "access to /dev/urandom failed: %s", strerror(errno));
            fd = open("/dev/random", O_RDONLY);
            if (fd == -1){
                LOG(LOG_ERR, "random source failed to provide random data : couldn't open device");
                // TODO If random fails an exception should be raised, because security layers depends on random and should probably refuse working
                memset(dest, 0x44, size);
                return;
            }
            LOG(LOG_INFO, "using /dev/random as random source");
        }
        else {
            LOG(LOG_INFO, "using /dev/urandom as random source");
        }

         // this object is useful for RAII, do not unwrap
        unique_fd file(fd);

        // TODO This is basically a blocking read, we should provide timeout management and behaviour
        auto read = [fd](uint8_t * data, size_t len) -> ssize_t {
            size_t remaining_len = len;
            while (remaining_len) {
                ssize_t ret = ::read(fd, data + (len - remaining_len), remaining_len);
                if (ret < 0){
                    if (errno == EINTR){
                        continue;
                    }
                    // Error should still be there next time we try to read
                    if (remaining_len != len){
                        return len - remaining_len;
                    }
                    return ret;
                }
                // We must exit loop or we will enter infinite loop
                if (ret == 0){
                    break;
                }
                remaining_len -= ret;
            }
            return len - remaining_len;
        };

        ssize_t res = read(static_cast<uint8_t*>(dest), size);
        if (res != static_cast<ssize_t>(size)) {
            if (res >= 0){
                LOG(LOG_ERR, "random source failed to provide enough random data [%zd]", res);
            }
            else {
                LOG(LOG_ERR, "random source failed to provide random data [%s]", strerror(errno));
            }
            memset(dest, 0x44, size);
        }
    }
};
