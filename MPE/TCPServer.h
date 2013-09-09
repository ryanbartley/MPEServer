//
//  TCPServer.h
//  MPE
//
//  Created by Ryan Bartley on 9/7/13.
//
//

#pragma once

#include <algorithm>
#include <cstdlib>
#include <map>
#include <deque>
#include <iostream>
#include <list>
#include <set>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <thread>
#include <functional>
#include "TCPConnection.h"

using boost::asio::ip::tcp;
using std::cout;
using std::endl;

typedef std::shared_ptr<class TCPServer> ServerRef;

class TCPServer : public boost::enable_shared_from_this<TCPServer> {
public:
    explicit TCPServer( int port, int numThreads = 1 )
    : mIoService(), mAcceptor( mIoService, tcp::endpoint( tcp::v4(), port ) ),
        numClients( 0 ), numThreads( numThreads ), accept(true)
    {
        run();
    }
    
    static ServerRef create( int port, int numThreads )
    { return ServerRef( new TCPServer( port, numThreads ) ); }
    
    std::map<int, TCPConnectionRef>& getConnections() { return connections; }
    
    ~TCPServer()
    {
        stop();
    }
    
    void run()
    {
        for( int i = 0; i < numThreads; ++i ) {
            threadPool.push_back( std::thread( boost::bind( &boost::asio::io_service::run, &mIoService ) ) );
        }
        doAccept();
    }
    
    void stop()
    {
        mIoService.stop();
        for( auto threadIt = threadPool.begin(); threadIt < threadPool.end(); ++threadIt ) {
            threadIt->join();
        }
    }
    
    template<typename T, typename Y>
    void connectOnAccept( T callback, Y * callbackObject )
    {
        onAccept = std::bind( callback, callbackObject, std::placeholders::_1 );
    }

private:
    void doAccept()
    {
        if( accept ) {
            TCPConnectionRef newConnection( new TCPConnection( mIoService, numClients ) );
            mAcceptor.async_accept( newConnection->getSocket(),
                [this, newConnection]( const boost::system::error_code& error )
                {
                    if( !error ) {
                        cout << "I'm accepting a connection inside the lambda right now" << endl;
                        connections.insert( std::pair< int, TCPConnectionRef >( numClients++, newConnection ) );
                        cout << "I should be calling onAccept" << endl;
                        accept = onAccept( newConnection );
                        newConnection->start();
                    }
                    
                    doAccept();
                });
        }
    }
    
    
private:
    boost::asio::io_service                         mIoService;
    tcp::acceptor                                   mAcceptor;
    std::vector<std::thread>                        threadPool;
    int                                             numThreads;
    std::map<int, TCPConnectionRef>                 connections;
    int                                             numClients;
    bool                                            accept;
    std::function<bool( TCPConnectionRef )>         onAccept;
    
};




























