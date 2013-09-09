//
//  TCPConnection.h
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
#include <string>
#include <functional>

using boost::asio::ip::tcp;

typedef std::shared_ptr<class TCPConnection> TCPConnectionRef;
typedef std::deque<std::string> MessageQueue;

class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
  public:
    TCPConnection( boost::asio::io_service &ioService, int clientId )
    : mSocket( ioService ), mClientId( clientId )
    {
        
    }
    
    tcp::socket& getSocket() { return mSocket; }
    
    void start()
    {
        if( onStart ) {
            onStart( shared_from_this() );
        }
        doRead();
    }
    
    void deliver( const std::string &msg )
    {
        bool writeInProgress = !mWriteMsg.empty();
        mWriteMsg.push_back( msg );
        if ( ! writeInProgress ) {
            doWrite();
        }
    }
    
    void close()
    {
        
    }
    
  private:
    void doRead()
    {
        auto self( shared_from_this() );
        boost::asio::async_read_until( mSocket, read, delim,
            [this, self]( boost::system::error_code error, std::size_t length )
            {
                if( ! error ) {
                    std::istream is(&read);
                    std::string mReadMsg;
                    std::getline( is, mReadMsg );
                    onRead( mReadMsg );
                    doRead();
                }
                else {
                    onError( mClientId, error );
                }
            });
    }
    
    void doWrite()
    {
        auto self( shared_from_this() );
        boost::asio::async_write( mSocket,
            boost::asio::buffer( mWriteMsg.front().data(), mWriteMsg.front().length() ),
            [this, self]( boost::system::error_code error, std::size_t length )
            {
                if( !error ) {
                    mWriteMsg.pop_front();
                    if( !mWriteMsg.empty() ) {
                        doWrite();
                    }
                }
                else {
                    onError( mClientId, error );
                }
            });
    }
    
  public:
    template<typename T, typename Y>
    void connectOnError( T callback, Y * callbackObject )
    {
        onError = std::bind( callback, callbackObject, std::placeholders::_1, std::placeholders::_2 );
    }
    
    template<typename T, typename Y>
    void connectOnRead( T callback, Y * callbackObject )
    {
        onRead = std::bind( callback, callbackObject, std::placeholders::_1 );
    }
    
    template<typename T, typename Y>
    void connectOnStart( T callback, Y * callbackObject )
    {
        onStart = std::bind( callback, callbackObject, std::placeholders::_1 );
    }
    
  private:
    char                                delim;
    tcp::socket                         mSocket;
    boost::asio::streambuf              read;
    MessageQueue                        mWriteMsg;
    int                                 mClientId;
    std::function<void(TCPConnectionRef)>               onStart;
    std::function<void(std::string)>                    onRead;
    std::function<void(int, boost::system::error_code&)> onError;
};