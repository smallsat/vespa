// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "rpcserviceaddress.h"
#include <vespa/slobrok/imirrorapi.h>

namespace mbus {

class RPCNetwork;

/**
 * An RPCService represents a set of remote sessions matching a service pattern.
 * The sessions are monitored using the slobrok. If multiple sessions are
 * available, round robin is used to balance load between them.
 */
class RPCService {
private:
    typedef slobrok::api::IMirrorAPI  Mirror;
    typedef Mirror::SpecList          AddressList;

    const Mirror &_mirror;
    string        _pattern;
    uint32_t      _addressIdx;
    uint32_t      _addressGen;
    AddressList   _addressList;

public:
    using UP = std::unique_ptr<RPCService>;
    RPCService(const RPCService &) = delete;
    RPCService & operator = (const RPCService &) = delete;
    /**
     * Create a new RPCService backed by the given network and using
     * the given service pattern.
     *
     * @param mirror  The naming server to send queries to.
     * @param pattern The pattern to use when querying.
     */
    RPCService(const Mirror &mirror, const string &pattern);
    ~RPCService();

    /**
     * Resolve a concrete address from this service. This service may represent
     * multiple remote sessions, so this will select one that is online.
     *
     * @return A concrete service address.
     */
    RPCServiceAddress::UP resolve();

    /**
     * Returns the pattern used when querying for the naming server for
     * addresses. This is given at construtor time.
     *
     * @return The service pattern.
     */
    const string &getPattern() const { return _pattern; }
};

} // namespace mbus
