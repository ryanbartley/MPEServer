//
//  MPEServer.h
//  MPE
//
//  Created by Ryan Bartley on 9/7/13.
//
//

#pragma once

#include "TCPServer.h"
#include "MPEProtocol.h"
#include <boost/signals2.hpp>

namespace mpe {
    
typedef std::shared_ptr<class MPEServer> MPEServerRef;

class MPEServer {
public:
    
    MPEServer( int port, int maxClients, int numThreads )
    : mServer( TCPServer::create( port, numThreads ) ),
        connections( mServer->getConnections() ), frameCount( 0 ),
        isPaused(false), numClients( 0 ), maxClients( maxClients ),
        screensDrawn( 0 )
    {
        mServer->connectOnAccept( &MPEServer::addClients, this );
    }
    
    static MPEServerRef create( int port, int numClients, int numThreads )
    { return MPEServerRef( new MPEServer( port, numClients, numThreads ) ); }
    
    bool addClients( TCPConnectionRef newConnection )
    {
        
        deliver.connect( std::bind( &TCPConnection::deliver, newConnection.get(), std::placeholders::_1 ) );
        newConnection->connectOnStart( &MPEServer::onConnectionStart, this );
        newConnection->connectOnError( &MPEServer::onError, this );
        newConnection->connectOnRead( &MPEServer::newMessage, this );
        
        
        std::cout << "I'm accepting connections and current connection size is " << connections.size() << std::endl;
        if ( connections.size() < maxClients ) {
            return true;
        }
        else {
            return false;
        }
    }
    
    void sendNextFrame() { deliver( std::to_string( frameCount ) ); }
    
    void reset()
    {
        frameCount = 0;
        screensDrawn = 0;
        mWriteMsgs.clear();
        sendReset();
        sendNextFrame();
    }
    void sendReset() {/* deliver( resetMessage ); */ }
    bool isNextFrameReady() { return screensDrawn >= numClients && ( !isPaused ) && numClients >= maxClients; }
    void broadcastMessage( const std::string &msg, int from, int to ) { auto m = BroadcastMessage( msg, from, to ); }
    void onError( int clientId, boost::system::error_code &error ) { std::cerr << "ERROR: " << error.message() << " clientID # " << clientId << endl; }
    void newMessage( const std::string &msg ) { cout << msg << endl; }
    void onConnectionStart( TCPConnectionRef connection ) { cout << "I'm in MPE and I've been called because things have started " << endl; }

    struct BroadcastMessage
    {
        BroadcastMessage( const std::string &msg, int from, int to )
        : msg( std::move(msg) ), from( from ), to( to )
        {}
        
    private:
        std::string msg;
        int         from;
        int         to;
    };
    
private:
    ServerRef                                   mServer;
    boost::signals2::signal<void(std::string)>  deliver;
    MessageQueue                                mWriteMsgs;
    std::map<int, TCPConnectionRef>             &connections;
    int                                         numClients;
    int                                         maxClients;
    int                                         frameCount;
    int                                         screensDrawn;
    bool                                        isPaused;
};

}