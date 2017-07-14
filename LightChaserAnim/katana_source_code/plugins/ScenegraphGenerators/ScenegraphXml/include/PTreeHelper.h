// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef PTREEHELPER_H_
#define PTREEHELPER_H_

#include <iostream>

// Property Tree using the RapidXML parser
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
using boost::property_tree::ptree;

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

class PTreeHelper
{
public:

    static ptree parse(std::string filename)
    {
        ptree pt;
        read_xml(filename, pt);

        if( !pt.empty() )
            return (*pt.begin()).second;
        else
            return ptree::basic_ptree();
    }

    static bool hasAttr(ptree node, std::string attrName)
    {
        // NOTE: Here find(..) should be used but for some reason it doesn't work
        //       unless the root (or current node) name is specified before <xmlattr>
        //       so using get(..) for the time being
        try
        {
            std::string attrValue = node.get<std::string>( "<xmlattr>." + attrName );
            return true;
        }
        catch( const std::exception & )
        {
            return false;
        }
    }

    static std::string getAttr(ptree node, std::string attrName)
    {
        try
        {
            return node.get<std::string>( "<xmlattr>." + attrName );
        }
        catch( const std::exception & )
        {
            return std::string();
        }
    }

    static void split(std::string str, std::vector<std::string> & result)
    {
        result.clear();

        boost::char_separator<char> sep(" ");
        tokenizer tokens(str, sep);

        BOOST_FOREACH( std::string token, tokens )
            result.push_back(token);
    }
};

#endif /* PTREEHELPER_H_ */
