/*VRP*********************************************************************
 *
 * vehicle routing problems
 *      A collection of C++ classes for developing VRP solutions
 *      and specific solutions developed using these classes.
 *
 * Copyright 2014 Stephen Woodbridge <woodbri@imaptools.com>
 * Copyright 2014 Vicky Vergara <vicky_vergara@hotmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the MIT License. Please file LICENSE for details.
 *
 ********************************************************************VRP*/
#include "Library/OSRM.h"

#ifdef LOG
#include "logger.h"
#endif


#include "osrmclient.h"

#ifdef DOSTATS
#include "timer.h"
#include "stats.h"
#endif


OsrmClient *OsrmClient::p_osrm = NULL;
OSRM *OsrmClient::routing_machine = NULL;
bool OsrmClient::connectionAvailable = true;

/*!
 * \brief The OsrmClient constructor.
 */
OsrmClient::OsrmClient() {
    if ( not connectionAvailable ) return;

    #ifdef DOSTATS
    Timer timer;
    #endif

    try {
        ServerPaths server_paths;
        OsrmClient::routing_machine = new OSRM( server_paths, true );
    }
    catch ( std::exception &e ) {
        status = -1;
        err_msg = std::string( "OsrmClient::OsrmClient caught exception: " ) + e.what();
        #ifdef DOSTATS
        STATS->inc( err_msg );
        STATS->addto( "OsrmClient::OsrmClient Cumulative time", timer.duration() );
        #endif
        connectionAvailable = false;
        return;
    };

    route_parameters.zoomLevel = 18;
    route_parameters.printInstructions = true;
    route_parameters.alternateRoute = false;
    route_parameters.geometry = false;
    route_parameters.compression = false;
    route_parameters.checkSum = UINT_MAX;
    route_parameters.service = "viaroute";
    route_parameters.outputFormat = "json";
    route_parameters.jsonpParameter = "";
    route_parameters.language = "";
    status = 0;

    use = false;

    addPenalty = false;

    #ifdef LOG
    testOsrmClient();

    #endif
    #ifdef DOSTATS
    STATS->addto( "OsrmClient::OsrmClient Cumulative time", timer.duration() );

    #endif
}



/*!
 * \brief Clear out any old points and reset the OsrmClient to a clean state.
 */
void OsrmClient::clear() {
    if ( not connectionAvailable ) return;
    if ( not use ) return;

    #ifdef DOSTATS
    Timer timer;
    #endif
    route_parameters.coordinates.clear();
    route_parameters.hints.clear();
    route_parameters.geometry = false;
    httpContent = "";
    err_msg = "";

    if ( status > 0 ) status = 0;

    #ifdef DOSTATS
    STATS->addto( "OsrmClient::Clear Cumulative time", timer.duration() );
    #endif
}


/*!
 * \brief Add a location in WGS84 to the OSRM request.
 * \param[in] lat The latitude of the location you want to add.
 * \param[in] lon The Longitude of the location you want to add.
 * \note route_parameters.coordinates is defined as std::vector<FixedPointCoordinate> in OSRM.h. COORDINATE_PRECISION is also defined in OSRM.h.
 */
void OsrmClient::addViaPoint( double lat, double lon ) {
    if ( not connectionAvailable ) return;

    if ( not use ) return;

    #ifdef DOSTATS
    Timer timer;
    #endif
    FixedPointCoordinate p( lat * COORDINATE_PRECISION,
                            lon * COORDINATE_PRECISION );
    route_parameters.coordinates.push_back( p );
    #ifdef DOSTATS
    STATS->addto( "OsrmClient::addViaPoint Cumulative time", timer.duration() );
    #endif
}


/*!
 * \brief Add a \ref Node as a location to the OSRM request.
 * \param[in] node The Node to add, assumed to be in WGS84.
 */
void OsrmClient::addViaPoint( const Node &node ) {
    if ( not connectionAvailable ) return;

    if ( not use ) return;

    #ifdef DOSTATS
    Timer timer;
    #endif
    addViaPoint( node.gety(), node.getx() );
    route_parameters.hints.push_back( node.getHint() );
    #ifdef DOSTATS
    STATS->addto( "OsrmClient::addViaPoint Cumulative time", timer.duration() );
    #endif
}


/*!
 * \brief Add a path of \ref Node as locations to a the OSRM request.
 * \param[in] path A std::deque<Node> that you want to add.
 */
void OsrmClient::addViaPoints( const std::deque<Node> &path ) {
    if ( not connectionAvailable ) return;

    if ( not use ) return;

    std::deque<Node>::const_iterator it;

    for ( it = path.begin(); it != path.end(); ++it )
        addViaPoint( *it );
}

bool OsrmClient::getOsrmTime( double lat1, double lon1 , double lat2,
                              double lon2, double &time ) {
    if ( not connectionAvailable ) return false;

    if ( not use ) return false;

    #ifdef DOSTATS
    STATS->inc( "OsrmClient::getOsrmTime (2 points) " );
    #endif
    clear();
    addViaPoint( lat1, lon1 );
    addViaPoint( lat2, lon2 );

    if ( getOsrmViaroute() ) return getOsrmTime( time );

    return false;
}

bool OsrmClient::getOsrmTime( double lat1, double lon1 , double lat2,
                              double lon2, const  std::string &hint1, const std::string &hint2,
                              double &time ) {
    if ( not connectionAvailable ) return false;

    if ( not use ) return false;

    #ifdef DOSTATS
    STATS->inc( "OsrmClient::getOsrmTime (2 points) " );
    #endif
    clear();
    addViaPoint( lat1, lon1 );
    addViaPoint( lat2, lon2 );
    route_parameters.hints.push_back( hint1 );
    route_parameters.hints.push_back( hint2 );

    if ( getOsrmViaroute() ) return getOsrmTime( time );

    return false;
}

bool OsrmClient::getOsrmTime( const Node &node1, const Node &node2,
                              double &time ) {
    if ( not connectionAvailable ) return false;

    if ( not use ) return false;

    #ifdef DOSTATS
    STATS->inc( "OsrmClient::getOsrmTime (2 nodes) " );
    #endif
    clear();
    addViaPoint( node1 );
    addViaPoint( node2 );
    route_parameters.hints.push_back( node1.getHint() );
    route_parameters.hints.push_back( node2.getHint() );

    if ( getOsrmViaroute() ) return  getOsrmTime( time );

    return false;
}

bool OsrmClient::getOsrmTime( const Node &node1, const Node &node2,
                              const Node &node3, double &time ) {
    if ( not connectionAvailable ) return false;

    if ( not use ) return false;

    #ifdef DOSTATS
    STATS->inc( "OsrmClient::getOsrmTime (3 nodes) " );
    #endif
    clear();
    addViaPoint( node1 );
    addViaPoint( node2 );
    addViaPoint( node3 );
    route_parameters.hints.push_back( node1.getHint() );
    route_parameters.hints.push_back( node2.getHint() );
    route_parameters.hints.push_back( node3.getHint() );

    if ( getOsrmViaroute() ) return getOsrmTime( time );

    return false;
}

bool OsrmClient::getOsrmTime( const Node &node1, const Node &node2,
                              const Node &node3, const Node &node4, double &time ) {
    if ( not connectionAvailable ) return false;

    if ( not use ) return false;

    #ifdef DOSTATS
    STATS->inc( "OsrmClient::getOsrmTime (4 nodes) " );
    #endif
    clear();
    addViaPoint( node1 );
    addViaPoint( node2 );
    addViaPoint( node3 );
    addViaPoint( node4 );
    route_parameters.hints.push_back( node1.getHint() );
    route_parameters.hints.push_back( node2.getHint() );
    route_parameters.hints.push_back( node3.getHint() );
    route_parameters.hints.push_back( node4.getHint() );

    if ( getOsrmViaroute() ) return getOsrmTime( time );

    return false;
}



/*!
 * \brief Connect to the OSRM engine, issue the request and save the json response back in the object.
 * \return True if an error happened and err_msg will be set. False if ok.
 */
bool OsrmClient::getOsrmViaroute() {
    if ( not connectionAvailable ) return false;

    if ( not use ) return false;

    #ifdef DOSTATS
    Timer timer;
    STATS->inc( "OsrmClient::getOsrmViaRoute (does the work) " );
    #endif

    if ( route_parameters.coordinates.size() < 2 ) {
        err_msg = "OsrmClient:getOsrmViaroute must be called with two ro more viapoints!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        STATS->addto( "OsrmClient::getOsrmViaRoute (does the work) Cumulative time",
                      timer.duration() );
        #endif
        return false;
    }

    if ( status == -1 ) {
        err_msg = "OsrmClient:getOsrmViaroute Failed to connect to server!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        STATS->addto( "OsrmClient::getOsrmViaRoute (does the work) Cumulative time",
                      timer.duration() );
        #endif
        return false;
    }

    err_msg = "";
    http::Reply osrm_reply;

    try {
        routing_machine->RunQuery( route_parameters, osrm_reply );
    }
    catch ( std::exception &e ) {
        err_msg = std::string( "OsrmClient:getOsrmViaRoute caught exception: " )
                  + e.what();
        connectionAvailable = false;
        #ifdef DOSTATS
        STATS->inc( err_msg );
        STATS->addto( "OsrmClient::getOsrmViaRoute (does the work) Cumulative time",
                      timer.duration() );
        #endif
        return false;
    }



    httpContent = "";
    std::vector<std::string>::iterator sit;

    for ( sit = osrm_reply.content.begin();
          sit != osrm_reply.content.end();
          ++sit )
        httpContent += *sit;

    status = 1;
    #ifdef DOSTATS
    STATS->addto( "OsrmClient::getOsrmViaRoute (does the work) Cumulative time",
                  timer.duration() );
    #endif
    return true;
}


/*!
 * \brief Get the OSRM travel time for the requested route.
 * \param[out] time The OSRM travel time in decimal minutes.
 * \return True if an error was encountered and err_msg will be set. False if ok.
 */
bool OsrmClient::getOsrmTime( double &time ) {
    if ( not connectionAvailable ) return false;

    if ( not use ) return false;

    #ifdef DOSTATS
    Timer timer;
    STATS->inc( "OsrmClient::getOsrmTime (does the work)" );
    #endif

    if ( status != 1 or httpContent.size() == 0 ) {
        err_msg = "OsrmClient:getOsrmTime does not have a valid OSRM response!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        STATS->addto( "OsrmClient::getOsrmTime (errors) Cumulative time:",
                      timer.duration() );
        #endif
        return false;
    }

    rapidjson::Document jsondoc;
    jsondoc.Parse( httpContent.c_str() );

    if ( jsondoc.HasParseError() ) {
        err_msg = "OsrmClient:getOsrmTime invalid json document in OSRM response!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        STATS->addto( "OsrmClient::getOsrmTime (errors) Cumulative time:",
                      timer.duration() );
        #endif
        return false;
    }

    if ( not getTime( jsondoc, time ) ) {
        #ifdef DOSTATS
        STATS->addto( "OsrmClient::getOsrmTime (errors) Cumulative time:",
                      timer.duration() );
        #endif
        return false;
    }

    #ifdef DOSTATS
    STATS->addto( "OsrmClient::getOsrmTime (does the work) Cumulative time:",
                  timer.duration() );
    #endif
    return true;
}

bool OsrmClient::getOsrmPenalty( double &penalty ) {
    if ( not connectionAvailable ) return false;

    if ( not use ) return false;

    #ifdef DOSTATS
    Timer timer;
    STATS->inc( "OsrmClient::getOsrmTurns" );
    #endif

    if ( status != 1 or httpContent.size() == 0 ) {
        err_msg = "OsrmClient:getOsrmTurns does not have a valid OSRM response!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        STATS->addto( "OsrmClient::getOsrmTurns (interface) Cumultaive time:",
                      timer.duration() );
        #endif
        return false;
    }

    rapidjson::Document jsondoc;
    jsondoc.Parse( httpContent.c_str() );

    if ( jsondoc.HasParseError() ) {
        err_msg = "OsrmClient:getOsrmTurns (interface) invalid json document in OSRM response!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        STATS->addto( "OsrmClient::getOsrmTurns (interface) Cumultaive time:",
                      timer.duration() );
        #endif
        return false;
    }

    if ( not getPenalty( jsondoc, penalty ) ) {
        #ifdef DOSTATS
        STATS->addto( "OsrmClient::getOsrmTurns (interface) Cumultaive time:",
                      timer.duration() );
        #endif
        return false;
    }

    #ifdef DOSTATS
    STATS->addto( "OsrmClient::getOsrmTurns (interface) Cumulative time:",
                  timer.duration() );
    #endif
    return true;
}


/*!
 * \brief Extract the geometry from the OSRM response if it was requested.
 * \param[out] geom A std::deque<Node> with each point in the path set as a \ref Node.
 * \return True if an error was encountered and err_msg will be set. False if ok.
 */
bool OsrmClient::getOsrmGeometry( std::deque<Node> &geom ) {
    if ( not connectionAvailable ) return false;

    if ( not use ) return false;

    #ifdef DOSTATS
    Timer timer;
    STATS->inc( "OsrmClient::getOsrmGeometry (does the work) " );
    #endif

    if ( status != 1 or httpContent.size() == 0 ) {
        err_msg = "OsrmClient::getOsrmGeometry does not have a valid OSRM response!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        #endif
        return false;
    }

    rapidjson::Document jsondoc;
    jsondoc.Parse( httpContent.c_str() );

    if ( jsondoc.HasParseError() ) {
        err_msg = "OsrmClient:getOsrmGeometry invalid json document in OSRM response!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        #endif
        return false;
    }

    if ( not getGeom( jsondoc, geom ) ) {
        return false;
    }

    return true;
}

bool OsrmClient::getOsrmHints( std::deque<std::string> &hints ) {
    if ( not connectionAvailable ) return false;

    if ( not use ) return false;

    #ifdef DOSTATS
    Timer timer;
    STATS->inc( "OsrmClient::getOsrmHint (interface) " );
    #endif

    if ( status != 1 or httpContent.size() == 0 ) {
        err_msg = "OsrmClient::getOsrmHint (interface) does not have a valid OSRM response!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        #endif
        return false;
    }

    rapidjson::Document jsondoc;
    jsondoc.Parse( httpContent.c_str() );

    if ( jsondoc.HasParseError() ) {
        err_msg = "OsrmClient:getOsrmHint (interface) invalid json document in OSRM response!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        #endif
        return false;
    }

    if ( not getHints( jsondoc, hints ) ) {
        return false;
    }

    return true;
}


bool OsrmClient::testOsrmClient() {
    if ( not use ) return false;

    if ( not connectionAvailable ) return false;

    std::deque<std::string> hints;

    if ( getStatus() == -1 ) {
        DLOG( WARNING ) << getErrorMsg();
        return false;
    }

    double penalty;
    bool oldPenalty = addPenalty;
    double time;

    // TODO Make this location a config value
    //34.890816,-56.165529
    if ( getOsrmTime( -34.8917, -56.167694, -34.890816, -56.165529, time ) )
        DLOG( INFO ) << "test time:" << time;
    else
        return false;

    getOsrmPenalty( penalty );

    if ( getOsrmHints( hints ) ) {
        std::string hint1 = hints[0];
        std::string hint2 = hints[1];

        if ( getOsrmTime( -34.8917, -56.167694, -34.890816, -56.165529,
                          hint1, hint2, time ) )
            DLOG( INFO ) << "test time:" << time;
        else
            return false;
    }
    else
        return false;

    if ( not getOsrmPenalty( penalty ) )
        return false;

    addPenalty = true;

    if ( getOsrmTime( -34.8917, -56.167694, -34.890816, -56.165529, time ) )
        DLOG( INFO ) << "test time: " << time;
    else return false;

    getOsrmPenalty( penalty );

    if ( getOsrmHints( hints ) ) {
        std::string hint1 = hints[0];
        std::string hint2 = hints[1];

        if ( getOsrmTime( -34.8917, -56.167694, -34.890816, -56.165529, hint1, hint2,
                          time ) )
            DLOG( INFO ) << "test time: " << time;
        else
            return false;
    }
    else
        return false;

    addPenalty = oldPenalty;

    return true;
};




// --------- private ----------------


/*!
 * \brief Parse the actual json document and extract the OSRM Travel time.
 * \param[in] jsondoc The json parse tree pointer.
 * \param[out] time The OSRM travel time as decimal minutes.
 * \return True if an error and err_msg will be set. False otherwise.
 */
bool OsrmClient::getTime( rapidjson::Document &jsondoc, double &time ) {
    if ( not connectionAvailable ) return false;

    if ( not jsondoc.HasMember( "route_summary" ) ) {
        err_msg = "OsrmClient:getTime failed to find 'route_summary' key in OSRM response!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        #endif
        return false;
    }

    // find the 'total_time' in the 'route_summary'
    if ( not jsondoc["route_summary"].HasMember( "total_time" ) ) {
        err_msg = "OsrmClient:getTime failed to find 'total_time' key in OSRM response!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        #endif
        return false;
    }

    // extract the total_time and convert from seconds to minutes
    time = ( double ) jsondoc["route_summary"]["total_time"].GetDouble() / 60.0;
    double penalty;

    if ( addPenalty and getPenalty( jsondoc, penalty ) ) time += penalty;

    return true;
}


/*!
 * \brief Parse the actual json document and extract the geometry Nodes
 * \param[in] jsondoc The json parse tree pointer.
 * \param[out] geom A std::deque<Node> with each point in the path.
 * \return True if an error and err_msg will be set. False otherwise.
 */
bool OsrmClient::getGeom( rapidjson::Document &jsondoc, std::deque<Node> &geom ) {
    if ( not connectionAvailable ) return false;

    // clear the path
    geom.clear();

    // find the route 'geometry' key in the response
    const rapidjson::Value &jgeom = jsondoc["route_geometry"];

    if ( not jgeom.IsArray() ) {
        err_msg = "OsrmClient:getGeom failed to find 'geometry' key in OSRM response!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        #endif
        return false;
    }

    for ( rapidjson::Value::ConstValueIterator itr = jgeom.Begin();
          itr != jgeom.End(); ++itr ) {

        // osrm returns [lat, lon], node expects [lon, lat]
        Node n( (*itr)[1].GetDouble(), (*itr)[0].GetDouble() );
        geom.push_back( n );
    }

    return true;
}

bool OsrmClient::getHints( rapidjson::Document &jsondoc,
                           std::deque<std::string> &hints ) {
    if ( not connectionAvailable ) return false;

    hints.clear();

    // find the route 'hint_data' key in the response
    const rapidjson::Value &jHintData = jsondoc["hint_data"];

    if ( not jHintData.IsObject() ) {
        err_msg = "OsrmClient:getHints (private)  failed to find 'hint_data' key in OSRM response!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        #endif
        return false;
    }

    const rapidjson::Value &jLocations = jHintData["locations"];

    if ( not jHintData.IsArray() ) {
        err_msg = "OsrmClient:getHints (private)  failed to find 'locations' key in OSRM response!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        #endif
        return false;
    }


    std::string hint;

    for ( rapidjson::Value::ConstValueIterator itr = jHintData.Begin();
          itr != jHintData.End(); ++itr) {

        hint = std::string( itr->GetString() );
        hints.push_back( hint );

    }

    return true;
}


bool OsrmClient::getPenalty( rapidjson::Document &jsondoc,
                             double &penalty ) { //in munutes
    if ( not connectionAvailable ) return false;

    int turn;

    std::string  trace;

    penalty = 0;

    // find the route 'hint_data' key in the response
    const rapidjson::Value &jInstructionsArray = jsondoc["route_instructions"];

    if ( not jInstructionsArray.IsArray() ) {
        err_msg = "OsrmClient:getTurns (private) failed to find 'route_instructions' key in OSRM response!";
        #ifdef DOSTATS
        STATS->inc( err_msg );
        #endif
        return false;
    }

    for (rapidjson::Value::ConstValueIterator itr = jInstructionsArray.Begin();
            itr != jInstructionsArray.End(); ++itr) {

        turn = strtol( (*itr)[0].GetString(), NULL, 10 );

        trace = std::string( (*itr)[0].GetString() );
        DLOG( INFO ) << "InstructionsData " << trace;
        DLOG( INFO ) << "Instruction " << turn;

        switch ( turn ) {
            case 2: penalty += 0.05; break; //slight right

            case 3: penalty += 0.10; break; //right

            case 4: penalty += 0.3 ; break; //sharp right

            case 5: penalty += 1;  break; //uturn

            case 8: penalty += 0.05; break; //slight left

            case 7: penalty += 0.10; break; //left

            case 6: penalty += 0.3 ; break; //sharp left
        }

    }

    DLOG( INFO ) << "Penalty " << penalty;
    return true;
}

